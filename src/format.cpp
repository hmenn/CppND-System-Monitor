#include <sstream>
#include <string>

#include "format.h"

using std::string;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  std::stringstream ss;
  ss << seconds / 3600;
  ss << ":" << (seconds % 3600) / 60;
  ss << ":" << seconds % 60;
  return ss.str();
}