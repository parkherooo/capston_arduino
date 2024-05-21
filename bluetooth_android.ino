#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PulseSensorPlayground.h>
#include <SoftwareSerial.h>    
// OLED 디스플레이 설정
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial bluetooth(3,4);    
// 심박수 센서 설정
const int PulseWire = A0;       // 심박수 센서의 아날로그 핀 번호
const int LED13 = 13;           // 내장 LED 핀
const int Threshold = 550;      // 심박수 임계값

PulseSensorPlayground pulseSensor;

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600); 
  // OLED 디스플레이 초기화
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // 무한루프
  }

  // 심박수 센서 설정
  pulseSensor.analogInput(PulseWire);
  pulseSensor.blinkOnPulse(LED13);
  pulseSensor.setThreshold(Threshold);

  // 심박수 센서 시작
  if (pulseSensor.begin()) {
    Serial.println("Pulse Sensor 시작");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
}

void loop() {
  int myBPM = pulseSensor.getBeatsPerMinute();  // 심박수 측정
  boolean beatDetected = pulseSensor.sawStartOfBeat();  // 새로운 비트 감지 확인
  display.clearDisplay();

  // 심박수 출력
  display.setCursor(15, 20);
  display.setTextSize(2);  // 글자 크기를 키움
  display.print("BPM : ");
  display.print(myBPM);
  bluetooth.print("BPM : ");
  bluetooth.println(myBPM);
  display.display();
  delay(1000); // 1초 대기
}