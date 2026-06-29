#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>

struct MemInfo {
    long total, available, buffers, cached;
};

MemInfo read_mem_info() {
    std::ifstream file("/proc/meminfo");
    std::string line;
    MemInfo mem = {0, 0, 0, 0};

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        long value;
        iss >> key >> value;

        if (key == "MemTotal:") mem.total = value;
        else if (key == "MemAvailable:") mem.available = value;
        else if (key == "Buffers:") mem.buffers = value;
        else if (key == "Cached:") mem.cached = value;
    }
    return mem;
}

double calculate_mem_usage(const MemInfo& mem) {
    if (mem.total == 0) return 0.0;
    return 100.0 * (mem.total - mem.available) / mem.total;
}

int main() {
    // 以追加模式打开管道
    std::ofstream pipe("/tmp/monitor.pipe", std::ios::app);
    if (!pipe.is_open()) {
        std::cerr << "无法打开管道 /tmp/monitor.pipe" << std::endl;
        return 1;
    }

    while (true) {
        MemInfo mem = read_mem_info();
        double usage = calculate_mem_usage(mem);

        // 写入管道（加上 flush 确保立即发送）
        pipe << "{\"type\": \"memory\", \"total\": " << mem.total
             << ", \"available\": " << mem.available
             << ", \"usage\": " << usage << "}" << std::endl;
        pipe.flush();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
