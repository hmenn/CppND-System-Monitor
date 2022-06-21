#include <sstream>
#include <string>
#include <iomanip>

#include "format.h"

using std::string;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  std::stringstream ss;
  ss << std::setw(2) << std::setfill('0') << seconds / 3600;
  ss << ":" << std::setw(2) << std::setfill('0') << (seconds % 3600) / 60;
  ss << ":" << std::setw(2) << std::setfill('0') <<seconds % 60;
  return ss.str();
}