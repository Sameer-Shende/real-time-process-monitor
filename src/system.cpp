#include "system.h"

#include <unistd.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// Return the system's CPU
Processor &System::Cpu() { return cpu_; }

// Return a container composed of the system's processes
vector<Process> &System::Processes() {
  UpdateProcesses();
  return processes_;
}

std::string System::Kernel() { return LinuxParser::Kernel(); }

float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

long int System::UpTime() { return LinuxParser::UpTime(); }

void System::UpdateProcesses() {
  processes_.clear();
  for (int pid : LinuxParser::Pids()) {
    UpdateProcess(pid);
  }
  sort(processes_.begin(), processes_.end());
}

void System::UpdateProcess(int pid) {
  Process process(pid);
  processes_.emplace_back(process);
}
