#include <Wire.h>
#include <U8g2lib.h>

// MPU-6050 센서 I2C 주소
const int MPU_addr = 0x68; 
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
const float accelThreshold = 1.5; // 낙상 감지 가속도 임계값
const float gyroThreshold = 250; // 낙상 감지 자이로 임계값
unsigned long lastMillis = 0;
unsigned long fallDetectedMillis = 0;
bool fallDetected = false;
float previousAccX = 0, previousAccY = 0, previousAccZ = 0;
const float alpha = 0.5; // 필터 계수

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// 심박수 센서 핀 설정
const int pulsePin = A0; 
int pulseValue = 0;
unsigned long lastPulseTime = 0;
unsigned long displayHeartRateUntil = 0; // 심박수 표시 시간

// OLED 디스플레이 설정
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  u8g2.begin();
  u8g2.enableUTF8Print();
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 레지스터
  Wire.write(0);     // 0으로 설정 (MPU-6050 깨우기)
  Wire.endTransmission(true);
  Serial.begin(9600);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // 시작 시 OLED에 표시
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_korean1);
  u8g2.setCursor(10, 40);
  u8g2.print("설정 완료");
  u8g2.sendBuffer();
  Serial.println("Setup ok");
}

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // ACCEL_XOUT_H 레지스터부터 시작
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // 총 14개 레지스터 요청

  // 가속도 및 자이로 데이터 읽기
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Tmp = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();

  // 데이터 필터링
  float accX = alpha * (AcX / 16384.0) + (1 - alpha) * previousAccX;
  float accY = alpha * (AcY / 16384.0) + (1 - alpha) * previousAccY;
  float accZ = alpha * (AcZ / 16384.0) + (1 - alpha) * previousAccZ;
  previousAccX = accX;
  previousAccY = accY;
  previousAccZ = accZ;
  accZ -= 1.0; // 중력 요소 제거

  // 낙상 감지
  float accelMagnitude = sqrt(accX * accX + accY * accY + accZ * accZ);
  if (accelMagnitude > accelThreshold && (abs(GyX) > gyroThreshold || abs(GyY) > gyroThreshold || abs(GyZ) > gyroThreshold)) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastMillis > 1000) {
      Serial.println("낙상감지");
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_unifont_t_korean1);
      u8g2.setCursor(10, 40);
      u8g2.print("!!위험 감지!!");
      u8g2.sendBuffer();              
      fallDetected = true;
      fallDetectedMillis = currentMillis;
      lastMillis = currentMillis;
    }
  }

  // 심박수 센서 데이터 처리
  int readValue = analogRead(pulsePin);
  if (readValue > 600) {
    unsigned long currentTime = millis();
    if (currentTime - lastPulseTime > 200) {
      pulseValue = 60000 / (currentTime - lastPulseTime);
      lastPulseTime = currentTime;
      displayHeartRateUntil = currentTime + 3000; // 3초간 심박수 표시
    }
  }

  // 심박수 표시
  if (millis() < displayHeartRateUntil) {
    Serial.print("Heart Rate: ");
    Serial.println(pulseValue);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_unifont_t_korean1);
    u8g2.setCursor(0, 20);
    u8g2.print("BPM: ");
    u8g2.print(pulseValue);
    u8g2.sendBuffer();
  }

  delay(100);
}
