#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  long prevIdle = 0;
  long prevTotal = 0;
};

#endif