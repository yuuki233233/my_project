#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

// 步骤1：定义磁盘数据结构
struct DiskStats {
    std::string name;      // 磁盘名称（如 sda）
    long reads;            // 读次数
    long reads_sectors;    // 读扇区数
    long writes;           // 写次数
    long writes_sectors;   // 写扇区数
    long io_in_progress;   // 正在进行的 IO
    long read_time;        // 读耗时 (ms)
    long write_time;       // 写耗时 (ms)
};

// 步骤2：读取 /proc/diskstats，过滤出真实磁盘
std::vector<DiskStats> read_disk_stats() {
    // 步骤2.1：打开文件
    std::ifstream file("/proc/diskstats");
    std::string line;
    std::vector<DiskStats> disks;
    
    // 步骤2.2：逐行解析
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        DiskStats ds = {"", 0, 0, 0, 0, 0, 0, 0};
        int major, minor;
        
        // 步骤2.3：提取 11 个字段（从内核文档中定义）
        // 格式：major minor name reads reads_sectors writes writes_sectors ...
        iss >> major >> minor >> ds.name 
            >> ds.reads >> ds.reads_sectors >> ds.writes >> ds.writes_sectors
            >> ds.io_in_progress >> ds.read_time >> ds.write_time;
        
        // 步骤2.4：过滤掉回环设备和分区（只保留真正的磁盘）
        // 条件：名称以 sd（SATA）或 vd（虚拟磁盘）开头，且无数字后缀
        if ((ds.name.find("sd") == 0 || ds.name.find("vd") == 0) 
            && ds.name.length() == 3) {
            disks.push_back(ds);
        }
    }
    return disks;
}

// 步骤3：计算两次采样之间的读写速率
void calculate_disk_io(const std::vector<DiskStats>& prev, 
                       const std::vector<DiskStats>& curr) {
    // 步骤3.1：假设两个向量大小相同，按名称匹配
    for (size_t i = 0; i < prev.size() && i < curr.size(); ++i) {
        // 步骤3.2：计算差值（1秒内的变化）
        long read_diff = curr[i].reads - prev[i].reads;
        long write_diff = curr[i].writes - prev[i].writes;
        long read_sector_diff = curr[i].reads_sectors - prev[i].reads_sectors;
        long write_sector_diff = curr[i].writes_sectors - prev[i].writes_sectors;
        
        // 步骤3.3：扇区转 KB（通常是 512 字节/扇区）
        double read_kb = read_sector_diff * 0.5;   // 扇区 * 512 / 1024
        double write_kb = write_sector_diff * 0.5;
        
        // 步骤3.4：输出 JSON
        std::cout << "{\"disk\": \"" << curr[i].name 
                  << "\", \"read_kb\": " << read_kb 
                  << ", \"write_kb\": " << write_kb
                  << ", \"read_ops\": " << read_diff
                  << ", \"write_ops\": " << write_diff << "}" << std::endl;
    }
}

// 步骤4：主函数，测试磁盘采集
int main() {
    std::cout << "💾 磁盘监控程序启动..." << std::endl;
    
    // 步骤4.1：第一次采样
    std::vector<DiskStats> prev = read_disk_stats();
    
    while (true) {
        // 步骤4.2：等待 1 秒
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 步骤4.3：第二次采样
        std::vector<DiskStats> curr = read_disk_stats();
        
        // 步骤4.4：计算并输出 IO 速率
        calculate_disk_io(prev, curr);
        
        // 步骤4.5：更新 prev 为当前值
        prev = curr;
    }
    return 0;
}
