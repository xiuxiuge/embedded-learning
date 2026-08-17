#include "FrameParser.h"
#include <iostream>
#include <vector>

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