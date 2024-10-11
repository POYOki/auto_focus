#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display width and height, in pixels
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// HC-SR04 pins
#define TRIG_PIN 7
#define ECHO_PIN 6

// A4988 stepper motor driver pins
#define STEP_PIN 2
#define DIR_PIN 3

// Global variables
bool calibrationComplete = true; // 假设校准已完成
long currentPosition = 0; // 步进电机的当前位置

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize I2C communication
  Wire.begin(4, 5); // SDA = GPIO 4, SCL = GPIO 5

  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Set up HC-SR04 pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Set up A4988 stepper motor driver pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
}

long measureDistance() {
  // Send ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo pulse duration
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate distance in cm
  return duration * 0.034 / 2;
}

void loop() {
  if (calibrationComplete) {
    // 读取距离
    long distance = measureDistance();

    // 在OLED显示屏上显示距离
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Distance: ");
    display.print(distance);
    display.println(" cm");

    // 控制步进电机移动到测量的距离
    long stepsToMove = distance - currentPosition; // 计算需要移动的步数
    if (stepsToMove != 0) {
      digitalWrite(DIR_PIN, stepsToMove > 0 ? HIGH : LOW); // 设置方向
      stepsToMove = abs(stepsToMove);
      for (long i = 0; i < stepsToMove; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(2000); // 根据需要调整延时
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(2000); // 根据需要调整延时
      }
      currentPosition = distance; // 更新当前位置
    }

    // 在OLED显示屏上显示步进电机位置
    display.setCursor(0, 16);
    display.print("Stepper Pos: ");
    display.print(currentPosition);
    display.println(" steps");
    display.display();

    // 延时一段时间再进行下一次测量
    delay(300);
  }
}
