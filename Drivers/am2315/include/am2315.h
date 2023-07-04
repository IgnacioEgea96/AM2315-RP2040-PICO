#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define AM2315_I2CADDR 0x5C
#define AM2315_READREG 0x03

void init_i2c();
bool init_am2315();
void scan_i2c();
bool read_data();
float readTemperature();
float readHumidity();
bool readTemperatureAndHumidity(float *t, float *h);
