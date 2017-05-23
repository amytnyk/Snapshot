#pragma once
#include <string>
#include <fstream>
#include <time.h>

class Log
  {
  private:
    std::ofstream o;
  public:
    Log() : o("ScreenshotLog.txt")
      {

      }
    void LogF(const std::string& error)
      {
      time_t t = time(0);
      struct tm * now = localtime(&t);
      std::string message = std::to_string(now->tm_mday) + "_" + std::to_string(now->tm_hour) + ":" + std::to_string(now->tm_min) + ":" + std::to_string(now->tm_sec) + " - ";
      message += error;
      o << message << std::endl;
      }
  };
