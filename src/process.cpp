#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() { return _pid; }

float Process::CpuUtilization() { return _cpuUtilization; }

string Process::Command() { return _command; }

string Process::Ram() { return _ram; }

string Process::User() { return _user; }

long int Process::UpTime() { return _upTime; }

// operator to sort processes based on CPU Utilization
bool Process::operator<(Process const &a) const {
  if (_cpuUtilization > a._cpuUtilization)
    return true;
  else
    return false;
}

Process::Process(int pid) {
  _pid = pid;
  _user = LinuxParser::User(_pid);
  _command = LinuxParser::Command(_pid);
  _ram = LinuxParser::Ram(_pid);
  _upTime = LinuxParser::UpTime(_pid);
  _cpuUtilization = LinuxParser::CpuUtilization(_pid);
}
