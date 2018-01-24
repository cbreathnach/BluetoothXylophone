//Receiver Code together with Bluetooth and Record functionality

//define libraries used
#include <LiquidCrystal.h>
#include <SoftwareSerial.h> 
#include <EEPROM.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);

// Set Pin Values
// Output Notes
const int note1 = 31;
const int note2 = 33;
const int note3 = 35;
const int note4 = 37;
const int note5 = 39;
const int note6 = 41;
const int note7 = 43;
const int note8 = 45;

//pin for restet function 
int resetPin = 10;

//varables for note delay 
const int del_1 = 55;
const int del_2 = 800;

//bluetooth receiver pins -
const int bluetoothTx = 12;
const int bluetoothRx = 13; 

//setup for Bluetooth
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

//Global Variables - to avoid creating new variables
//these will be accessed by frequently used functions throughout the code
//Each value is named specifcally with reference to the function that uses it

int check_values_count;
char check_values_first;
int check_delay_i;
long check_delay_time_t1 = millis();
long check_delay_time_t2;
long check_delay_time_t_total = 0;

//define the array of characters that holds the state of the sensors
char values[6] = {'L','L','L','L','L','L'};

// setup function
void setup()
{
  //use this to clear all the memory if needs be
  //erase_memory();

  //setup interupt to look for any change
  digitalWrite(resetPin,HIGH);
  delay(200);
  pinMode(resetPin,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(18),reset2,CHANGE);

  //begin Serial display
  Serial.begin(9600);

  //note outputs
  pinMode(note1, OUTPUT);
  pinMode(note2, OUTPUT);
  pinMode(note3, OUTPUT);
  pinMode(note4, OUTPUT);
  pinMode(note5, OUTPUT);
  pinMode(note6, OUTPUT);
  pinMode(note7, OUTPUT);
  pinMode(note8, OUTPUT);

  //set pins to off state
  pinMode(note1, LOW);
  pinMode(note2, LOW);
  pinMode(note3, LOW);
  pinMode(note4, LOW);
  pinMode(note5, LOW);
  pinMode(note6, LOW);
  pinMode(note7, LOW);
  pinMode(note8, LOW);

  //bluetooth setup
  bluetooth.begin(115200);
  bluetooth.print("$");
  bluetooth.print("$");
  bluetooth.print("$");
  delay(100); 
  bluetooth.println("U,9600,N"); 
  bluetooth.begin(9600);

  //lcd setup
  lcd.begin(16,2);
  Serial.begin(9600);

  //Welcome Message
  lcd.setCursor(0,0);
  lcd.print("    Start up");
  lcd.setCursor(0,1);
  lcd.print(" Xylophone Glove");
  delay(2000);

  //Main Menu Message
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Select mode");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("1-Play 2-Record");
  lcd.setCursor(0,1);
  lcd.print("3-Play Melody");
}

//main function
void loop()
{
  //check for values received
  check_values();
  print_values();

  //Actions
  
  //if first finger pressed, go into manual mode
  if(values[2]=='H')
  {
    //double check
    check_delay_time(50);
    check_values();
    if(values[2]=='H')
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Manual Mode");
      check_delay_time(500);
      //will loop in manual mode until an interupt is detected
      while(1)
      {
        play_manual();  
      }
    }
  }
     

  //if second finger pressed, go into record mode
  else if(values[3]=='H')
  {
    //double check
    check_delay_time(50); 
    check_values();
    if(values[3]=='H')
    {     
      lcd.clear();
      lcd.setCursor(0,0);
      //let the user know they are in Record mode
      lcd.print("Record Mode"); 
      check_delay_time(1000);
      
      //enter the record function
      record();
      
      // re-prints main message upon exit
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1-Play 2-Record");
      lcd.setCursor(0,1);
      lcd.print("3-Play Melody");

    }
  }

 //if third finger pressed, go into play back mode
  else if(values[4]=='H')
  {
    lcd.clear();
    check_values();
    print_values();
    check_delay_time(50); //double check
    check_values();
    if(values[4]=='H')
    {
      //option set one - built in vs recorded functions
      lcd.setCursor(0,0);
      lcd.print("Play Back Mode"); 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1: Built In");
      lcd.setCursor(0,1);
      lcd.print("2: Recorded");
      check_delay_time(1000);
      //variable for loop control
      int control = 0;
      while (control == 0)
      {

        check_values();
        print_values();

        //1 - Enter the built in function
        if (values[2] == 'H')
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("1-Twinke 2-Row");
          lcd.setCursor(0,1);
          lcd.print("3-Sleeping 4-Mary");
          check_delay_time(500);
          //enter functon to plat back built in functions
          //can be exited by interupt
          while(1)
          {
            play_back_built_in();
          }
          //control = 1; 
        }
    
        //2 - Enter the function to play back the recorded songs
        if (values[3] == 'H')
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Recorded");
          check_delay_time(500);

          //Enter function
          play_recorded();

          // re-print menu message
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("1-Play 2-Record");
          lcd.setCursor(0,1);
          lcd.print("3-Play Melody");
          control = 1;
        }
      }
    }
  }
}

// HELPER FUNCTIONS

//Check Values
//function to return state of notes
//updates the array passed into the function
void check_values()
{
  //Set global variable to 1
  check_values_count = 1;

  //If the bluetooth sent any characters - should be in continous receipt
  if(bluetooth.available()){

    //check the first character received
    check_values_first = (char)bluetooth.read();

    //if the first recived does not equal 'S', keep looking until we find it
    //This ensure they all arrive in order
    while(check_values_first != 'S'){
      check_values_first = (char)bluetooth.read();
    }

    //when 'S' is recived, assign this as the first value of the array
    values[0] = check_values_first;

    //records the other values to complete the array of values
    while(bluetooth.available() && check_values_count<6){
      values[check_values_count++] = (char)bluetooth.read(); 
    }
  }
}

//Delay a note check by a fixed amount
//designed to get rid of any values caught in the buffer
void check_delay()
{
  //set global variable back to 0
  check_delay_i = 0;

  //loops 100 times checks values
  while(check_delay_i<100)
  {
    check_values();
    print_values();
    check_delay_i++;
  }
}

//same function as above but works for a given time
void check_delay_time(int t)
{
  //set global variable to 0 and set first tie
  check_delay_time_t_total = 0;
  check_delay_time_t1 = millis();

  //checks values for the given time
  while(check_delay_time_t_total < t)
  {
    check_delay_time_t2 = millis();
    check_delay_time_t_total = (check_delay_time_t2-check_delay_time_t1);
    check_values();
    print_values();
  }
}

//Print Values
//Prints values to Serial monitor
void print_values()
{
  Serial.print(values[0]);
  Serial.print(values[1]);
  Serial.print(values[2]);
  Serial.print(values[3]);
  Serial.print(values[4]);
  Serial.print(values[5]);
  Serial.print("\n");
}

//Play Values - based off the array
//cycles through each note and sets high or low
//uses 5 fingers to play 8 notes
void play_values()
{
  //1
  if(values[1] == 'L' && values[2] == 'H')
  {
    pinMode(note1, HIGH);
  }
  else
  {
    pinMode(note1, LOW);
  }
  
  //2
  if(values[1] == 'L' && values[3] == 'H')
  {
    pinMode(note2, HIGH);
  }
  else
  {
    pinMode(note2, LOW);
  }
  
  //3
  if(values[1] == 'L' && values[4] == 'H')
  {
    pinMode(note3, HIGH);
  }
  else
  {
    pinMode(note3, LOW);
  }
  
  //4
  if(values[1] == 'L' && values[5] == 'H')
  {
    pinMode(note4, HIGH);
  }
  else
  {
    pinMode(note4, LOW);
  }
  
  //5
  if(values[1] == 'H' && values[2] == 'H')
  {
    pinMode(note5, HIGH);
  }
  else
  {
    pinMode(note5, LOW);
  }
  
  //6
  if(values[1] == 'H' && values[3] == 'H'){
    pinMode(note6, HIGH);
  }
  else{
    pinMode(note6, LOW);
  }
  
  //7
  if(values[1] == 'H' && values[4] == 'H')
  {
    pinMode(note7, HIGH);
  }
  else
  {
    pinMode(note7, LOW);
  }
  
  //8
  if(values[1] == 'H' && values[5] == 'H')
  {
    pinMode(note8, HIGH);
  }
  else
  {
    pinMode(note8, LOW);
  } 
}
 
/////////////MAIN FUNCTIONS///////////////

// 1: Manual mode 
//plays notes as they are inputted on the Xylophone
void play_manual(void)
{
  //checks values
  check_values();
  print_values();

  //reset if needed
  if (values[1]=='H' && values[2]=='H' && values[3]=='H' && values[4]=='H' && values[5]=='H')
  {
     //double check
     check_delay_time(50);
     check_values();
     if (values[1]=='H' && values[2]=='H' && values[3]=='H' && values[4]=='H' && values[5]=='H')
     {
       //resest
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("Restarting");
       digitalWrite(resetPin,LOW);
     }
   }

  //if no reset detected - play values
  //will loop over this continously until resest detected
  play_values();
}

// 2: Play Back Mode
//This has two components - to play back the built in functions
// and to play back the recorded functions
//This function plays back the built in songs
//It plays the songs hardcoded into the Arduino
void play_back_built_in(void)
{
  //check values
  check_values();
  print_values();

  //act on inputted values
  //passes functionality to given functions
  //Then re-prints the menu
  //1: Twinkle Twinkle
  if (values[2]=='H')
  {
    play_twinkle_twinkle();
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("1-Twinke 2-Row");
    lcd.setCursor(0,1);
    lcd.print("3-Sleeping 4-Mary");
    check_delay_time(1000);
  }

  //2: Row Row
  if (values[3]=='H')
  {
    play_row_row();
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("1-Twinke 2-Row");
    lcd.setCursor(0,1);
    lcd.print("3-Sleeping 4-Mary");
    check_delay_time(1000);
  }
  
  if (values[4]=='H')
  {
    are_you_sleeping();
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("1-Twinke 2-Row");
    lcd.setCursor(0,1);
    lcd.print("3-Sleeping 4-Mary");
    check_delay_time(1000);
  }
  
  if (values[5]=='H')
  {
    mary_has_a_little_lamb();
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("1-Twinke 2-Row");
    lcd.setCursor(0,1);
    lcd.print("3-Sleeping 4-Mary");
    check_delay_time(1000);
  }
  //The function loops and you can choose another song
  //an interupt exits the function
}

// pre-recorded music functions
// These songs are hardcoded in

// 1)- twinkle_twinkle 
void play_twinkle_twinkle(void)
{
  //print name
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Twinkle_Twinkle ");
  //play notes
  play_purpule();play_purpule();
  play_yellow();play_yellow();
  play_orange();play_orange();
  play_yellow();
  delay(400);
  play_lightgreen();play_lightgreen();
  play_deepgreen();play_deepgreen();
  play_blue();play_blue();
  play_purpule();
  delay(550);
  play_yellow();play_yellow();
  play_lightgreen();play_lightgreen();
  play_deepgreen();play_deepgreen();
  play_blue();
  delay(500);
  play_yellow();play_yellow();
  play_lightgreen();play_lightgreen();
  play_deepgreen();play_deepgreen();
  play_blue(); 
  delay(500);
  play_purpule();play_purpule();
  play_yellow();play_yellow();
  //delay(300);
  play_orange();play_orange();
  play_yellow();
  delay(400);
  play_lightgreen();play_lightgreen();
  play_deepgreen();play_deepgreen();
  play_blue();play_blue();
  play_purpule();
}

// 2) - row_row
void play_row_row(void)
{
   //prints message
   lcd.clear();
   lcd.setCursor(0, 1);
   lcd.print("Row Row");
   //Plays Notes
   play_purpule();
   play_purpule();
   play_purpule();
   play_blue();
   play_deepgreen();
   play_deepgreen();
   play_blue();
   play_deepgreen();
   play_lightgreen();
   delay(50);
   play_yellow();
   delay(450);
   play_pink();
   play_pink();
   play_pink();
   play_yellow();
   play_yellow();
   play_yellow();
   play_deepgreen();
   play_deepgreen();
   play_deepgreen();
   play_purpule();
   play_purpule();
   play_purpule();
   play_yellow();
   play_lightgreen();
   play_deepgreen();
   play_blue();
   play_purpule();
}

// 3- Are You Sleeping 
void are_you_sleeping(void)
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Are you ");
  lcd.setCursor(0, 1);
  lcd.print("Sleeping");
  play_purpule();
  play_blue();
  play_deepgreen();
  play_purpule();
  delay(400);
  play_purpule(); 
  play_blue();
  play_deepgreen();
  play_purpule();
  delay(300);
  play_blue();
  play_lightgreen();
  play_yellow();
  delay(200);
  play_blue();
  play_lightgreen();
  play_yellow();
  delay(300);
  play_yellow();
  play_orange();
  play_yellow();
  play_lightgreen();
  play_blue();
  play_yellow();
  delay(500);
  play_orange();
  play_yellow();
  play_lightgreen();
  play_blue();
  play_purpule();
  play_pink();
  delay(300);
  play_yellow();
  play_pink();
  delay(1000);
  play_pink();
  play_yellow();
  play_pink();
  delay(1000);
}

// 4- Mary Had A Little Lamb

void mary_has_a_little_lamb (void)
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Mary had a");
  lcd.setCursor(0, 1);
  lcd.print("Little Lamb");
  
  play_lightgreen();
  play_blue();
  play_purpule();
  play_blue();
  delay(200);
  play_lightgreen();
  play_lightgreen();
  play_lightgreen();
  delay(200);
  play_blue();
  play_blue();
  play_blue();
  delay(400);
  play_lightgreen();
  play_yellow();
  play_yellow();
  delay(300);
  play_lightgreen();
  play_blue();
  play_purpule();
  play_blue();
  delay(200);
  play_lightgreen();
  play_lightgreen();
  play_lightgreen();
  play_lightgreen();
  play_blue();
  play_blue();
  play_lightgreen();
  play_blue();
  play_purpule();
  delay (100);
}

//code for notes 
//each function plays a certain note
//it sets it high and then to low
//each note also checks for an interupt
//this would exit the song and re-start the Arduino

void play_purpule(void)
{
   //resetCheck();
   pinMode(note1,HIGH);
   delay(del_1);
   pinMode(note1,LOW);
   reset_check_time(del_2);
}
void play_blue(void)
{
   resetCheck();
   pinMode(note2,HIGH);
   delay(del_1);
   pinMode(note2,LOW);
   reset_check_time(del_2);
}
void play_deepgreen(void)
{
   resetCheck();
   pinMode(note3,HIGH);
   delay(del_1);
   pinMode(note3,LOW);
   reset_check_time(del_2);
  
}
void play_lightgreen(void)
{
   resetCheck();
   pinMode(note4,HIGH);
   delay(del_1);
   pinMode(note4,LOW);
   reset_check_time(del_2);
}

void play_yellow(void)
{
   resetCheck();
   pinMode(note5,HIGH);
   delay(del_1);
   pinMode(note5,LOW);
   reset_check_time(del_2);
  
}
void play_orange(void)
{
   resetCheck();
   pinMode(note6,HIGH);
   delay(del_1);
   pinMode(note6,LOW);
   reset_check_time(del_2);
}
void play_red(void)
{
   resetCheck();
   pinMode(note7,HIGH);
   delay(del_1);
   pinMode(note7,LOW);
   reset_check_time(del_2);
}
void play_pink(void)
{
   resetCheck();
   pinMode(note8,HIGH);
   delay(del_1);
   pinMode(note8,LOW);
   reset_check_time(del_2);
}

///////reset functions//////////
//interupt function
//sets the pre-set resest pin to low which restarts the Arduino
void reset2()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Restarting");
  digitalWrite(resetPin,LOW);
}

//checks for five high inputs and restarts the arduino if they are found
void resetCheck(void)
{
   check_values();
   if (values[1]=='H' && values[2]=='H' && values[3]=='H' && values[4]=='H' && values[5]=='H')
   {
     //double check
     check_delay_time(50);
     check_values();
     if (values[1]=='H' && values[2]=='H' && values[3]=='H' && values[4]=='H' && values[5]=='H')
     {
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("Restarting");
       digitalWrite(resetPin,LOW);
     }
  }        
}

//same function as above but works for a given time
//used as a delay in the playback built in function
void reset_check_time(int t)
{
  long t1 = millis();
  long t2;
  long t_total = 0;
  while(t_total < t)
  {
    t2 = millis();
    t_total = (t2-t1);
    resetCheck();
  }
}


//3: Record Function

//Record Function - used as a step into the following function
// - record_piece which does the actual recording
void record(void)
{
  int recordslot;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Record Mode");  //let the user know they are in record mode

  //will ask the user which slot they would like to record in
  lcd.setCursor(0,1);
  lcd.print("Select a Slot");

  //variable to control the loop
  int control = 0;
  while(control < 1)
  {
    check_values();
    print_values();
  
    //check for record slot
    //if found - exit loop
    //1
    if (values[2] == 'H')
    {
      recordslot = 1;
      control = 1;
    }
  
    //2
    if (values[3] == 'H')
    {
      recordslot = 2;
      control = 1;
    }
  
    //3
    if (values[4] == 'H')
    {
      recordslot = 3;
      control = 1;
    }
  
    //4
    if (values[5] == 'H')
    {
      recordslot = 4;
      control = 1;
    }
        
  }
  
  //pass to record slot function
  record_piece(recordslot);

  //will return to the main menu
  check_delay_time(2000);
}

//takes in the slot number from the previous function
//then records the values it reads and stores in EEPROM
//Songs recorded are thus always available even after resest
void record_piece(int n)
{
  //loop control variable
  int control = 0;
  check_delay_time(500);

  //check for already recorded first & Overwrite if needed
  if (EEPROM.read(0+(1000*(n-1))) == 1)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Slot ");
    lcd.print(n);
    lcd.print(" Recorded");
    lcd.setCursor(0,1);
    lcd.print("Already");

    check_delay_time(1500);

    //give the user the option to overwritse
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Overwrite?");
    lcd.setCursor(0,1);
    lcd.print("1:Yes  2:No");
    check_delay_time(1000);

    //second control variable
    int control2 = 0;
    while (control2 == 0)
    {
        check_values();
        print_values();

        //Yes
        if (values[2] == 'H')
        {
          control = 0;
          control2 = 1;
          //call function to erase the memory slot
          erase_memory_slot(n);
        }

        //No
        if (values[3] == 'H')
        {
          control = 1;
          control2 = 1;
        }
     }
 }

  //want to record
  if (control == 0)
  {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Now Recording ");
      lcd.print(n);

      //define byte value to be stored as a high
      byte high = 1;

      //modify record state to high
      int record_state_location = (0+(1000*(n-1)));
      EEPROM.write(record_state_location,high);

      //variable for the record loop
      int i = 0;
      int write_count = 0;
      int display_time;
    
      check_delay_time(500);
    
      while (i<199)
      {
        //calculate & print the display time
        display_time = (20 - (i/10));
        lcd.setCursor(0,1);
        lcd.print("Time: ");
        lcd.setCursor(6,1);
        lcd.print("   ");
        lcd.setCursor(6,1);
        lcd.print(display_time);

        //set the write count to zero for a new cycle
        write_count = 0;
  
        check_values();
        print_values();
        //play values as well here to create better recording experience
        play_values();
    
        //define locations
        //defined by record slot n & loop count i
        //i increase for each loop and records in order
        int location1 = ((5 + i) + (1000 * (n-1)));
        int location2 = ((204 + i) + (1000 * (n-1)));
        int location3 = ((403 + i) + (1000 * (n-1)));
        int location4 = ((602 + i) + (1000 * (n-1)));
        int location5 = ((801 + i) + (1000 * (n-1)));
    
        //record the notes
        //increase write count each time to allow for time adjustment
        //1
        if (values[1] == 'H')
        {
          EEPROM.write(location1,high);
          write_count++;
        }
    
        //2
        if (values[2] == 'H')
        {
          EEPROM.write(location2,high);
          write_count++;
        }
    
        //3
        if (values[3] == 'H')
        {
          EEPROM.write(location3,high);
          write_count++;
        }
    
        //4
        if (values[4] == 'H')
        {
          EEPROM.write(location4,high);
          write_count++;
        }
    
        //5
        if (values[5] == 'H')
        {
          EEPROM.write(location5,high);
          write_count++;
        }

        //increase loop count
        i++;
    
        //delay for recording next values - offset by time taken for each write cycle
        //must cycle through incoming inputs to avoid errors with buffered values
 
        long time_delay = 0;
        long time1 = millis();
        long time2;
    
        while(time_delay < (100 - (3.3*write_count)))
        {
          time2 = millis();
          time_delay = (time2-time1);
          check_values();
          print_values();
        }
      }

      //print message when finished
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Piece Recorded");
      lcd.setCursor(0,1);
      lcd.print("Slot ");
      lcd.print(n);
      check_delay_time(2000);
      //returns to menu
  }

  
  //if we do not want to overwrite
  else
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Not Recording");
    lcd.setCursor(0,1);
    lcd.print("Returning to menu");
    check_delay_time(2000);
  }
}

//Playback Recorded Functions
//play recorded - select here and then pass to 
//play_back_recorded function
void play_recorded(void)
{
  int recordslot;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Select a slot");
  lcd.setCursor(0,1);
  lcd.print("to play from");
  check_delay_time(500);

  //loop control variable
  int control = 0;

  //loop to select slot
  while(control < 1)
  {  
    //check values
    check_values();

    //Serial.print("Loop");
    print_values();
  
    //check for record slot
    //1
    if (values[2] == 'H')
    {
      recordslot = 1;
      control = 1;
    }
  
    //2
    if (values[3] == 'H')
    {
      recordslot = 2;
      control = 1;
    }
  
    //3
    if (values[4] == 'H')
    {
      recordslot = 3;
      control = 1;
    }
  
    //4
    if (values[5] == 'H')
    {
      recordslot = 4;
      control = 1;
    }   
  }

  //pass to next function
  play_back_recorded(recordslot);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Finished");
  lcd.setCursor(0,1);
  lcd.print("Return to menu");
  check_delay_time(2000);
}

//play back recorded songs
//reverse of the record function
//checks EEPROM locations and plays back in same order as was recorded
void play_back_recorded(int n)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Playing ");
  lcd.print(n);
  check_delay_time(1500);

  //check if something has been recorded first and then we proceed to play
  if (EEPROM.read(0+(1000*(n-1))) == 1)
  {
    int control = 0;
    int i = 0;

    //start timing now
    long time1 = millis();
    long T = 0.0;
    int display_time;

    //use while loop to recall array values and then play then
    while((control == 0) && (T<20000))
    {
      //check % display time left
      long time2 = millis();
      T = (time2 - time1);
      display_time = (20 - (T/1000));
      lcd.setCursor(0,1);
      lcd.print("Time: ");
      lcd.setCursor(6,1);
      lcd.print("  ");
      lcd.setCursor(6,1);
      lcd.print(display_time);

      //read and create the array - store in values array
      //1
      if (EEPROM.read((5 + i) + (1000 * (n-1))) == 1)
      {
        values[1] = 'H';
      }
      else
      {
        values[1] = 'L';
      }
      //2
      if (EEPROM.read((204 + i) + (1000 * (n-1))) == 1)
      {
        values[2] = 'H';
      }
      else
      {
        values[2] = 'L';
      }
      //3
      if (EEPROM.read((403 + i) + (1000 * (n-1))) == 1)
      {
        values[3] = 'H';
      }
      else
      {
        values[3] = 'L';
      }
      //4
      if (EEPROM.read((602 + i) + (1000 * (n-1))) == 1)
      {
        values[4] = 'H';
      }
      else
      {
        values[4] = 'L';
      }
      //5
      if (EEPROM.read((801 + i) + (1000 * (n-1))) == 1)
      {
        values[5] = 'H';
      }
      else
      {
        values[5] = 'L';
      }
     
      //play the notes
      play_values();
      print_values();

      // increase loop increment to play next note
      i++;

      //time delay as per recording
      delay(100);
    }

    //return the menu when finished
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Song Finished");
    lcd.setCursor(0,1);
    lcd.print("Returning to menu");
  }

  //if no song recorded
  else
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("No Song Recorded");
    delay(2000);
  }
}

//OTHER HELPER FUNCTIONS - ERASE OPTIONS

void erase_memory_slot(int slot)
{ 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Now Erasing ");
  lcd.print(slot);
  delay(500);

  //start and finish calculations for slot
  int start = (1000*(slot-1));
  int finish = (1000*(slot) -1);
  int erase = start;

  //cycle though loctions until all erased
  while(erase <= finish){
    EEPROM.write(erase,0);
    erase++;
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Erased ");
  lcd.print(slot);
  delay(2000); 
}

//clears all memory if required
void erase_memory(void)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Now Erasing ");
  lcd.setCursor(0,1);
  lcd.print("All Memory ");

  //cycle through all locations
  //write them to zero
  for (int i = 0 ; i < EEPROM.length() ; i++) 
  {
    EEPROM.write(i, 0);
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Erased ");
  delay(1000); 
}

