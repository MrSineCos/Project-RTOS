#ifndef __TEMP_HUMI_MONITOR__
#define __TEMP_HUMI_MONITOR__
#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "DHT20.h"
#include <SimpleDHT.h>
#include "global.h"

#define DHT_PIN GPIO_NUM_6 // GPIO pin where the DHT11 is connected

#define DHT_TYPE_DHT20 1
#define DHT_TYPE_DHT11 2

#define DHT_TYPE DHT_TYPE_DHT11

void temp_humi_monitor(void *pvParameters);


#endif