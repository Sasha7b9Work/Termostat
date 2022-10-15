#pragma once

class Wdt {
  public:
    static void setup();
    static void enable();
    static void disable();
  private:
    Wdt() {}
};
