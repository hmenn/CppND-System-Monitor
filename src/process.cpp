#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

// DONE: Return this process's ID
int Process::Pid() { return this->pid; }

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() const {
  auto clk = sysconf(_SC_CLK_TCK);

  return ((float)(LinuxParser::ActiveJiffies(this->pid) / (float)clk) /
          LinuxParser::UpTime(this->pid));
}

// DONE: Return the command that generated this process
string Process::Command() {
  const int MAX_CMD_LEN = 40;
  auto cmd = LinuxParser::Command(this->pid);
  if (cmd.size() > MAX_CMD_LEN) {
    cmd = cmd.substr(0, MAX_CMD_LEN).append("...");
  }
  return cmd;
}

// DONE: Return this process's memory utilization
string Process::Ram() {
  auto ramKB = LinuxParser::Ram(this->pid);
  return ramKB.substr(
      0, ramKB.size() - 3);  // Remove last 3 digit to convert KB->MB
}

// DONE: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(this->pid); }

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(this->pid); }

// DONE: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return this->CpuUtilization() < a.CpuUtilization();
}