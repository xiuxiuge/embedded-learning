#include <iostream>
#include <vector>
#include <cstdint>

// 状态枚举
enum class State {
    FIND_HEADER1,  // 等待 0xAA
    FIND_HEADER2,  // 等待 0x55
    READ_BODY,     // 接收数据体
    CHECK          // 校验并解析
};

// 温湿度数据
struct SensorData {
    float temperature;
    float humidity;
};

class FrameParser {
private:
    static constexpr size_t MAX_FRAME_LEN = 16;  // 最大帧长度保护
    
    State current_state = State::FIND_HEADER1;
    uint8_t buffer[MAX_FRAME_LEN] = {0};  
    size_t index = 0;          // 当前已接收字节数
    uint8_t data_len = 0;      // 从帧里解析出的数据长度
    
    // 温湿度结果
    SensorData m_data{0.0f, 0.0f}; // 存储解析出的数据，等待外部来取
    bool m_hasNewData = false;
    
public:
    // 核心函数：每收到一个字节调用一次
    void feedByte(uint8_t byte) {
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
    bool getData(SensorData& data) {
        if (!m_hasNewData) {
            return false;
        }
        data = m_data;        // 拷贝出去
        m_hasNewData = false; // 消费掉
        return true;
    }

    // 重置状态机（用于外部强制复位）
    void reset() {
        current_state = State::FIND_HEADER1;
        index = 0;
        data_len = 0;
        // 不要重置 m_data 和 m_hasNewData
    }

private:
    // ========== 状态处理函数 ==========
    
    void handleFindHeader1(uint8_t byte) {
        if (byte == 0xAA) {
            buffer[0] = byte;
            index = 1;
            current_state = State::FIND_HEADER2;
        }
        // 其他字节直接丢弃，留在 FIND_HEADER1
    }
    
    void handleFindHeader2(uint8_t byte) {
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
    
    void handleReadBody(uint8_t byte) {
        // 溢出保护：如果已经超出缓冲区大小，直接重置
        if (index >= MAX_FRAME_LEN) {
            reset();
            return;
        }

        buffer[index] = byte;
        index++;
        
        // 如果刚收到 LEN 字段（位置 3），记录数据长度
        if (index == 4) { 
            data_len = buffer[3];

            // 长度合法性检查：规定type=0x01长度必须为4B
            if (buffer[2] != 0x01 || data_len != 0x04) {
                reset();  // 不符合协议
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
                uint16_t temp_raw = (buffer[4] << 8) | buffer[5];
                m_data.temperature = temp_raw / 10.0f;
                uint16_t humi_raw = (buffer[6] << 8) | buffer[7];
                m_data.humidity = humi_raw / 10.0f;
                
                m_hasNewData = true;  // 标记有新数据
            } else {
                
            }

            // ====== 无论成败，清空状态，重新开始 ======
            reset();
        }
    }
};

// ========== 测试代码 ==========
int main() {
    FrameParser parser;
    std::cout << "\n=== Final test: Checksum error ===" << std::endl;
    std::vector<uint8_t> test4 = {
        0x00, 0xFF, 0xAA, 0x12, 0x34, 0xAA, 0x55, 0x01, 0x04, 0x01,
        0x00, 0x02, 0x0B, 0x13, 0xAA, 0x55, 0x01, 0x04, 0x01, 0x00,
        0x02, 0x0C, 0x13, 0xAA, 0x55, 0x01, 0x04, 0x01, 0x00, 0x02,
        0x0B, 0x13
    };
    
    for (uint8_t b : test4) {
        parser.feedByte(b);
        
        // 每次喂完字节，检查是否有新数据
        SensorData data;
        if (parser.getData(data)) {
            std::cout << "[OK] Temperature: " << data.temperature << " C, "
                      << "Humidity: " << data.humidity << " %" << std::endl;
        }
    }

    return 0;
}