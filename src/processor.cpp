#include "processor.h"
#include "linux_parser.h"

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() {
  auto stats = LinuxParser::CpuUtilization();

  auto newIdle = std::stol(stats[LinuxParser::kIdle_]) +
                 std::stol(stats[LinuxParser::kIOwait_]);
  auto newTotal = newIdle + std::stol(stats[LinuxParser::kUser_]) +
                  std::stol(stats[LinuxParser::kNice_]) +
                  std::stol(stats[LinuxParser::kSystem_]) +
                  std::stol(stats[LinuxParser::kIRQ_]) +
                  std::stol(stats[LinuxParser::kSoftIRQ_]) +
                  std::stol(stats[LinuxParser::kSteal_]);

  auto idle = newIdle - prevIdle;
  auto total = newTotal - prevTotal;

  prevIdle = idle;
  prevTotal = total;

  return (float)(total - idle) / total;
}