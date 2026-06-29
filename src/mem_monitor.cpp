#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <thread>
#include <chrono>

// 步骤1：定义内存数据结构
struct MemInfo {
    long total;        // 总内存 (kB)
    long available;    // 可用内存 (kB)
    long buffers;      // 缓冲区内存 (kB)
    long cached;       // 缓存内存 (kB)
};

// 步骤2：读取 /proc/meminfo，提取关键字段
MemInfo read_mem_info() {
    // 步骤2.1：打开文件
    std::ifstream file("/proc/meminfo");
    std::string line;
    MemInfo mem = {0, 0, 0, 0};
    
    // 步骤2.2：逐行读取，提取需要的字段
    while (std::getline(file, line)) {
        // 步骤2.3：用 istringstream 解析每一行
        std::istringstream iss(line);
        std::string key;
        long value;
        iss >> key >> value;
        
        // 步骤2.4：根据 key 存入对应字段
        if (key == "MemTotal:") {
            mem.total = value;
        } else if (key == "MemAvailable:") {
            mem.available = value;
        } else if (key == "Buffers:") {
            mem.buffers = value;
        } else if (key == "Cached:") {
            mem.cached = value;
        }
    }
    return mem;
}

// 步骤3：计算内存使用率
double calculate_mem_usage(const MemInfo& mem) {
    // 步骤3.1：如果总内存为0，防止除以零
    if (mem.total == 0) return 0.0;
    
    // 步骤3.2：计算已用内存 = 总内存 - 可用内存
    // 使用 MemAvailable 计算更准确（包含可回收缓存）
    long used = mem.total - mem.available;
    
    // 步骤3.3：返回百分比
    return 100.0 * used / mem.total;
}

// 步骤4：主函数，测试内存采集
int main() {
    std::cout << "🧠 内存监控程序启动..." << std::endl;
    
    while (true) {
        // 步骤4.1：读取内存信息
        MemInfo mem = read_mem_info();
        
        // 步骤4.2：计算使用率
        double usage = calculate_mem_usage(mem);
        
        // 步骤4.3：输出 JSON 格式
        std::cout << "{\"mem_total\": " << mem.total 
                  << ", \"mem_available\": " << mem.available
                  << ", \"mem_usage\": " << usage << "}" << std::endl;
        
        // 步骤4.4：每秒采集一次
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
