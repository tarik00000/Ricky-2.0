#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <PCF8575.h>   // PCF8575 I/O Expander

// ============================================================
//                     PCF8575 I2C SETUP
// ============================================================
PCF8575 pcf8575(0x20);   // Default I2C address

// ============================================================
//                        NRF24 SETUP
// ============================================================
RF24 radio(7, 8);        // CE, CSN
const byte address[5] = {'0','0','0','0','1'};

// ============================================================
//                     DC MOTOR CONTROLS
// ============================================================
const int VRy_PINM = A2;     // Motor joystick Y-axis
const int BtnBuzz  = 10;    // Buzzer button

// PCF8575 pins
const int camera_PCF_PIN = 2;
const int lights_PCF_PIN = 3;
const int BaseRotateShoulder  = 1;
const int WristRotateFineJoint = 0;
const int NormalSpeed = 5;

const int SlowPrecise = 9;
// Direction buttons
const int RBtn = 6;
const int LBtn = 3;

// ============================================================
//                        ARM CONTROLS
// ============================================================
const int VRx_PIN   = A1;
const int VRy_PIN   = A3;

const int home   = 5;

int gripper = A6;
int cameraX = A0;
// ============================================================
//                     STATUS & BUZZER
// ============================================================
const int OnLed = 4;
const int buzz  = 2;

// ============================================================
//                   JOYSTICK DATA STRUCT
// ============================================================
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

JoystickData joystickData;

// ============================================================
//                            SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  // ---- I2C / PCF8575 ----
  Wire.begin();
  if (pcf8575.begin()) {
    Serial.println(F("PCF8575 found."));
  } else {
    Serial.println(F("PCF8575 not found."));
  }

  // ---- Inputs ----
  pinMode(BtnBuzz, INPUT_PULLUP);
  
  pinMode(home, INPUT_PULLUP);
  pinMode(SlowPrecise, INPUT_PULLUP);
  pinMode(LBtn, INPUT_PULLUP);
  pinMode(RBtn, INPUT_PULLUP);

  

  // ---- Outputs ----
  pinMode(OnLed, OUTPUT);
  pinMode(buzz, OUTPUT);

  // ---- NRF24 ----
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println(F("Transmitter ready."));

  // ---- Startup Beep ----
  tone(buzz, 1000);
  digitalWrite(OnLed, HIGH);
  delay(300);

  digitalWrite(OnLed, LOW);
  noTone(buzz);
  delay(500);

  digitalWrite(OnLed, HIGH);
  tone(buzz, 1000);
  delay(300);

  digitalWrite(OnLed, LOW);
  tone(buzz, 1000);
  delay(200);
  noTone(buzz);
}

// ============================================================
//                            LOOP
// ============================================================
void loop() {
  // ---------------- Read Motor Joystick ----------------
  int rawY_Motor = analogRead(VRy_PINM);
  joystickData.yValueM = map(rawY_Motor, 0, 1023, 0, 255);

  // Direction & Buzzer buttons
  joystickData.buttonPressedR = (digitalRead(RBtn) == LOW);
  joystickData.buttonPressedL = (digitalRead(LBtn) == LOW);
  joystickData.buttonPressedB = (digitalRead(BtnBuzz) == LOW);

  // ---------------- Read PCF8575 buttons ----------------
  joystickData.buttonPressedC      = (pcf8575.read(camera_PCF_PIN) == LOW);
  joystickData.buttonPressedLights = (pcf8575.read(lights_PCF_PIN) == LOW);
  joystickData.Way3UpRight         = (pcf8575.read(BaseRotateShoulder) == LOW);
  joystickData.Way3DownRight       = (pcf8575.read(WristRotateFineJoint) == LOW);
  joystickData.Way2UpLeft          = (pcf8575.read(NormalSpeed) == LOW);
  // ---------------- Read Arm Joystick ----------------
  joystickData.xValue = map(analogRead(VRx_PIN), 0, 1023, 0, 180);
  joystickData.yValue = map(analogRead(VRy_PIN), 0, 1023, 0, 180);

  // ---------------- Read Gripper & Camera ----------------
  int rawGripper = analogRead(gripper);
  int rawCamX    = analogRead(cameraX);
  joystickData.gripperValue = map(rawGripper, 0, 1023, 0, 180);
  joystickData.CamXValue    = map(rawCamX, 0, 1023, 0, 180);

  // ---------------- Read Open/Close Buttons ----------------
  joystickData.homeArm  = (digitalRead(home) == LOW);
  joystickData.Way2DownLeft = (digitalRead(SlowPrecise) == LOW);

  // ---------------- Transmit ----------------
  bool transmissionOK = radio.write(&joystickData, sizeof(joystickData));
  digitalWrite(OnLed, transmissionOK ? HIGH : LOW);

  // ---------------- Debug Output ----------------
  Serial.println(F("========== JOYSTICK DATA =========="));
  Serial.print(F("Motor Y: ")); Serial.println(joystickData.yValueM);
  
  Serial.print(F("R Btn: ")); Serial.print(joystickData.buttonPressedR ? "Pressed" : "Released");
  Serial.print(F(" | L Btn: ")); Serial.print(joystickData.buttonPressedL ? "Pressed" : "Released");
  Serial.print(F(" | Buzz Btn: ")); Serial.println(joystickData.buttonPressedB ? "Pressed" : "Released");

  Serial.print(F("Camera Btn: ")); Serial.print(joystickData.buttonPressedC ? "Pressed" : "Released");
  Serial.print(F(" | Lights Btn: ")); Serial.println(joystickData.buttonPressedLights ? "Pressed" : "Released");

  Serial.print(F("Way3 UpRight: ")); Serial.print(joystickData.Way3UpRight ? "Yes" : "No");
  Serial.print(F(" | Way3 DownRight: ")); Serial.println(joystickData.Way3DownRight ? "Yes" : "No");

  Serial.print(F("Arm X: ")); Serial.print(joystickData.xValue);
  Serial.print(F(" | Arm Y: ")); Serial.println(joystickData.yValue);

  Serial.print(F("Home: ")); Serial.print(joystickData.homeArm ? "Pressed" : "Released");
  Serial.print(F(" | Slow: ")); Serial.println(joystickData.Way2DownLeft ? "Pressed" : "Released");
  Serial.print(F(" | Normal: ")); Serial.println(joystickData.Way2UpLeft ? "Pressed" : "Released");

  Serial.print(F("Gripper: ")); Serial.print(joystickData.gripperValue);
  Serial.print(F(" | CamX: ")); Serial.println(joystickData.CamXValue);

  Serial.println(F("==================================\n"));

  delay(50);
}

