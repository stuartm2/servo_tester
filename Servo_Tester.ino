
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Servo.h>

#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);
Servo myservo;  // create servo object to control a servo

static const int SERVO_PIN = 9;
static const int MODEBTN_PIN = 2; //7 Interrupt
static const int POSINPUT_PIN = A0; // A0

int MODE_NULL = 0;
int MODE_CENTRE = 1;
int MODE_MANUAL = 2;
int MODE_SWEEP = 3;

int curMode = 0;
bool changingMode = false;
int pos = 90;
int sweepDir = 1;

void setup() {
  pinMode(MODEBTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MODEBTN_PIN), onModeBtnPressed, RISING);
  
  // Initialize display with the I2C addr
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  if (changingMode && curMode != 0 && !myservo.attached()) {
    myservo.attach(SERVO_PIN);
  }
  
  if (curMode == MODE_CENTRE) {
    centreServo();
    
    display.println("Mode:\n Centre");
  } else if (curMode == MODE_SWEEP) {
    if (changingMode) {
      pos = 90;
    }
    
    sweepServo();
    
    display.println("Mode:\n Sweep");
    display.print("Angle:\n ");
    display.println(abs(pos - 180));
  } else if (curMode == MODE_MANUAL) {
    followServo();
    
    display.println("Mode:\n Manual");
    display.print("Angle:\n ");
    display.println(abs(pos - 180));
  } else {
    if (changingMode) {
      myservo.detach();
    }
    
    display.println("Mode:\n None");
  }

  changingMode = false;

  display.drawLine(28, 57, 128, 57, WHITE);

  for (int i = 7; i < 57; i += 4) {
    display.drawPixel(78, i, WHITE);
  }

  if (curMode != 0) {
    float rad = abs(pos - 180) * (PI / 180);
    int base = 78 - (50 * cos(rad));
    int rise = 57 - (50 * sin(rad));
    
    display.drawLine(78, 57, base, rise, WHITE);
  }
  
  display.display();
}

void onModeBtnPressed() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    curMode += 1;
    changingMode = true;

    if (curMode > MODE_SWEEP) {
      curMode = 0;
    }
  }
  
  last_interrupt_time = interrupt_time;
}

void sweepServo() {
  int multiplier = 5;
  
  if (pos <= 0 && sweepDir < 0) {
    sweepDir = 1;
  } else if (pos >= 180 && sweepDir > 0) {
    sweepDir = -1;
  }

  pos += (sweepDir * multiplier);
  myservo.write(pos);
  delayMicroseconds(50);
}

void followServo() {
  int val = analogRead(POSINPUT_PIN);

  if (val <= 20) {
    val = 0; 
  }

  if (val >= 1003) {
    val = 1023;
  }

  pos = map(val, 0, 1023, 0, 180);
  myservo.write(pos);
  delay(15);
}

void centreServo() {
  pos = 90;
  myservo.write(pos);
  delay(15);
}

