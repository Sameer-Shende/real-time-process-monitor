#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// -------- Macros ---------------------------------------------------------------------------------------------------
#define ll                     long long int
#define ull                     unsingned long long
#define ff                      first
#define ss                      second
#define pii                     pair<int,int>
#define vii                     vector<int>
#define usii                    unordered_set<int>
#define sii                     set<int>
#define mii                     map<int, int>
#define umii                    unordered_map<int, int>
#define effi(x)                 x.max_load_factor(0.2);     // takes greater space to avoid collisions
#define pb                      push_back
#define sp                      " "
#define yep                     cout<<"YES"<<endl;
#define nope                    cout<<"NO"<<endl;
#define neg1                    cout<<-1<<endl;
#define boost                   ios_base::sync_with_stdio(false);cin.tie(NULL);cout.tie(NULL);
#define forl(i,n)               for(int i=0;i<n;i++)
#define itr(i,a,b)              for(int i=a;(b>=a)?i<b:i>b;(b>=a)?i++:i--)
#define print_arr2d(arr,n,m)    forl(i,n){forl(j,m){cout<<arr[i][j]<<' ';}cout<<endl;}
#define input(arr,n)            forl(i,n){cin>>arr[i];}
#define input_arr2d(arr, n, m)  forl(i,n){forl(j,m){cin>>arr[i][j];}}
#define sprec(x,y)              fixed<<setprecision(y)<<x
#define all(v)                  v.begin(),v.end()
#define rall(v)                 v.rbegin(),v.rend()
#define gcd(a,b)                __gcd(a,b)
#define lcm(a,b)                ((a)*(b)) / __gcd((a),(b))
#define min3(a,b,c)             min(c, min(a, b))
#define min4(a,b,c,d)           min(d, min(c, min(a, b)))
#define max3(a,b,c)             max(c, max(a, b))
#define max4(a,b,c,d)           max(d, max(c, max(a, b)))
// --------------------------------------------------------------------------------------------------------------------


// Read data from the filesystem
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

string LinuxParser::Kernel() {
  string os, kernel, sth;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel >> sth;
  }
  return sth;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR *directory = opendir(kProcDirectory.c_str());
  struct dirent *file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string total, free, trash;
  string line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    // first line
    std::getline(stream, line);
    std::istringstream linestream1(line);
    linestream1 >> trash >> total;
    // second line
    std::getline(stream, line);
    std::istringstream linestream2(line);
    linestream2 >> trash >> free;
    // calculations
    float lTotal = stof(total);
    float lFree = stof(free);
    float lUsed = lTotal - lFree;
    return lUsed / lTotal;
  } else {
    return 0.0;
  }
}

// read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  long secondsUptime = 0;
  std::ifstream stream(kProcDirectory + "." + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> secondsUptime;
  }

  return secondsUptime;
}

// Read and return CPU utilization
float LinuxParser::CpuUtilization(int pid) {
  try {
    std::stringstream ss;
    ss << kProcDirectory << pid << kStatFilename;

    std::stringstream ss_uptime;
    ss_uptime << kProcDirectory << kUptimeFilename;

    long hertz = sysconf(_SC_CLK_TCK);

    string utime = GetPropertyFromFile(ss.str(), 13);
    string stime = GetPropertyFromFile(ss.str(), 14);
    string cutime = GetPropertyFromFile(ss.str(), 15);
    string cstime = GetPropertyFromFile(ss.str(), 16);
    string starttime = GetPropertyFromFile(ss.str(), 21);
    string uptime = GetPropertyFromFile(ss_uptime.str(), 0);

    long lutime = stol(utime);
    long lstime = stol(stime);
    long lcutime = stol(cutime);
    long lcstime = stol(cstime);
    long lstarttime = stol(starttime);
    long luptime = stol(uptime);

    long total_time = lutime + lstime;
    total_time = total_time + lcutime + lcstime;
    float seconds = luptime - (lstarttime / hertz);
    float cpu_usage = ((total_time / hertz) / seconds);

    if (cpu_usage > 1) cpu_usage = 0;

    return cpu_usage;

  } catch (...) {
    return 0;
  }
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string processes, desc;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line))  // Read a whole line per cycle
    {
      std::istringstream linestream(line);
      linestream >> desc >> processes;
      if (desc == "processes") {
        return stoi(processes);
      }
    }
  } else {
    return 0;
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string property =
      GetPropertyFromFile(kProcDirectory + kStatFilename, "procs_running");
  if (!property.empty()) {
    return stoi(property);
  }
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::stringstream ss;
  ss << kProcDirectory << pid << kStatCommandline;
  string property = GetPropertyFromFile(ss.str(), 0);
  if (property.size() < 46) {
    property.insert(property.end(), 46 - property.size(), ' ');
  }
  return StringReplace(&property, '\000', ' ');
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  std::stringstream ss;
  ss << kProcDirectory << pid << kStatusFilename;
  std::string mem = GetPropertyFromFile(ss.str(), "VmSize:");

  try {
    float mem2 = stof(mem);
    mem2 = mem2 / (1024*8);

    std::stringstream stream;
    stream << std::fixed << std::setprecision(0) << mem2;
    return stream.str();

  } catch (...) {
    return mem;
  }
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::stringstream ss;
  ss << kProcDirectory << pid << kStatusFilename;
  std::string aux1 = ss.str();
  std::string aux2 = GetPropertyFromFile(ss.str(), "Uid:");
  return GetPropertyFromFile(ss.str(), "Uid:");
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);

  string line, user;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line))  // Read a whole line per cycle
    {
      line = StringReplace(&line, ':', ' ');
      std::istringstream linestream(line);
      using StrIt = std::istream_iterator<std::string>;
      std::vector<std::string> container{StrIt{linestream}, StrIt{}};
      if (container.at(2).compare(uid) == 0) {
        return container.at(0);
      }
    }
  }
  return uid;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  try {
    std::stringstream ss;
    ss << kProcDirectory << pid << kStatFilename;
    std::string property = GetPropertyFromFile(ss.str(), 21);
    if (!property.empty()) {
      long secs = stoi(property);
      long hertz = sysconf(_SC_CLK_TCK);
      return secs / hertz;
    }
  } catch (...) {
    return 0;
  }
  return 0;
}

vector<string> LinuxParser::CpuUtilization() {
  string line, cpu, cpu_time;
  vector<string> cpu_utilizations;
  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu;

    while (linestream >> cpu_time) {
      cpu_utilizations.emplace_back(cpu_time);
    }
  }
  return cpu_utilizations;
}

/*
    The following are the three helper functions to get the property from a file easily
    Here the property name can be given as a parameter along with the file path.
 */

// 1st implementation of the GetPropertyFromFile()
std::string LinuxParser::GetPropertyFromFile(std::string filePath,
                                             std::string propertyName) {
  string line, property, desc;
  std::ifstream stream(filePath);
  if (stream.is_open()) {
    while (std::getline(stream, line))  // Read a whole line per cycle
    {
      std::istringstream linestream(line);
      linestream >> desc >> property;
      if (desc == propertyName) {
        return property;
      }
    }
  }
  return {};
}

// 2nd overloaded Implementation with the position passed as the search
// parameter
std::string LinuxParser::GetPropertyFromFile(std::string filePath,
                                             unsigned long position) {
  string line, property, desc;
  std::ifstream stream(filePath);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    using StrIt = std::istream_iterator<std::string>;
    std::vector<std::string> container{StrIt{linestream}, StrIt{}};
    if (container.size() >= position + 1) {
      return container.at(position);
    }
  }
  return {};
}

// For ease of use for the strings with non-alphabetic characters this function
// is used for a temporary replacement
// of the character
string LinuxParser::StringReplace(string *input, char oldChar, char newChar) {
  std::transform(input->begin(), input->end(), input->begin(),
                 [oldChar, newChar](char c) {
                   if (c != oldChar) {
                     return c;
                   } else {
                     return newChar;
                   }
                 });
  return *input;
}
