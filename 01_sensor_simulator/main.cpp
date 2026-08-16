#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>
#include <cstdint>
#include <vector>
using namespace std;


int main() {
    random_device rd;
    default_random_engine gen(rd());

    cout << "Sensor Simulator Started." << endl;

    uniform_int_distribution<int> T(200, 300);
    uniform_int_distribution<int> H(400, 600);

    cout << fixed << setprecision(1);

    // while(1) {
        // 人看的，最后/10.0即可
        // int temperature = T(gen);
        // int humidity = H(gen);
        int temperature = 256;
        int humidity = 523;
        
        // 协议看的
        uint16_t temp = temperature;
        uint16_t hum = humidity;

        // 生成DATA字节
        uint8_t temp_high = (temp >> 8) & 0xFF;
        uint8_t temp_low = temp & 0xFF;
        uint8_t hum_high = (hum >> 8) & 0xFF;
        uint8_t hum_low = hum & 0xFF;

        // 数据帧
        vector<uint8_t> data = {
            0xAA, 0x55, 0x01, 0x04, temp_high, temp_low, hum_high, hum_low
        };

        // 校验
        uint8_t checksum = 0;
        for (size_t i = 2; i < data.size(); ++i) {
            checksum += data[i];
        }
        data.push_back(checksum);

        cout << "Temperature: " << temperature/10.0 << " C" << endl; 
        cout << "Humidity: " << humidity/10.0 << " %" << endl;
        cout << "Data Frame: ";
        for(const auto& byte : data) {
            cout << hex << setw(2) << setfill('0') << static_cast<int>(byte) << " ";
        }
        cout << endl;

        this_thread::sleep_for(chrono::seconds(1));
    // }

    return 0;
}