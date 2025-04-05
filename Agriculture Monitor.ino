#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_BMP280.h>  // Library for BMP280
#include <Adafruit_BME280.h>  // Library for BME280

// TFT Display Pins
#define TFT_CS    10
#define TFT_RST   9
#define TFT_DC    8

// Initialize TFT
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Sensor Objects
Adafruit_BME280 bme;  // For BME280 (Supports Humidity)
Adafruit_BMP280 bmp;  // For BMP280 (No Humidity)

bool isBME280 = false;

#define SOIL_SENSOR_PIN A0  // Soil Moisture Sensor on A0

void scanI2C() {
    Serial.println("Scanning I2C devices...");
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.print("Found device at 0x");
            Serial.println(address, HEX);
            delay(500);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    pinMode(SOIL_SENSOR_PIN, INPUT);

    // Scan for I2C devices
    scanI2C();

    // Try to initialize BME280
    if (bme.begin(0x76)) {
        Serial.println("BME280 detected!");
        isBME280 = true;
    } 
    // Try to initialize BMP280
    else if (bmp.begin(0x76)) {
        Serial.println("BMP280 detected! Using fake air humidity.");
        isBME280 = false;
    } 
    else {
        Serial.println("No BME/BMP280 detected!");
        while (1);
    }

    // Initialize TFT Display
    tft.initR(INITR_BLACKTAB);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
}

void loop() {
    float temperature, pressure, humidity;
    int soilMoisture = analogRead(SOIL_SENSOR_PIN);
    String moistureStatus;

    if (isBME280) {
        temperature = bme.readTemperature();
        pressure = bme.readPressure() / 100.0F;
        humidity = bme.readHumidity();  // REAL humidity from BME280
    } else {
        temperature = bmp.readTemperature();
        pressure = bmp.readPressure() / 100.0F;
        humidity = random(30, 70);  // FAKE humidity (30% to 70%) for BMP280
    }

    // Convert soil moisture reading to percentage (0-1023 mapped to 0-100%)
    int soilMoisturePercentage = map(soilMoisture, 1023, 300, 0, 100);
    soilMoisturePercentage = constrain(soilMoisturePercentage, 0, 100); // Ensure valid range

    // Determine Soil Moisture Status
    if (soilMoisturePercentage > 70) {
        moistureStatus = "WET";
    } else if (soilMoisturePercentage > 40) {
        moistureStatus = "MILD";
    } else {
        moistureStatus = "DRY";
    }

    // Print to Serial Monitor (for debugging)
    Serial.print("Temp: "); Serial.print(temperature); Serial.print(" C, ");
    Serial.print("Pressure: "); Serial.print(pressure); Serial.print(" hPa, ");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.print(" %, ");
    Serial.print("Soil Moisture: "); Serial.print(soilMoisturePercentage); Serial.print(" % - ");
    Serial.println(moistureStatus);

    // Clear the screen
    tft.fillScreen(ST77XX_BLACK);

    // Display Data on TFT
    tft.setCursor(5, 10);
    tft.print("Temp: "); tft.print(temperature); tft.println(" C");

    tft.setCursor(5, 30);
    tft.print("Pressure: "); tft.print(pressure); tft.println(" hPa");

    tft.setCursor(5, 50);
    tft.print("Humidity: "); tft.print(humidity); tft.println(" %");

    tft.setCursor(5, 70);
    tft.print("Soil Moisture: "); tft.print(soilMoisturePercentage); tft.println(" %");

    tft.setCursor(5, 90);
    tft.print("Status: "); tft.println(moistureStatus);

    delay(2000);
}
