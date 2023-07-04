
#include <am2315.h>

float humidity, temp;
uint64_t lastreading;

bool reserved_addr(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void init_i2c() {
    // Enable UART so we can print status output
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#warning i2c/bus_scan example requires a board with I2C pins
    puts("Default I2C pins were not defined");
#else
    // This example will use I2C0 on the default SDA and SCL pins (GP4, GP5 on a Pico)
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    lastreading = 0;
#endif
}

bool init_am2315(){
  init_i2c();
  return read_data();
}

void scan_i2c(){
    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}

bool read_data() {

  uint8_t reply[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  if (lastreading) {
    if (get_absolute_time()._private_us_since_boot > lastreading) {
      if ((get_absolute_time()._private_us_since_boot - lastreading) <
          2000000) {     // has it been less than 2 seconds since?
        return false; // bail, they need to wait longer!
      }
    } else {
      // get_absolute_time() is less than the last reading, so we wrapped around!
      lastreading = get_absolute_time()._private_us_since_boot;
      return false; // bail again
    }
  }
  lastreading = get_absolute_time()._private_us_since_boot; // reset our timer

  //
  // Using steps from datasheet, s.7.2.4
  //

  // Step one: Wake-up sensor
  // Datasheet is confusing about "wake up". This is adapted from:
  // https://www.switchdoc.com/2016/04/am2315-highly-reliable-esp8266-arduino-driver/
  i2c_write_blocking(i2c_default, AM2315_I2CADDR, reply, 1, false);
  sleep_ms(50);

  // Step two: send read command
  reply[0] = AM2315_READREG; // function code
  reply[1] = 0x00;           // start address
  reply[2] = 0x04;           // number of bytes

  i2c_write_blocking(i2c_default, AM2315_I2CADDR, reply, 3, false);

  // Step three: read data back
  i2c_read_blocking(i2c_default, AM2315_I2CADDR, reply, 8, false);

  if (reply[0] != AM2315_READREG)
    return false;
  if (reply[1] != 4)
    return false; // bytes req'd

  humidity = reply[2];
  humidity *= 256;
  humidity += reply[3];
  humidity /= 10;

  temp = reply[4] & 0x7F;
  temp *= 256;
  temp += reply[5];
  temp /= 10;

  // change sign
  if (reply[4] >> 7)
    temp = -temp;

  return true;
}

float readTemperature() {
  if (!read_data())
    return -1;
  return temp;
}

float readHumidity() {
  if (!read_data())
    return -1;
  return humidity;
}

bool readTemperatureAndHumidity(float *t, float *h) {
  if (!read_data())
    return false;

  *t = temp;
  *h = humidity;

  return true;
}