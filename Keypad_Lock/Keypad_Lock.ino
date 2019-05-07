/**
 * This Program was developed for a Digital Electronics lab on Spring Semester 2019.
 * 
 * Keypad Parking Lot Entrance: (BASIC FUNCTIONALITY)
 *  Designed to grant acces through mechanical arm into private porperty.
 *  Password (or PIN) can be changed.
 *  Ultrasonic Sensor does not allow arm to lower when vehicle has not passed.
 *  LCD Screen displays feedback to person in vehicle.
 */

#include <Keypad.h>
#include<LiquidCrystal.h>
#include<EEPROM.h>
#include <Servo.h>

/* --- Global Variables --- */
LiquidCrystal liquid_crystal_display(9,8,7,6,5,4);

char password[4];                           //Pin
char initial_password[4], new_password[4];
int i=0;

int servo_pwm = 11;                         //Servo Motors
int servo_state = 0;

const int trigPin = 12;                     // Ultrasonic Sensor
const int echoPin = 13;
long duration;
int distanceInch;

char key_pressed = 1;                       //Keypad
const byte rows = 4; 
const byte columns = 4; 
char hexaKeys[rows][columns] = {{'1','2','3','A'},
                                {'4','5','6','B'},
                                {'7','8','9','C'},
                                {'*','0','#','D'}};

byte row_pins[rows] = {2,3,A5,A0};
byte column_pins[columns] = {A1,A2,A3,A4};   

Keypad keypad_key = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);
Servo lock_motor;

void setup()
{
  /* --- Init Servo --- */
  lock_motor.attach(servo_pwm);           // Attaches servo PWM pin to servo_pwm.
  if(servo_state != 180)                  // Set Servo default position.
  {
    lock_motor.write(180);
    servo_state = 180;
  }
  pinMode(servo_pwm, OUTPUT);

  /* --- Init Ultrasound Sensor --- */
  pinMode(trigPin, OUTPUT);             // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);              // Sets the echoPin as an Input
  Serial.begin(9600);
  
  /* --- Init Servo --- */
  liquid_crystal_display.begin(16,2);
  liquid_crystal_display.print(" Hello Daniel.");
  liquid_crystal_display.setCursor(0,1);
  liquid_crystal_display.print("Electronic Lock. ");
  delay(2000);
  /* --- Setup LCD for loop. --- */
  liquid_crystal_display.clear();
  liquid_crystal_display.print("Enter Password");
  liquid_crystal_display.setCursor(0,1);
  initialpassword();

}

void loop()
{
  // Reset Servo State.
  distanceInch = getDistance();                     // Get distance of object in front of untrasonic sensor.

  if(servo_state != 0 && distanceInch > 3)          // Reset Servo State
  {
    lock_motor.write(180);
    servo_state = 180;
    digitalWrite(trigPin, LOW);
  }
  
  key_pressed = keypad_key.getKey();
  if(key_pressed=='#')                              // Change Pin
    change();
  if(key_pressed=='*')                              // Re-Enter Password.
  {
    liquid_crystal_display.clear();
    liquid_crystal_display.print("Enter Password:");
    liquid_crystal_display.setCursor(0,1);
    i = 0;
  }
  else if (key_pressed)
  {
    password[i++] = key_pressed;
    liquid_crystal_display.print(key_pressed);
  }

  if(i==4)                                          //When Pin input is complete.
  {
    delay(200);                                     // Delay for Stability

    for(int j=0;j<4;j++)                            //Get Correct Password.
      initial_password[j]=EEPROM.read(j);
    
    if(!(strncmp(password, initial_password,4)))    // Pin is Correct
    {
      liquid_crystal_display.clear();
      liquid_crystal_display.print("Pass Accepted");
      if(servo_state != 90)
      {
        lock_motor.write(90);
        servo_state = 90;
      }
      
      delay(2000);

      liquid_crystal_display.clear();
      liquid_crystal_display.setCursor(0,1);
      liquid_crystal_display.print("Pres # to change");
      
      delay(20);

      liquid_crystal_display.clear();
      liquid_crystal_display.print("Enter Password:");
      liquid_crystal_display.setCursor(0,1);
      i=0;
    }
    else                                              // Pin is Wrong
    {
      if(servo_state != 180)
      {
        lock_motor.write(180);
        servo_state = 180;
      }

      liquid_crystal_display.clear();
      liquid_crystal_display.print("Wrong Password");
      liquid_crystal_display.setCursor(0,1);
      liquid_crystal_display.print("Pres # to Change");

      delay(2000);

      liquid_crystal_display.clear();
      liquid_crystal_display.print("Enter Password");
      liquid_crystal_display.setCursor(0,1);

      i=0;                                              // Reset counter
    }
  }
}

/**
 * Calculates the distance in inches based on 
 * Ultrasonic sensor measurements.
 */
int getDistance()
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  distanceInch = duration*0.0133/2;
  Serial.println(distanceInch);

  return distanceInch;
}

/**
 * Changes the password or PIN.
 */
void change()
{
  int j=0;
  
  liquid_crystal_display.clear();
  liquid_crystal_display.print("Current Password");
  liquid_crystal_display.setCursor(0,1);

  while(j<4)
  {
    char key=keypad_key.getKey();
    
    if(key)
    {
      new_password[j++]=key;
      liquid_crystal_display.print(key);
    }
    key=0;
  }
  delay(500);

  if((strncmp(new_password, initial_password, 4)))
  {
    liquid_crystal_display.clear();
    liquid_crystal_display.print("Wrong Password");
    liquid_crystal_display.setCursor(0,1);
    liquid_crystal_display.print("Try Again");
    
    delay(1000);
  }
  else
  {
    j=0;

    liquid_crystal_display.clear();
    liquid_crystal_display.print("New Password:");
    liquid_crystal_display.setCursor(0,1);
    
    while(j<4)
    {
      char key=keypad_key.getKey();
      
      if(key)
      {
        initial_password[j]=key;
        liquid_crystal_display.print(key);
        EEPROM.write(j,key);
        j++;
      }
    }
    liquid_crystal_display.print("Pass Changed");

    delay(1000);

  }
  liquid_crystal_display.clear();
  liquid_crystal_display.print("Enter Password");
  liquid_crystal_display.setCursor(0,1);

  key_pressed=0;
}

/**
 * Sets Default Password
 */
void initialpassword()
{
  for(int j=0;j<4;j++)
    EEPROM.write(j, j+49);
  
  for(int j=0;j<4;j++)
    initial_password[j]=EEPROM.read(j);
} 
