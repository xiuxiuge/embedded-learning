#include "FrameParser.h"
#include <string.h>   // memset

// ===== 内部状态枚举（不对外暴露） =====

typedef enum
{
    STATE_FIND_HEADER1,
    STATE_FIND_HEADER2,
    STATE_READ_BODY,
    STATE_CHECK
} ParserState;

// ===== FrameParser 结构体定义（内部可见，外部不可见） =====

#define MAX_FRAME_LEN 16

struct FrameParser
{
    // 状态机变量
    ParserState current_state;
    uint8_t buffer[MAX_FRAME_LEN];
    size_t index;
    uint8_t data_len;

    // 输出数据
    SensorData sensor_data;
    bool has_new_data;

    // 错误状态
    ParseError last_error;
};

// ===== 私有函数声明（内部使用） =====

static void handleFindHeader1(FrameParser *me, uint8_t byte);
static void handleFindHeader2(FrameParser *me, uint8_t byte);
static void handleReadBody(FrameParser *me, uint8_t byte);

// ===== 公有函数实现 =====

void FrameParser_Init(FrameParser *me)
{
    me->current_state = STATE_FIND_HEADER1;
    me->index = 0;
    me->data_len = 0;
    me->sensor_data.temperature = 0.0f;
    me->sensor_data.humidity = 0.0f;
    me->has_new_data = false;
    me->last_error = PARSE_ERROR_NONE;
    memset(me->buffer, 0, sizeof(me->buffer));
}

void FrameParser_FeedByte(FrameParser *me, uint8_t byte)
{
    switch (me->current_state)
    {
        case STATE_FIND_HEADER1:
            handleFindHeader1(me, byte);
            break;
        case STATE_FIND_HEADER2:
            handleFindHeader2(me, byte);
            break;
        case STATE_READ_BODY:
            handleReadBody(me, byte);
            break;
        case STATE_CHECK:
            // CHECK 状态只是瞬间跳转，不会停留
            break;
    }
}

bool FrameParser_GetData(FrameParser *me, SensorData *outData)
{
    if (!me->has_new_data)
    {
        return false;
    }
    *outData = me->sensor_data;  // 拷贝出去
    me->has_new_data = false;    // 消费掉
    return true;
}

ParseError FrameParser_GetLastError(const FrameParser *me)
{
    return me->last_error;
}

void FrameParser_Reset(FrameParser *me)
{
    me->current_state = STATE_FIND_HEADER1;
    me->index = 0;
    me->data_len = 0;
    // 注意：不重置 sensor_data 和 has_new_data（让应用层决定是否丢弃）
}

// ===== 私有函数实现 =====

static void handleFindHeader1(FrameParser *me, uint8_t byte)
{
    if (byte == 0xAA)
    {
        me->buffer[0] = byte;
        me->index = 1;
        me->current_state = STATE_FIND_HEADER2;
    }
    // 其他字节直接丢弃，留在 STATE_FIND_HEADER1
}

static void handleFindHeader2(FrameParser *me, uint8_t byte)
{
    if (byte == 0x55)
    {
        me->buffer[1] = byte;
        me->index = 2;
        me->current_state = STATE_READ_BODY;
    }
    else if (byte == 0xAA)
    {
        // 特殊情况：收到 AA 55 AA，第二个 AA 当作新的头1
        me->buffer[0] = byte;
        me->index = 1;
        me->current_state = STATE_FIND_HEADER2;
    }
    else
    {
        // 收到非法字节，回到找头1
        me->current_state = STATE_FIND_HEADER1;
        me->index = 0;
    }
}

static void handleReadBody(FrameParser *me, uint8_t byte)
{
    // 溢出保护
    if (me->index >= MAX_FRAME_LEN)
    {
        me->last_error = PARSE_ERROR_BUFFER_OVERFLOW;
        FrameParser_Reset(me);
        return;
    }

    me->buffer[me->index] = byte;
    me->index++;

    // 刚收到 LEN 字段（位置 3），记录数据长度并检查合法性
    if (me->index == 4)
    {
        me->data_len = me->buffer[3];

        // 检查 TYPE（假设 0x01 表示传感器数据）
        if (me->buffer[2] != 0x01)
        {
            me->last_error = PARSE_ERROR_TYPE_ERROR;
            FrameParser_Reset(me);
            return;
        }

        // 检查 LEN（假设传感器数据固定 4 字节）
        if (me->data_len != 0x04)
        {
            me->last_error = PARSE_ERROR_LENGTH_ERROR;
            FrameParser_Reset(me);
            return;
        }
    }

    // 判断是否收完了整个帧
    // 协议：AA 55 TYPE LEN DATA[LEN] CHECK
    // 总长度 = 2（头） + 1（TYPE） + 1（LEN） + LEN + 1（CHECK）
    size_t total_frame_len = 5 + me->data_len;

    if (me->index == total_frame_len)
    {
        // 收完了，进入校验状态
        me->current_state = STATE_CHECK;

        // 计算校验和：从 TYPE 到 DATA 最后一个字节
        uint8_t calc_checksum = 0;
        for (size_t i = 2; i < 4 + me->data_len; i++)
        {
            calc_checksum += me->buffer[i];
        }

        uint8_t received_checksum = me->buffer[4 + me->data_len];

        if (calc_checksum == received_checksum)
        {
            // ===== 校验成功，解析数据 =====
            // 假设数据是大端模式：高字节在前
            uint16_t temp_raw = (uint16_t)(me->buffer[4] << 8) | me->buffer[5];
            uint16_t humi_raw = (uint16_t)(me->buffer[6] << 8) | me->buffer[7];

            me->sensor_data.temperature = temp_raw / 10.0f;
            me->sensor_data.humidity = humi_raw / 10.0f;

            me->has_new_data = true;
            me->last_error = PARSE_ERROR_NONE;
        }
        else
        {
            me->last_error = PARSE_ERROR_CHECKSUM_ERROR;
        }

        // ===== 无论成败，清空状态，重新开始 =====
        FrameParser_Reset(me);
    }
}