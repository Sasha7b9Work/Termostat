#include "wdt.h"
#include "device_config.h"

void Wdt::setup() {
  PORT_OUT(WDT_PIN);
}

void Wdt::enable() {
  PORT_HIGH(WDT_PIN);
}

void Wdt::disable() {
  PORT_LOW(WDT_PIN);
}
