#define BLYNK_TEMPLATE_ID "Template ID"
#define BLYNK_TEMPLATE_NAME "Template Name"
#define BLYNK_AUTH_TOKEN "Auth Token"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <SimpleTimer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
char ssid[] = "Wi-Fi SSID";  //เปลี่ยนชื่อ และรหัสผ่าน Wi-Fi และทำการ upload code ใหม่
char pass[] = "Wi-Fi Pass";

// Blynk authentication
char auth[] = BLYNK_AUTH_TOKEN;

// 2CH Onboard Relay
#define Relay1_sw1 32
#define Widget_Btn_sw1 V2
#define Relay2_sw2 33
#define Widget_Btn_sw2 V3


// กำหนดขา GPIO ที่เชื่อมต่อกับ DHT11
#define DHTPIN 18  // GPIO 18

// กำหนดประเภทของเซนเซอร์ DHT
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Simple Timer
SimpleTimer timer;

// LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Timing variables
unsigned long previousMillisLCD = 0;
const long intervalLCD = 5000;  // Update LCD every 5 seconds

//=================Blynk Virtual Pin=================//
//V0  Temperature
//V1  Humidity
//V2  Sw1
//V3  Sw2
//=================Blynk Virtual Pin=================//

//Setup Function
void setup() {
  Serial.begin(9600);
  // LCD
  lcd.begin();
  lcd.backlight();

  // Relay Mode
  pinMode(Relay1_sw1, OUTPUT);
  pinMode(Relay2_sw2, OUTPUT);

  // Initial relay status
  digitalWrite(Relay1_sw1, HIGH);
  digitalWrite(Relay2_sw2, HIGH);


  // เริ่มต้นเซนเซอร์ DHT
  dht.begin();  // ฟังก์ชันนี้จะใช้งานได้หลังจากติดตั้งไลบรารีที่ถูกต้อง
  // แสดงข้อความเริ่มต้นที่ต้องการให้คงอยู่ตลอด
  displayInitialLCDMessages();  // แสดงข้อความที่ไม่ต้องการให้หายไป

  // Function: Connect to Wi-Fi
  initWiFi();

  timer.setInterval(5000L, updatedht11sensor);  // Update sensor data every 5 seconds


  Blynk.config(auth);
  delay(1000);  // Short delay to ensure Wi-Fi and Blynk connection
}

//Loop Function
void loop() {
  Blynk.run();  // Run Blynk connection
  timer.run();  // Run the SimpleTimer to update sensors regularly

  unsigned long currentMillis = millis();

  // Update LCD every 5 seconds
  if (currentMillis - previousMillisLCD >= intervalLCD) {
    previousMillisLCD = currentMillis;
    displaySensorDataOnLCD();  // Update only the sensor data
  }
}

// Function connect to Wi-Fi
void initWiFi() {
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());
}

// Function Check connect to Blynk-IoT
void checkBlynkStatus() {
  if (!Blynk.connected()) {
    Serial.println("Blynk Not Connected");
  }
}

//If blynk connected sync all virtual pin
BLYNK_CONNECTED() {
  Blynk.syncAll();
}

// Function to read DHT11 sensor: Temperature & Humidity
void readdht11sensor(float &temp1, float &humi1) {
  humi1 = dht.readHumidity();
  temp1 = dht.readTemperature();  // ค่าอุณหภูมิเป็นองศาเซลเซียส

  if (isnan(humi1) || isnan(temp1)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // ส่งค่าไปยัง Blynk
  Blynk.virtualWrite(V0, temp1);  // ส่งค่าอุณหภูมิไปยัง Virtual Pin V0
  Blynk.virtualWrite(V1, humi1);  // ส่งค่าความชื้นไปยัง Virtual Pin V1
}

// สร้างฟังก์ชันใหม่สำหรับ SimpleTimer เพื่ออัปเดตข้อมูลเซ็นเซอร์
void updatedht11sensor() {
  float temp1, humi1;
  readdht11sensor(temp1, humi1);  // เรียกใช้ฟังก์ชันเพื่ออ่านค่าจากเซ็นเซอร์ DHT11
}

// Display data on LCD
void displayInitialLCDMessages() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("*===SMFThailand===*");

  lcd.setCursor(2, 1);
  lcd.print("Tel:081-xxx-xxxx");
}

// BTN ON/OFF Relay1 (active-low logic)
BLYNK_WRITE(Widget_Btn_sw1) {
  digitalWrite(Relay1_sw1, param.asInt() == 1 ? LOW : HIGH);  // Active-low
}

// BTN ON/OFF Relay2 (active-low logic)
BLYNK_WRITE(Widget_Btn_sw2) {
  digitalWrite(Relay2_sw2, param.asInt() == 1 ? LOW : HIGH);  // Active-low
}

// แสดงข้อมูลเซ็นเซอร์ในบรรทัดที่ 3 และ 4
void displaySensorDataOnLCD() {
  // ไม่เคลียร์หน้าจอ เพื่อให้ข้อความที่บรรทัด 1 และ 2 ยังคงอยู่
  float humi1 = dht.readHumidity();
  float temp1 = dht.readTemperature();  // ค่าอุณหภูมิเป็นองศาเซลเซียส

  // แสดงข้อมูลเซ็นเซอร์ในบรรทัดที่ 3 และ 4
  lcd.setCursor(0, 2);  // Start at row 3, column 0
  lcd.print("T:");
  lcd.print(temp1);
  lcd.setCursor(8, 2);
  lcd.print("C");

  lcd.setCursor(10, 2);
  lcd.print("Hum:");
  lcd.print(humi1);
  lcd.setCursor(19, 2);
  lcd.print("%");
}
