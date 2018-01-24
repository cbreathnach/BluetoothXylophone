// Glove Code - Bluetooth transmission

//library for Bluetooth
#include <SoftwareSerial.h> 

//define constant pin numbers
const int sensor0 = A5;
const int sensor1 = A4;
const int sensor2 = A0;
const int sensor3 = A1;
const int sensor4 = A2;
const int sensor5 = A3;

//variable for sensor reading
int FDR0_Value = 0;
int FDR1_Value = 0;
int FDR2_Value = 0;
int FDR3_Value = 0;
int FDR4_Value = 0;
int FDR5_Value = 0;

//Bluetooth Variables
const int bluetoothTx = 3;
const int bluetoothRx = 4; 

//setup for Bluetooth
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

//functions
void sensor_read(void);
void transmit_pins(void);

void setup()
{
  //beging serial function
  Serial.begin(9600);

  //setup for Bluetoothn - sets to reliable rate
  bluetooth.begin(115200);
  bluetooth.print("$"); 
  bluetooth.print("$");
  bluetooth.print("$"); 
  delay(100);  
  bluetooth.println("U,9600,N");
  bluetooth.begin(9600);  
}

void loop()
{
   transmit_pins_blue();

   //use this to read the analogue values of the sensors
   //sensor_read();
}

void transmit_pins_blue(void)
{
  //read sensor values
  FDR0_Value = analogRead(sensor0);
  FDR1_Value = analogRead(sensor1);
  FDR2_Value = analogRead(sensor2);
  FDR3_Value = analogRead(sensor3);
  FDR4_Value = analogRead(sensor4);
  FDR5_Value = analogRead(sensor5);

  //Transmit Values

  //0 - always transmit 'S'
  //pin connected to VCC
  if (FDR0_Value > 20){
    //sensors[0] = 'S';
    bluetooth.write('S');
    Serial.print('S');
  }
  else{
    //sensors[0] = 'S';
    bluetooth.write('S');
    Serial.print('S');
  }

  //Five finger values for transmission
  //checks for H/L & writes to Bluetooth connection
  
  //1
  if (FDR1_Value > 20)
  {
    bluetooth.write('H');
    Serial.print('H');
  }
  else
  {
    bluetooth.write('L');
    Serial.print('L');
  }
  
  //2
  if (FDR2_Value > 20)
  {
    bluetooth.write('H');
    Serial.print('H');
  }
  else
  {
     bluetooth.write('L');
     Serial.print('L');
  }
  
  //3
  if (FDR3_Value > 220)
  {
    bluetooth.write('H');
    Serial.print('H');
  }
  else
  {
    bluetooth.write('L');
    Serial.print('L');
  }
  
  //4
  // offset for sensor
  if (FDR4_Value > 80)
  {
    bluetooth.write('H');
    Serial.print('H');
  }
  else
  {
    bluetooth.write('L');
    Serial.print('L');
  }
  
  //5
  if (FDR5_Value > 20)
  {
    bluetooth.write('H');
    Serial.print('H');
  }
  else
  {
    bluetooth.write('L');
    Serial.print('L');
  }

  Serial.print("\n");
}

void sensor_read(void)
{
  // prints sensor readings to screen
  FDR1_Value = analogRead(sensor1);
  FDR2_Value = analogRead(sensor2);
  FDR3_Value = analogRead(sensor3);
  FDR4_Value = analogRead(sensor4);
  FDR5_Value = analogRead(sensor5);
  
  //Serial.print("Sensor 1 Value: ");
  Serial.print(String(FDR1_Value) + ";  ");
  
  //Serial.print("Sensor 2 Value: ");
  Serial.print(String(FDR2_Value) + ";  ");
  
  //Serial.print("Sensor 3 Value: ");
  Serial.print(String(FDR3_Value) + ";  ");
  
  //Serial.print("Sensor 4 Value: ");
  Serial.print(String(FDR4_Value) + ";  ");
  
  //Serial.print("Sensor 5 Value: ");
  Serial.print(String(FDR5_Value) + ";  ");
  
  Serial.print("\n");
  delay(100);
}


