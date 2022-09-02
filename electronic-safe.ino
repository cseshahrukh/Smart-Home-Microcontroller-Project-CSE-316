#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include "SafeState.h"
#include "icons.h"

#define SERVO_PIN 6
#define SERVO_LOCK_POS   0
#define SERVO_UNLOCK_POS 180
Servo lockServo;

/* Display */
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* Keypad setup */
int servoPosition = 0;
int c = 0;
boolean doorClosed = true;
const int echoPin = 12;
const int trigPin = 13;
boolean manushDekhaGeche=false; 
boolean personEntered=false; 
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {8, 9, 10, 11};
const byte buttonPin = 13;
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
SafeState safeState;

void lock() {
  if (!doorClosed) {
    //Serial.println("Inside lock function before for loop"); 

    /*
  for (servoPosition = 180; servoPosition >= 0; servoPosition -= 1) {
    
          lockServo.write(servoPosition);         
          delay(15); 
  
  }
  */
  lockServo.write(0); 
  doorClosed = true;
  
}
safeState.lock();
}

void showStartupMessage() {
  //lcd.setCursor(4, 0);
  Serial.print("Welcome!\n");
  boolean set = false;
  while (!set) {
    set = setNewCode();
  }
  //lockServo.write(90);
  doorClosed = true;
}

String inputSecretCode() {
  //lcd.setCursor(5, 1);
  Serial.print("[____]\n");
  //lcd.setCursor(6, 1);
  String result = "";
  String toPrint = "[";
  while (result.length() < 4) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      toPrint += "*";
      result += key;
    }
  }
  toPrint += "]\n";
  Serial.print(toPrint);
  return result;
}

void showWaitScreen(int delayMillis) {
  //lcd.setCursor(2, 1);
  Serial.print("[..........]\n");
  //lcd.setCursor(3, 1);
  for (byte i = 0; i < 10; i++) {
    delay(delayMillis);
    //Serial.print("=");
  }
  Serial.print("\n");
}

bool setNewCode() {
  //lcd.clear();
  //lcd.setCursor(0, 0);
  Serial.print("Enter First Time Code:\n");
  String newCode = inputSecretCode();

  //lcd.clear();
  //lcd.setCursor(0, 0);
  Serial.print("Confirm new code\n");
  String confirmCode = inputSecretCode();

  if (newCode.equals(confirmCode)) {
    safeState.setCode(newCode);
    Serial.print("Passcode set\n");
    return true;
  } else {
    //lcd.clear();
    //lcd.setCursor(1, 0);
    Serial.print("Code mismatch\n");
    //lcd.setCursor(0, 1);
    Serial.print("Door not locked!\n");
    return false;
  }
}

void showUnlockMessage() {
  //lcd.clear();
  //lcd.setCursor(0, 0);
  //lcd.write(ICON_UNLOCKED_CHAR);
  //lcd.setCursor(4, 0);
  
  //lcd.setCursor(15, 0);
  //lcd.write(ICON_UNLOCKED_CHAR);
  delay(1000);
}

void safeUnlockedLogic() {
  //lcd.clear();

  //lcd.setCursor(0, 0);
  //lcd.write(ICON_UNLOCKED_CHAR);
  //lcd.setCursor(2, 0);
  lcd.print(" # to lock\n");
  //lcd.setCursor(15, 0);
  //lcd.write(ICON_UNLOCKED_CHAR);

  bool newCodeNeeded = true;

  //if (safeState.hasCode()) {
    //lcd.setCursor(0, 1);
    //lcd.print("  A = new code");
  //  newCodeNeeded = false;
  //}

  auto key = keypad.getKey();
  while (key != 'A' && key != '#') {
    key = keypad.getKey();
  }

  bool readyToLock = true;
  //if (key == 'A' || newCodeNeeded) {
    //readyToLock = setNewCode();
  //}

  if (readyToLock) {
    //lcd.clear();
    //lcd.setCursor(5, 0);
    //lcd.write(ICON_UNLOCKED_CHAR);
    //lcd.print(" ");
    //lcd.write(ICON_RIGHT_ARROW);
    //lcd.print(" ");
    //lcd.write(ICON_LOCKED_CHAR);

    safeState.lock();
    lock();
    showWaitScreen(100);
  }
}

void funcwithCount(int n) {
  lockServo.write(n);
  delay(100); 
  //Serial.println(n);
  c++;
}

void safeLockedLogic() {
  //lcd.clear();
  //lcd.setCursor(0, 0);
  //lcd.write(ICON_LOCKED_CHAR);
  //sonarSensor();
  Serial.print(" Door Locked! \n");
  //lcd.write(ICON_LOCKED_CHAR);
  String userCode; 
  //for(int i=0; i<10000000; i++)
  userCode = inputSecretCode();
  Serial.println("given data is "+userCode); 
  bool unlockedSuccessfully = safeState.unlock(userCode);
  showWaitScreen(20);
  
  if (unlockedSuccessfully) {
    personEntered=true; 
    Serial.print("Unlocked!\n");
    //if (doorClosed)
      //funcwithCount(90); //door khulchi
      if(doorClosed)
      {
        for(int i=0; i<180; i++)
        {
          lockServo.write(i); 
          delay(15); 
        }
          
        
      }
    doorClosed = false;
    delay(1000);
    //funcwithCount(0);//abar lock kore dilam 
    if(!doorClosed)
    {
      for(int i=180; i>0; i--)
      {
        lockServo.write(i); 
        delay(15); 
      }
          
      
    }
    delay(700); 
    doorClosed=true; 
    //safeState.lock();
  } else {
    //lcd.clear();
    //lcd.setCursor(0, 0);
    Serial.print("Access Denied!\n");
    showWaitScreen(10);
  }
}

void setup() {
  lcd.begin();
  init_icons(lcd);

  lockServo.attach(SERVO_PIN);
  lockServo.write(-20);
  Serial.begin(9600);
  safeState.lock();
  showStartupMessage();
  //safeState.setCode("1111");
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void sonarSensor() {
  //Serial.println("INside sonar sensor"); 
  digitalWrite(trigPin, LOW);
  delay(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm\n");
  if (distance < 5.00 && doorClosed) {
    for(int i=0; i<=180; i++)
      {lockServo.write(i); delay(15); }
    //lockServo.write(180);
    doorClosed=false; 
    personEntered=false; 
    delay(5000);
    
  }
  if (!doorClosed) {
    doorClosed = true;
    //Serial.print("Looping");
    for(int i=180; i>=0; i--)
      {lockServo.write(i); delay(15); }
    //lockServo.write(0);

    
  }
}

void loop() {
  if(personEntered)
    sonarSensor();
  else
  {
    safeLockedLogic(); 
  }
  //sonarSensor();
  
    //Serial.print("Original boss loop er condition true"); 
    //Serial.print(c);
    //string userCode1 = inputSecretCode();

  
    //safeLockedLogic();
  
    
    //Serial.print("Outside");
    delay(1000);
  
  //else {
    //safeUnlockedLogic();
  //}
}
