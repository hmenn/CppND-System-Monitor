#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

// requires a higher version of g++
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
// requires target_link_libraries(... stdc++fs)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

template <typename Key, typename Value>
Value ExtractDataFromFile(const std::string& file, const Key& key) {
  string line;
  Key key_;
  Value value;
  std::ifstream filestream(file);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key_ >> value) {
        if (key_ == key) {
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// DONE: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;

  namespace fs = std::experimental::filesystem;
  for (const auto& entry : fs::directory_iterator(kProcDirectory)) {
    string filename = entry.path().filename().string();
    if (std::all_of(filename.begin(), filename.end(), isdigit)) {
      int pid = stoi(filename);
      pids.push_back(pid);
    }
  }
  return pids;
}

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  auto memTotalKB = ExtractDataFromFile<string, long>(
      kProcDirectory + kMeminfoFilename, "MemTotal:");
  auto memFreeKB = ExtractDataFromFile<string, long>(
      kProcDirectory + kMeminfoFilename, "MemFree:");
  return (memTotalKB - memFreeKB) / (float)memTotalKB;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  long totalUptimeSeconds;
  long totalIdleSeconds;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    filestream >> totalUptimeSeconds >> totalIdleSeconds;
  }
  return totalUptimeSeconds;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  string line;
  string junkData;
  long totalJiff = 0;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::getline(filestream, line);
  std::istringstream linestream(line);
  linestream >> junkData;

  constexpr int jiffCounter = 8;
  for (int i = 0; i < jiffCounter; ++i) {
    long val;
    linestream >> val;
    totalJiff += val;
  }

  return totalJiff;
}

// DONE: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);
  string line;
  string junkData;
  int readStartIndex = 13;
  long totalJiff = 0;

  std::getline(filestream, line);
  std::istringstream linestream(line);
  for (int i = 0; i < readStartIndex; ++i) {
    linestream >> junkData;
  }

  // read utime, stime, cutime , cstime sequentially
  long val;
  for (int i = 0; i < 4; ++i) {
    linestream >> val;
    totalJiff += val;
  }
  return totalJiff;
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto stats = CpuUtilization();
  return std::stol(stats[kUser_]) + std::stol(stats[kNice_]) +
         std::stol(stats[kSystem_]) + std::stol(stats[kIRQ_]) +
         std::stol(stats[kSoftIRQ_]) + std::stol(stats[kSteal_]);
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto stats = CpuUtilization();
  return std::stol(stats[kIdle_]) + std::stol(stats[kIOwait_]);
}

// DONE: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> stats;
  string line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  std::getline(filestream, line);

  string junkData;
  string stat;
  std::istringstream linestream(line);
  linestream >> junkData;
  while (linestream >> stat) {
    stats.push_back(stat);
  }

  return stats;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  return ExtractDataFromFile<string, int>(kProcDirectory + kStatFilename,
                                          "processes");
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return ExtractDataFromFile<string, int>(kProcDirectory + kStatFilename,
                                          "procs_running");
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  return line;
}

// DONE: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  // I used VmRSS key instead of VmSize, because VmSize gives sum of all virtual
  // memory
  // Please check corresponding man page for detailed information:
  // - man proc |grep -C 5 VmSize
  // - man proc |grep -C 5 VmRSS
  return ExtractDataFromFile<string, string>(
      kProcDirectory + std::to_string(pid) + kStatusFilename, "VmRSS:");
}

// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  return ExtractDataFromFile<string, string>(
      kProcDirectory + std::to_string(pid) + kStatusFilename, "Uid:");
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  auto uid_exp = LinuxParser::Uid(pid);

  string line;
  string user;
  string junk;
  string uid_actual;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> junk >> uid_actual) {
        if (uid_actual == uid_exp) {
          return user;
        }
      }
    }
  }
  return user;
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);

  string line;

  std::getline(filestream, line);
  std::istringstream iss(line);
  string uptimeStr;
  for (int i = 0; i < 22; ++i) {
    iss >> uptimeStr;
  };

  // TODO: Its' clock tics. convert this to seconds
  return std::stoi(uptimeStr) / sysconf(_SC_CLK_TCK);
}
