#define BTN_PIN 2
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
const float accelThreshold = 1.5; // 낙상 감지를 위한 가속도 임계값 (g 단위)
const float gyroThreshold = 250; // 낙상 감지를 위한 자이로 임계값 (degrees/sec)
unsigned long lastMillis = 0;
unsigned long fallDetectedMillis = 0;
bool fallDetected = false;

float previousAccX = 0, previousAccY = 0, previousAccZ = 0;
const float alpha = 0.5; // 필터 계수


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // OLED 초기화
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // I2C 주소를 0x3C로 설정
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000); // 초기 화면 유지 시간
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers

  AcX = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  float accX = AcX / 16384.0;
  float accY = AcY / 16384.0;
  float accZ = AcZ / 16384.0;

  // 이동 평균 필터 적용
  accX = alpha * accX + (1 - alpha) * previousAccX;
  accY = alpha * accY + (1 - alpha) * previousAccY;
  accZ = alpha * accZ + (1 - alpha) * previousAccZ;

  previousAccX = accX;
  previousAccY = accY;
  previousAccZ = accZ;

  // 중력 요소 제거 (assuming device is flat initially)
  accZ -= 1.0; // assuming Z-axis is aligned with gravity

  float accelMagnitude = sqrt(accX * accX + accY * accY + accZ * accZ);

  if (accelMagnitude > accelThreshold) {
    if (abs(GyX) > gyroThreshold || abs(GyY) > gyroThreshold || abs(GyZ) > gyroThreshold) {
      unsigned long currentMillis = millis();
      if (currentMillis - lastMillis > 1000) { // 낙상 이벤트를 중복 감지하지 않기 위한 시간 간격 설정
        Serial.println("낙상감지");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("warring!");
        display.display();
        fallDetected = true;
        fallDetectedMillis = currentMillis;
        lastMillis = currentMillis;
      }
    }
  }
  if (fallDetected) {
    if (digitalRead(BTN_PIN) == 1) {
      if (millis() - fallDetectedMillis > 3000) { // 버튼이 3초 동안 눌려있을 경우
        Serial.println("낙상감지 확인");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("ok");
        display.display();
        fallDetected = false; // 확인 후 상태 초기화
      }
    } else {
      // 버튼이 눌리지 않으면 타이머 초기화
      fallDetectedMillis = millis();
    }
  }
  delay(100); // 데이터 읽기 주기
}
