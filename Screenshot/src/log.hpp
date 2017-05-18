#pragma once
#include <string>
#include <fstream>
#include <time.h>

class Log
{
public:
  void LogF(std::string error)
  {
    std::ifstream i("log.txt");
    std::string str((std::istreambuf_iterator<char>(i)), std::istreambuf_iterator<char>());
    i.close();
    time_t t = time(0);
    struct tm * now = localtime(&t);
    std::string name = std::to_string(now->tm_mday) + "_" + std::to_string(now->tm_hour) + ":" + std::to_string(now->tm_min) + ":" + std::to_string(now->tm_sec) + " - " + error + "\n";
    std::ofstream o("log.txt");
    o << str + name;
    o.close();
  }
};
