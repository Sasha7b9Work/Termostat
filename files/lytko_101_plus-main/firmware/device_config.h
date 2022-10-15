#ifndef DEVCONF_H
#define DEVCONF_H

#include "Arduino.h"

#define PORT_IN(X) pinMode(X, INPUT)
#define PORT_OUT(X) pinMode(X, OUTPUT)
#define PORT_LOW(X) digitalWrite(X, LOW);
#define PORT_HIGH(X) digitalWrite(X, HIGH);

#define WDT_PIN   16
#define RELAY_PIN 15

#endif
