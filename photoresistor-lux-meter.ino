#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Wire.h>

#define LDR_PIN 2

// LDR Characteristics

const float GAMMA = 0.7;
const float RL10 = 50;
const int MQ_PIN=A1;                                
int RL_VALUE=5;                                  
float RO_CLEAN_AIR_FACTOR=9.83; 
int CALIBARAION_SAMPLE_TIMES=50;               
int CALIBRATION_SAMPLE_INTERVAL=500;                                           
int READ_SAMPLE_INTERVAL=50;                        
int READ_SAMPLE_TIMES=5;
Servo servo;  
int servoPosition = 0;
bool isShutterUp = false;
const int relayPin = 2;
const int tempPin = A3;
const int lightControl=12; 
const int gasSensorPin = A1;
const int fan = 5;
const int buzzer = 13;
const int windowOverridePin = 7;
boolean windowOverriden = false;
int windowState = 0;
int buttonState = 0;
float lux, sensorValue;
float temp;
float LPGCurve[3] = {2.3,0.21,-0.47};
float COCurve[3]  =  {2.3,0.72,-0.34};
float SmokeCurve[3] ={2.3,0.53,-0.44};
float Ro = 10;
long iPPM_LPG = 0;
long iPPM_CO = 0;
long iPPM_Smoke = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(lightControl,OUTPUT);
  digitalWrite(lightControl,LOW);
  Serial.begin(9600);
  Serial.print("Calibrating gas Sensor...\n");  
  Ro = MQCalibration(MQ_PIN);
  Serial.print("Done!\n");
  lcd.begin();
  lcd.backlight();
  servo.attach(2);
  servo.write(0); 
  pinMode(windowOverridePin, INPUT_PULLUP);
  pinMode(fan,OUTPUT);
  pinMode(0,OUTPUT);
  pinMode(1,OUTPUT);
  pinMode(tempPin, INPUT);
  pinMode(buzzer, OUTPUT);
  //delay(20000);
}

float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}

float MQCalibration(int mq_pin)
{
  int i;
  float val=0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {          
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                  
  val = val/RO_CLEAN_AIR_FACTOR;                                                          
  return val;                                             

}

float getLux() {
  int analogValue = analogRead(A0);
  float voltage = analogValue / 1024. * 5;
  float resistance = 2000 * voltage / (1 - voltage / 5);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
  return lux;
}

float MQRead(int mq_pin)
{
  int i;
  float rs=0;
 
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
 
  rs = rs/READ_SAMPLE_TIMES;
 
  return rs;  
}

long MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == 0 ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == 1 ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == 2 ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    
 
  return 0;
}

long  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}

float readTemp() { 
int temp1 = analogRead(tempPin);
float temp_val = (temp1 * 4.88);
 temp_val = (temp_val/10); 
return temp_val; 
}

void light(float lux){
  if (lux > 2000) {
      digitalWrite(lightControl, HIGH);
    } else {
      digitalWrite(lightControl, LOW);
    }
}

void shutterControl(int mode) {
  if (mode == 0) {
    for (servoPosition = 0; servoPosition <= 180; servoPosition += 1) {
          servo.write(servoPosition);         
          delay(15); 
    }
  }
  else { 
     for (servoPosition = 180; servoPosition >= 0; servoPosition -= 1) {
          servo.write(servoPosition);         
          delay(15); 
        }
  }
}

void buzzerControl() {
  if (iPPM_CO >= 10 || iPPM_LPG >= 15 || iPPM_Smoke >= 51 || sensorValue >= 150) {
    tone(buzzer, 1000); 
    delay(1000);  
    noTone(buzzer);
  }
}

void fanControl() {

  if (temp > 40) 
    digitalWrite(9, HIGH);
  else
    digitalWrite(9, LOW); 
}

void printer() {
  Serial.print(lux);
  Serial.print("\t");
  Serial.print("Temp = ");
  Serial.print(temp);
  Serial.print(" C\t");
  Serial.print("Gas value: ");
  Serial.print(sensorValue);
  Serial.print("\n");
  Serial.print("Concentration of gas : ");
  Serial.print(iPPM_LPG);
  Serial.print("ppm\t");
  Serial.print(iPPM_CO);
  Serial.print("ppm\t");
  Serial.print(iPPM_Smoke);
  Serial.print("ppm\n");
}

void getSensorReadings() {
  lux = getLux();
  temp = readTemp();
  sensorValue = analogRead(gasSensorPin);
  iPPM_LPG = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,0);
  iPPM_CO = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,1);
  iPPM_Smoke = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,2);
}

void loop() {
  lcd.print("HERE");
  lcd.setCursor(0, 0);
  lcd.print("Room: ");
  lcd.setCursor(0, 1);
  lcd.print("Lux: ");
  getSensorReadings();
  printer();
  fanControl();
  buzzerControl();
  if (lux > 1000) {
      Serial.print("Shutter up\n");
      if (!isShutterUp) {
        Serial.print("Raising shutter\n");
        shutterControl(0);
        isShutterUp = true;
      }
      light(lux);
      delay(100); 
      
    } else {
      Serial.print("Shutter down\n");
      if (isShutterUp){
        Serial.print("Lowering shutter\n");
        shutterControl(1);
        isShutterUp = false;
      }
      light(lux);
    }
  //Serial.print(buttonState);
  //Serial.print(" ");
  //Serial.print(windowOverriden);
  //Serial.print("\n");
}
