/*
 Authours:
 Sebastian Andersson
 Gustaf Carlstedt
 Ulrik Åkesson
  
  Analog:
  A0 - Sensor signal
  A1 - Potentiometer signal for reference
  A2 - Potentiometer signal for PWM
  Digital:
  D9 - PWM output

  BLDC:
  Red - Vcc, run at 7V
  Black - Ground
  Green - PWM, 0-5V
  Yellow - Do not use*/ 
  
#include <Servo.h>
#define VOLTS_PER_UNIT    .0049F
#define LOWER_ANGLE       16
#define UPPER_ANGLE       29
#define MOTORSPEED        82 //70 is full throttle
#define DELAY             1
  
Servo myservo;
  
int sensorpin = A0; // analog pin used to connect the sharp sensor
int potPinRef = A1;  // analog pin used to connect potentiometer
int potPinPWM = A2;
int minPipe = 10; // Min value of pipe in cm
int maxPipe = 30;  // Max value of pipe in cm
int PWM = 9 ; // PWM output
  
void analogWrite25k(int pin, int value) {
    switch (pin) {
        case 9:
            OCR1A = value;
            //Serial.print(value);
            break;
        case 10: 
            OCR1B = value;
            break;
        default:
            // no other pin will work
            break;
    }
}
  
int PotentiometerReadPWM() {
  int potVal, PWM;
  potVal = analogRead(potPinPWM);  // Read poetentiometer value
  PWM = map(potVal, 0, 1023, 0, 255);
  
  return PWM;
  }
  
  int PotentiometerReadRef() {
  int potVal, ref;
  potVal = analogRead(potPinRef);  // Read poetentiometer value
  ref = map(potVal, 0, 1023, maxPipe, minPipe);
  
  return ref;
  }
  
  int SensorRead() {
    int i, cm=0;
    float sensorVal=0;
    
    for (i=0; i<10; i++) {
      sensorVal = analogRead(sensorpin); //sensorVal; // + (analogRead(sensorpin)*VOLTS_PER_UNIT);       // reads the value of the sharp sensor
    } 
    //Serial.println(sensorVal);
    cm =  4800/(sensorVal - 20);//21.25/(sensorVal-0.175);  //60.495 * pow((sensorVal/10),-1.1904);
    return cm;
}
  
void setup(){
  pinMode(3, OUTPUT); // Output pin for OCR2B
   // Configure Timer 1 for PWM @ 25 kHz.
   TCCR1A = 0;           // undo the configuration done by...
   TCCR1B = 0;           // ...the Arduino core library
   TCNT1  = 0;           // reset timer
   TCCR1A = _BV(COM1A1)  // non-inverted PWM on ch. A 
          | _BV(COM1B1)  // same on ch; B
          | _BV(WGM11);  // mode 10: ph. correct PWM, TOP = ICR1
   TCCR1B = _BV(WGM13)   // ditto
          | _BV(CS10);   // prescaler = 1
   ICR1   = 320;         // TOP = 320
  
   // Set up the 250 kHz output (but cro measures only 125 kHz)
   TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
   TCCR2B = _BV(WGM22) | _BV(CS20);
   OCR2A = 255;
   OCR2B = 0;
   myservo.attach(5);
   // Set the PWM pins as output.
   pinMode(PWM, OUTPUT);
    
  Serial.begin(9600); // starts the serial monitor
}
  
void loop(){
  int err;
  double error;
  int prevError = 0, deltaError = 0;
  int8_t cm, ref, i, angle, changeInAngle;
  double normError, normDeltaError;
  double  eN = 0, eZ = 0, eP = 0, deN = 0, deZ = 0, deP = 0;
  double ruleP = 0, ruleZ = 0, ruleN = 0;
  double areaP = 0, areaZ = 0, areaN = 0;
  double deltaAngle = 0;
  double alpha = asin(1.0/6.0);
  
  // Read component values
  OCR2B = MOTORSPEED; // D3
  angle = 27;
  myservo.write(angle); 
  delay(200);
  while(1) {
    cm = SensorRead();
    ref = PotentiometerReadRef();
    err = cm - ref; // Calculate the reference distance in cm
    /*
    Serial.print("Error: ");
    Serial.println(err, DEC);
    Serial.print("Ref: ");
    Serial.println(ref, DEC);
    */
    error = (double)err;
    deltaError = error - prevError;
    //Begin Fuzzy
    normError = (error+20)/40;
    normDeltaError = (deltaError+10)/20;
  
    if (error <= -10){
        eN = 1;
    } else if (error > -10 && error < 0){
        eN = -4*(normError-0.25)+1;
        eZ = 4*(normError-0.25);
    } else if (error == 0){
        eZ = 1;
    } else if (error > 0 && error < 10){
        eZ = -4*(normError-0.5)+1;
        eP = 4*(normError-0.5);
    } else if (error >= 10){
        eP = 1;
    }
  
    if (deltaError <= -6){
        deN = 1;
    } else if (deltaError > -6 && deltaError < 0){
        deN = -(3+(1.0/3.0))*(normDeltaError-0.2)+1;
        deZ = (3+(1.0/3.0))*(normDeltaError-0.2);
    } else if (deltaError == 0){
        deZ = 1;
    } else if (deltaError > 0 && deltaError < 6){
        deZ = -(3+(1.0/3.0))*(normDeltaError-0.5)+1;
        deP = (3+(1.0/3.0))*(normDeltaError-0.5);
    } else if (deltaError >= 6){
        deP = 1;
    }
  
    ruleP = max(max(min(eN, deN), min(eN, deZ)), min(eZ, deP));
    ruleZ = max(max(min(eN, deP), min(eZ, deZ)), min(eP, deN));
    ruleN = max(max(min(eZ, deP), min(eP, deZ)), min(eP, deP));
  
    areaP = ((6*ruleP-((1-ruleP)/tan(alpha))*ruleP)/2)+((1-ruleP)/tan(alpha))*ruleP;
    areaZ = ((12*ruleZ-((1-ruleZ)/tan(alpha))*ruleZ)/2)+((1-ruleZ)/tan(alpha))*ruleZ;
    areaN = ((6*ruleN-((1-ruleN)/tan(alpha))*ruleN)/2)+((1-ruleN)/tan(alpha))*ruleN;
  
    if (areaZ >= 1) {
        deltaAngle = 0 + (areaP / areaZ) * 6 - (areaN / areaZ) * 6;
    } else if (areaN >= 1){
        deltaAngle = -6 + (areaZ / areaN) * 6 + (areaP / areaN) * 12;
    } else if (areaP >= 1){
        deltaAngle = 6 - (areaZ / areaP) * 6 - (areaN / areaP) * 12;
    } else {
        Serial.println("Unknown error");
    }
    changeInAngle = -1*(int)round(deltaAngle);
    /*
    Serial.print("Delta Angle: ");
    Serial.println(changeInAngle, DEC);
    */
    //End Fuzzy
    if (((changeInAngle+angle) >= LOWER_ANGLE) && ((changeInAngle+angle) <= UPPER_ANGLE)) {
      angle = changeInAngle+angle;
    }else if (changeInAngle+angle > UPPER_ANGLE){
      angle = UPPER_ANGLE;
    }else if(changeInAngle+angle < LOWER_ANGLE){
      angle = LOWER_ANGLE;
    }
    myservo.write(angle);
    delay(DELAY);
    prevError = error;
    Serial.print(ref);
    Serial.print("\t");
    Serial.print(error);
    Serial.print("\t");
    Serial.print(deltaError);
    Serial.print("\t");
    Serial.println(angle);
    
  }
} 
