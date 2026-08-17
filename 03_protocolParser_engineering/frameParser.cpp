#include "FrameParser.h"

// 核心函数：每收到一个字节调用一次
void FrameParser::feedByte(uint8_t byte) {
    switch (current_state) {
        case State::FIND_HEADER1:
            handleFindHeader1(byte);
            break;
        case State::FIND_HEADER2:
            handleFindHeader2(byte);
            break;
        case State::READ_BODY:
            handleReadBody(byte);
            break;
        case State::CHECK:
            // CHECK 状态只是瞬间跳转，实际上不会在这个状态停留
            break;
    }
}

// 获取解析结果
bool FrameParser::getData(SensorData& data) {
    if (!m_hasNewData) {
        return false;
    }
    data = m_data;        // 拷贝出去
    m_hasNewData = false; // 消费掉
    return true;
}

// 重置状态机（用于外部强制复位）
void FrameParser::reset() {
    current_state = State::FIND_HEADER1;
    index = 0;
    data_len = 0;
    // 不要重置 m_data 和 m_hasNewData
}

ParseError FrameParser::getLastError() const { return m_lastError; }

void FrameParser::handleFindHeader1(uint8_t byte) {
    if (byte == 0xAA) {
        buffer[0] = byte;
        index = 1;
        current_state = State::FIND_HEADER2;
    }
    // 其他字节直接丢弃，留在 FIND_HEADER1
}

void FrameParser::handleFindHeader2(uint8_t byte) {
    if (byte == 0x55) {
        buffer[1] = byte;
        index = 2;
        current_state = State::READ_BODY;
    } else if (byte == 0xAA) {
        buffer[0] = byte;
        index = 1;
        current_state = State::FIND_HEADER2;
    } else {
        current_state = State::FIND_HEADER1; // 处理更严格
        index = 0;
    }
}

void FrameParser::handleReadBody(uint8_t byte) {
    // 溢出保护：如果已经超出缓冲区大小，直接重置
    if (index >= MAX_FRAME_LEN) {
        m_lastError = ParseError::BUFFER_OVERFLOW;
        reset();
        return;
    }

    buffer[index] = byte;
    index++;
    
    // 如果刚收到 LEN 字段（位置 3），记录数据长度
    if (index == 4) { 
        data_len = buffer[3];
        // 长度合法性检查：规定type=0x01长度必须为4B
        if (buffer[2] != 0x01) {
            m_lastError = ParseError::TYPE_ERROR; 
            reset(); 
            return;
        }
        if (data_len != 0x04) {
            m_lastError = ParseError::LENGTH_ERROR; 
            reset();  
            return;
        }
    }
    
    // 判断是否收完了整个帧
    size_t total_frame_len = 5 + data_len;
    
    if (index == total_frame_len) {
        // 收完了，进入校验状态
        current_state = State::CHECK;
        
        // 计算校验和从 TYPE 到 DATA 最后一个字节
        uint8_t calc_checksum = 0;
        for (size_t i = 2; i < 4 + data_len; i++) {
            calc_checksum += buffer[i];
        }
        
        uint8_t received_checksum = buffer[4 + data_len];
        if (calc_checksum == received_checksum) {

            // ====== 校验成功，解析数据 ======
            uint16_t temp_raw = 
                (static_cast<uint16_t>(buffer[4] << 8) 
                | static_cast<uint16_t>(buffer[5]));
            m_data.temperature = temp_raw / 10.0f;
            uint16_t humi_raw = 
                (static_cast<uint16_t>(buffer[6] << 8) 
                | static_cast<uint16_t>(buffer[7]));
            m_data.humidity = humi_raw / 10.0f;
            
            m_hasNewData = true;  // 标记有新数据
            m_lastError = ParseError::NONE; // 清除错误状态
        } else {
            m_lastError = ParseError::CHECKSUM_ERROR;
        }

        // ====== 无论成败，清空状态，重新开始 ======
        reset();
    }
}