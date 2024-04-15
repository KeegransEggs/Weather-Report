//==============Blynk Definition==============
#define BLYNK_PRINT Serial
//#define BLYNK_TEMPLATE_ID "Blynk template ID"
//#define BLYNK_TEMPLATE_NAME "Blynk template name"
//#define BLYNK_AUTH_TOKEN "Blynk authorization token"
//=================Libraries===================
#include <WiFi.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <BlynkSimpleEsp32.h>
#include <LCD-I2C.h>
//=================LCD Display=================
LCD_I2C lcd(0x27, 16, 2);
//=====================BME=====================
Adafruit_BME280 bme;
#define BME280_I2C_ADDRESS 0x77
//====================WIFI=====================
//const char* ssid     = "name of network";
//const char* password = "password of network";
//const char* auth     = "Blynk Template ID";
//===================Interval==================
const unsigned long BLYNK_INTERVAL = 1000;
const unsigned long SERIAL_INTERVAL = 10000;
const unsigned long LCD_INTERVAL = 5000;
//=============================================
BlynkTimer timer;
//=============================================
  void setup() {
    Serial.begin(115200);
    delay(2000);

    Wire.begin(11,10);

    bme.begin(BME280_I2C_ADDRESS);

    Blynk.begin(auth, ssid, password);
    timer.setInterval(BLYNK_INTERVAL, sendSensor);

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
}
void loop() {
  static unsigned long previousBlynkTime = 0;
  static unsigned long previousSerialTime = 0;
  static unsigned long previousLCDTime = 0;

  unsigned long currentMillis = millis();

  if (currentMillis - previousBlynkTime >= BLYNK_INTERVAL){
    previousBlynkTime = currentMillis;
    sendSensor();
  }

  if (currentMillis - previousSerialTime >= SERIAL_INTERVAL) {
    previousSerialTime = currentMillis;
    sendSensor();
  }

  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0;

  if (currentMillis - previousLCDTime >= LCD_INTERVAL) {
    previousLCDTime = currentMillis;
   slideText("Temperature (C): ", temperature);
    slideText("Humidity (%): ", humidity);
    slideText("Pressure (hPa): ", pressure);
  }

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
  Serial.print("Humidity: " );
  Serial.print(h);
  Serial.println(" %");
  Serial.print("Pressure: ");
  Serial.print(ap);
  Serial.println(" hPa");
  Serial.println(" ");
}
  void slideText(String label, float value) 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(label);
  lcd.setCursor(0, 1);
  lcd.print(value);
  delay(5000);

  for (int i = 0; i < label.length() + 1; i++) 
  {
    lcd.scrollDisplayLeft();
    delay(200);
  }
 }
