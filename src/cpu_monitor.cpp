#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>

struct CpuTime {
    long user;
    long nice;
    long system;
    long idle;
    long iowait;
    long irq;
    long softirq;
    long steal;
};

CpuTime read_cpu_time() {
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file,line);

    std::string label;
    CpuTime cpu;
    std::istringstream iss(line);
    iss >> label >> cpu.user >> cpu.nice >> cpu.system >> cpu.idle >> cpu.iowait
      >> cpu.irq >> cpu.softirq >> cpu.steal;

    return cpu;
}

double calculate_cpu_usage(const CpuTime& prev, const CpuTime& curr) {
    long prev_total = prev.user + prev.nice + prev.system + prev.idle +
    prev.iowait + prev.irq + prev.softirq + prev.steal;
    long curr_total = curr.user + curr.nice + curr.system + curr.idle+ curr.iowait + curr.irq + curr.softirq + curr.steal;

    long total_diff = curr_total - prev_total;
    long idle_diff = curr.idle - prev.idle;

    if(total_diff == 0) return 0.0;

    return 100.0 * (total_diff - idle_diff) / total_diff;
}

int main() {
    
    // 打开管道
    std::ofstream pipe("/tmp/monitor.pipe", std::ios::app);
    if(!pipe.is_open()) {
        std::cerr << "无法打开管道 /tmp/monitor.pipe" << std::endl;
        return 1;
    }

    CpuTime prev = read_cpu_time();

    while(true) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
          CpuTime curr = read_cpu_time();
          double usage = calculate_cpu_usage(prev, curr);
          prev = curr;

          // 写入管道
          pipe << "{\"type\": \"cpu\", \"usage\": " << usage << "}" << std::endl;
          pipe.flush();  // 确保数据立即写入
          // std::cout << "{\"cpu_usage\": " << usage << "}" << std::endl;
    }

    return 0;
}
