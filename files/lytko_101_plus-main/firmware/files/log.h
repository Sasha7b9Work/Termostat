#ifndef LOG_H
#define LOG_H

class Log
{
  public:
    static void print(String str) { }
    static void print(int str) { }

    static void println(String str) { }
    static void println(int str) { }

    static void printf(String str, ...) { }

  private:
    Log() {}
};

#endif
