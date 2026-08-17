#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H

#include <cstdint>
#include <cstddef>

// 状态枚举（外部不需要知道，所以不放在 .h 里）
// 但 SensorData 和 ParseError 外部需要用，所以要放

struct SensorData {
    float temperature;
    float humidity;
};

enum class ParseError {
    NONE = 0,
    TYPE_ERROR,
    LENGTH_ERROR,
    CHECKSUM_ERROR,
    BUFFER_OVERFLOW
};

class FrameParser {
public:
    void feedByte(uint8_t byte);
    bool getData(SensorData& data);
    ParseError getLastError() const;
    void reset();

private:
    // 成员变量声明（外部看不到具体类型也没关系，但需要知道大小）
    // 这里只放声明，不放定义
    enum class State {
        FIND_HEADER1,
        FIND_HEADER2,
        READ_BODY,
        CHECK
    };

    static constexpr size_t MAX_FRAME_LEN = 16;

    State current_state = State::FIND_HEADER1;  // 当前状态
    uint8_t buffer[MAX_FRAME_LEN] = {0};        // 接收缓冲区
    size_t index = 0;                           // 当前已接收字节数
    uint8_t data_len = 0;                       // 从帧里解析出的数据长度

    SensorData m_data{0.0f, 0.0f};              // 解析出的温湿度数据
    bool m_hasNewData = false;                  // 是否有新数据可供获取
    ParseError m_lastError = ParseError::NONE;  // 上一次解析错误类型

    // 私有方法声明（内部使用）
    void handleFindHeader1(uint8_t byte);
    void handleFindHeader2(uint8_t byte);
    void handleReadBody(uint8_t byte);
};

#endif