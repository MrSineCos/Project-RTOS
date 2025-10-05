#include "temp_humi_monitor.h"

#if DHT_TYPE == DHT_TYPE_DHT20
    DHT20 dht20;
#elif DHT_TYPE == DHT_TYPE_DHT11
    SimpleDHT11 dht11(DHT_PIN);
#endif

LiquidCrystal_I2C lcd(33,16,2);


void temp_humi_monitor(void *pvParameters){
    #if DHT_TYPE == DHT_TYPE_DHT20
        Wire.begin(11, 12);
        dht20.begin();
    #endif
    
    Serial.begin(115200);

    while (1){
        /* code */
        // Reading temperature and humidity from DHT20
        #if DHT_TYPE == DHT_TYPE_DHT20
            dht20.read();
            float temperature = dht20.getTemperature();
            float humidity = dht20.getHumidity();
            // Check if any reads failed and exit early
            if (isnan(temperature) || isnan(humidity)) {
                Serial.println("Failed to read from DHT sensor!");
                temperature = humidity =  -1;
                //return;
            }
        #elif DHT_TYPE == DHT_TYPE_DHT11
            float temperature = 0;
            float humidity = 0;
            int err = dht11.read2(&temperature, &humidity, NULL);
        #endif

        if (err != SimpleDHTErrSuccess) {
            Serial.print("Read DHT11 failed, err=");
            Serial.println(err);
            temperature = humidity = -1;
            //return;
        }

        //Update global variables for temperature and humidity
        glob_temperature = temperature;
        glob_humidity = humidity;

        // Print the results
        
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
        
        vTaskDelay(5000);
    }
    
}