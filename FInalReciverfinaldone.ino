#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>

// Display Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RoboEyes<Adafruit_SSD1306> roboEyes(display);

// NRF24
RF24 radio(7, 8);
const byte address[5] = {'0', '0', '0', '0', '1'};

// Servo
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVO_MIN 150
#define SERVO_MAX 600

// Motors Down (Pins)
const int RmotorF = 26; const int RmotorB = 27;
const int LmotorF = 24; const int LmotorB = 25;
const int speed1 = 6;   const int speed2 = 5;

// Motors Up (Pins)
const int URmotorF = 31; const int URmotorB = 32;
const int ULmotorF = 33; const int ULmotorB = 34;
const int speed3 = 46;   const int speed4 = 44;

// Other Pins
const int buzz = 4;
const int battery = 10;
const int cameraPin = 9;
const int lightsPin = 11;

struct JoystickData {
  byte yValueM;
  bool buttonPressedR;
  bool buttonPressedL;
  bool buttonPressedB;
  bool buttonPressedC;
  bool buttonPressedLights;
  byte CamXValue;
  byte gripperValue;
  byte xValue;
  byte yValue;
  bool buttonPressed;
  bool Way3UpRight;
  bool Way3DownRight;
  bool homeArm;
  bool Way2DownLeft;
  bool Way2UpLeft;
};

JoystickData receivedData;
bool connectionEstablished = false;

// Arm Variables
int baseAngle = 90, shoulderAngle = 90, elbowAngle = 90;
int wristAngle = 90, wristRotateAngle = 90, gripperAngle = 90, cameraAngle = 90;
int deadLow = 110, deadHigh = 145;
bool lastHomeState = false, lastButtonCState = false, lastButtonLightsState = false;
bool cameraOn = false, lightsOn = false;

void setup() {
  Serial.begin(115200);
  int outputPins[] = {RmotorF, RmotorB, LmotorF, LmotorB, speed1, speed2, URmotorF, URmotorB, ULmotorF, ULmotorB, speed3, speed4, buzz, battery, cameraPin, lightsPin};
  for (int pin : outputPins) pinMode(pin, OUTPUT);
  
  digitalWrite(battery, LOW);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  pwm.begin();
  pwm.setPWMFreq(50);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for (;;);
  introScreen();

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 3);
}

void loop() {
  display.clearDisplay();
  if (radio.available()) {
    if (!connectionEstablished) {
      connectionEstablished = true;
      flashLights(2);
    }
    radio.read(&receivedData, sizeof(receivedData));

    handleToggles();
    controlMotors();
    controlArm();

    if (receivedData.buttonPressedB) {
      roboEyes.anim_laugh();
      tone(buzz, 1000);
    } else {
      noTone(buzz);
    }
  }
  roboEyes.update();
  display.display();
}

void controlMotors() {
  int y = receivedData.yValueM;
  bool turnR = receivedData.buttonPressedR;
  bool turnL = receivedData.buttonPressedL;
  int speed = 0;

  // Determine base speed from Joystick
  if (y < 115) speed = map(y, 0, 114, 255, 0);
  else if (y > 140) speed = map(y, 141, 255, 0, 255);
  
  // If we are turning, we want a fixed turning speed if not moving, 
  // or use the current joystick speed if we are moving.
  if ((turnR || turnL) && speed < 100) speed = 180; 

  if (speed == 0 && !turnL && !turnR) {
    stopMotors();
    return;
  }

  // Define Motor Directions
  bool leftF = false, leftB = false, rightF = false, rightB = false;

  if (turnL) { // Spin Left (Left side back, Right side forward)
    leftB = true; rightF = true;
  } else if (turnR) { // Spin Right (Right side back, Left side forward)
    rightB = true; leftF = true;
  } else if (y < 115) { // Forward
    leftF = rightF = true;
  } else if (y > 140) { // Backward
    leftB = rightB = true;
  }

  // Apply Directions to Pins
  digitalWrite(LmotorF, leftF); digitalWrite(ULmotorF, leftF);
  digitalWrite(LmotorB, leftB); digitalWrite(ULmotorB, leftB);
  digitalWrite(RmotorF, rightF); digitalWrite(URmotorF, rightF);
  digitalWrite(RmotorB, rightB); digitalWrite(URmotorB, rightB);

  // Apply PWM Speed
  analogWrite(speed1, speed); analogWrite(speed2, speed);
  analogWrite(speed3, speed); analogWrite(speed4, speed);
}

void stopMotors() {
  digitalWrite(RmotorF, LOW); digitalWrite(RmotorB, LOW);
  digitalWrite(LmotorF, LOW); digitalWrite(LmotorB, LOW);
  digitalWrite(URmotorF, LOW); digitalWrite(URmotorB, LOW);
  digitalWrite(ULmotorF, LOW); digitalWrite(ULmotorB, LOW);
  analogWrite(speed1, 0); analogWrite(speed2, 0);
  analogWrite(speed3, 0); analogWrite(speed4, 0);
}

void controlArm() {
  int speed = receivedData.Way2UpLeft ? 1 : (receivedData.Way2DownLeft ? 4 : 2);
  int x = receivedData.xValue;
  int y = receivedData.yValue;

  if (receivedData.Way3UpRight) {
    if (x > deadHigh) baseAngle += speed; if (x < deadLow) baseAngle -= speed;
    if (y > deadHigh) shoulderAngle += speed; if (y < deadLow) shoulderAngle -= speed;
  } else if (receivedData.Way3DownRight) {
    if (x > deadHigh) elbowAngle += speed; if (x < deadLow) elbowAngle -= speed;
    if (y > deadHigh) wristAngle += speed; if (y < deadLow) wristAngle -= speed;
  } else {
    if (x > deadHigh) wristRotateAngle += speed; if (x < deadLow) wristRotateAngle -= speed;
  }

  baseAngle = constrain(baseAngle, 20, 160);
  shoulderAngle = constrain(shoulderAngle, 20, 160);
  elbowAngle = constrain(elbowAngle, 20, 170);
  wristAngle = constrain(wristAngle, 20, 160);
  wristRotateAngle = constrain(wristRotateAngle, 0, 180);
  
  if (receivedData.homeArm && !lastHomeState) {
    baseAngle = shoulderAngle = elbowAngle = wristAngle = wristRotateAngle = 90;
  }
  lastHomeState = receivedData.homeArm;

  setServoAngle(0, baseAngle);
  setServoAngle(1, shoulderAngle);
  setServoAngle(2, elbowAngle);
  setServoAngle(3, wristAngle);
  setServoAngle(5, wristRotateAngle);
  setServoAngle(4, map(receivedData.gripperValue, 0, 255, 20, 160));
  setServoAngle(6, map(receivedData.CamXValue, 0, 255, 0, 180));
}

void handleToggles() {
  if (receivedData.buttonPressedC && !lastButtonCState) {
    cameraOn = !cameraOn;
    digitalWrite(cameraPin, cameraOn ? HIGH : LOW);
    roboEyes.anim_laugh();
    tone(buzz, 1000, 300);
  }
  lastButtonCState = receivedData.buttonPressedC;

  if (receivedData.buttonPressedLights && !lastButtonLightsState) {
    lightsOn = !lightsOn;
    digitalWrite(lightsPin, lightsOn ? HIGH : LOW);
    roboEyes.anim_laugh();
    tone(buzz, 1000, 300);
  }
  lastButtonLightsState = receivedData.buttonPressedLights;
}

void setServoAngle(uint8_t kanal, uint8_t ugao) {
  pwm.setPWM(kanal, 0, map(constrain(ugao, 0, 180), 0, 180, SERVO_MIN, SERVO_MAX));
}

void flashLights(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(lightsPin, HIGH); delay(200);
    digitalWrite(lightsPin, LOW); delay(200);
  }
}

void introScreen() {
  display.clearDisplay();
  display.setTextSize(2); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 13); display.println(F("SEMAX&&TA"));
  display.setTextSize(1); display.setCursor(37, 40); display.println(F("RICKY 2.0"));
  display.display();
  delay(3000);
}