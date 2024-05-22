//==============Blynk Definition==============
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "Your template ID"
#define BLYNK_TEMPLATE_NAME "Your template name"
#define BLYNK_AUTH_TOKEN "Your authorization token"
//=================Libraries===================
#include <WiFi.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <BlynkSimpleEsp32.h>
#include <LCD-I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//=================LCD Display=================
LCD_I2C lcd(0x27, 16, 2);
//=====================BME=====================
Adafruit_BME280 bme;
#define BME280_I2C_ADDRESS 0x77
//====================WIFI=====================
const char* ssid     = "Your WiFi name";
const char* password = "Your WiFi password";
const char* auth     = "Your authorization token";
//===================Interval==================
const unsigned long BLYNK_INTERVAL = 1000;
const unsigned long SERIAL_INTERVAL = 10000;
const unsigned long LCD_INTERVAL = 8000;
//================NTP Client===================
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 10800);
//===================Timers====================
BlynkTimer timer;
//==================Time Variables=============
unsigned long currentTime = 0;
unsigned long lastUpdate = 0;
//=============================================
void setup() {
  Serial.begin(115200);
  delay(2000);
 
  Wire.begin(11, 10);
 
  bme.begin(BME280_I2C_ADDRESS);
 
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(BLYNK_INTERVAL, sendSensor);
  timer.setInterval(60000, checkWeatherWarning); // check weather warning every minute
  timer.setInterval(1000, incrementTime); // increment time every second
  timer.setInterval(LCD_INTERVAL, updateDisplay); // update LCD display every interval
 
  lcd.begin();
  lcd.display();
  lcd.backlight();
  lcd.print("Initializing...");
 
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);
 
  timeClient.begin();
 
  // Update the clock initially
  updateClockFromNTP();
}
 
void loop() {
  Blynk.run();
  timer.run();
}
 
void sendSensor() {
  float t = bme.readTemperature();
  float h = bme.readHumidity();
  float ap = bme.readPressure() / 100.0;
 
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
  Blynk.virtualWrite(V7, ap);
 
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("Pressure: ");
  Serial.print(ap);
  Serial.println(" hPa");
  Serial.println(" ");
}
 
void updateDisplay() {
  static int slideIndex = 0;
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0;
 
  lcd.clear();
  switch (slideIndex) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Temperature (C)");
      lcd.setCursor(0, 1);
      lcd.print(temperature);
      delay(5000); // Show parameter for 5 seconds
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Humidity (%)");
      lcd.setCursor(0, 1);
      lcd.print(humidity);
      delay(5000); // Show parameter for 5 seconds
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Pressure (hPa)");
      lcd.setCursor(0, 1);
      lcd.print(pressure);
      delay(5000); // Show parameter for 5 seconds
      break;
    case 3:
      updateClockFromNTP(); // Update the clock each time before displaying
      lcd.setCursor(0, 0);
      lcd.print("Time:");
      lcd.setCursor(0, 1);
      lcd.print(getFormattedTime());
      delay(5000); // Show parameter for 5 seconds
      break;
  }
  slideIndex = (slideIndex + 1) % 4; // Cycle through the 4 slides
  delay(3000); // Slide transition duration of 3 seconds
}
 
void checkWeatherWarning() {
  float pressure = bme.readPressure() / 100.0;
  static float previousPressure = pressure;
  float pressureDrop = previousPressure - pressure;
  previousPressure = pressure;
 
  if (pressureDrop > 3.0) { // Adjust the threshold as needed
    Serial.println("Warning: Rapid pressure drop detected! Potential bad weather.");
    Blynk.logEvent("pressure_drop_warning", "Rapid pressure drop detected! Potential bad weather.");
  }
}
 
void updateClockFromNTP() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  currentTime = epochTime % 86400; // seconds since midnight
}
 
void incrementTime() {
  currentTime++;
  if (currentTime >= 86400) { // One day in seconds
    updateClockFromNTP(); // Update from NTP at midnight
  }
}
 
String getFormattedTime() {
  unsigned long hours = currentTime / 3600;
  unsigned long minutes = (currentTime % 3600) / 60;
 
  char timeString[9];
  snprintf(timeString, sizeof(timeString), "%02lu:%02lu", hours, minutes);
  return String(timeString);
}
