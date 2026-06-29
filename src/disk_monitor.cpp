#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

struct DiskStats {
    std::string name;
    long reads, reads_sectors, writes, writes_sectors;
    long io_in_progress, read_time, write_time;
};

std::vector<DiskStats> read_disk_stats() {
    std::ifstream file("/proc/diskstats");
    std::string line;
    std::vector<DiskStats> disks;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        DiskStats ds = {"", 0, 0, 0, 0, 0, 0, 0};
        int major, minor;
        iss >> major >> minor >> ds.name
            >> ds.reads >> ds.reads_sectors >> ds.writes >> ds.writes_sectors
            >> ds.io_in_progress >> ds.read_time >> ds.write_time;

        if ((ds.name.find("sd") == 0 || ds.name.find("vd") == 0)
            && ds.name.length() == 3) {
            disks.push_back(ds);
        }
    }
    return disks;
}

int main() {
    std::ofstream pipe("/tmp/monitor.pipe", std::ios::app);
    if (!pipe.is_open()) {
        std::cerr << "无法打开管道 /tmp/monitor.pipe" << std::endl;
        return 1;
    }

    while (true) {
        std::vector<DiskStats> stats = read_disk_stats();

        for (const auto& ds : stats) {
            pipe << "{\"type\": \"disk\", \"name\": \"" << ds.name
                 << "\", \"reads\": " << ds.reads
                 << ", \"writes\": " << ds.writes
                 << ", \"read_sectors\": " << ds.reads_sectors
                 << ", \"write_sectors\": " << ds.writes_sectors << "}" << std::endl;
            pipe.flush();
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
