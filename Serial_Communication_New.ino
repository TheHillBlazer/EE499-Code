#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

#define RST_PIN 5  //Reset pin for RFID Reader
#define SS_PIN 53  //SDA pin for RFID Reader

//rows and columns for keyboard
const byte ROWS = 4;     //Keyboard has 4 rows
const byte COLUMNS = 4;  //Keyboards has 4 columns

//keyboard setup
char keys[ROWS][COLUMNS] = {  // 2D array aligning characters with buttons
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 24, 26, 28, 30 };     //Keypad Row Pins
byte colPins[COLUMNS] = { 32, 34, 36, 38 };  //Keypad Column Pins

//Create keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLUMNS);

//Variables for RFID
byte readCard[4];
String MasterTag = "C3626A30";    //ID that will be checked for
String MasterTag_2 = "B98C3C9B";  //Another valid user
String tagID = "";                //For getID tag

MFRC522 mfrc522(SS_PIN, RST_PIN);    //Tells arduino RFID SDA(SS) and Reset(RST) pin
LiquidCrystal_I2C lcd(0x27, 16, 2);  //Tells arduino LCD size and adress

bool raspOn = false;            //searching for raspberry pi on
bool authUser = false;          //Valid RFID has not been scanned
bool userAuthRec = false;       //Pi has not responded it knows valid user has been acknowledged by arduino
bool userChoice = false;        //User loads unloads, or designates room
bool menu = false;              //User tells robot information
bool cell_1 = false;            //Nothing in cell 1
bool cell_2 = false;            //Nothing in cell 2
bool where = false;             //Robot has not been told where to go yet
bool sendingRoom_1OR2 = false;  //Tells Pi to be expecting room 1 or 2 information
bool room_1_received = false;   //Room number 1 has not been received by Pi
bool room_2_received = false;   //Room number 1 has not been received by Pi
bool looking_for_confirmed = false;
bool confirm1 = false;
bool confirm2 = false;
bool command_recieved = false;  //Waiting for Open CV to start on Pi
bool key_1Input = false;
bool key_2Input = false;
bool key_3Input = false;

void setup() {
  SPI.begin();         //required for RFID
  mfrc522.PCD_Init();  //required for RFID
  Serial.begin(9600);

  lcd.init();       //INitialize LCD
  lcd.clear();      //Ensures no text on LCD
  lcd.backlight();  // Make sure backlight is on


}

void loop() {

  //This loop ensures pi and arduino are communicating with each other
  while (raspOn == false) {
    lcd.print("Waiting for Pi");  //To let user know what arudino is doing
    delay(1000);
    Serial.println("Arduino On");                     //Sends to Pi
    if (Serial.available() > 0) {                     //Reading response from Pi
      String message = Serial.readStringUntil('\n');  //Reads message from pi
      Serial.println(message);
      if (message == "Pi On") {
        raspOn = true;  //Pi has confirmed Arduino is on and Arduino confirmed pi is on
        lcd.clear();
        lcd.print("Pi connected");
        delay(3000);
      }
    }
    lcd.clear();
  }

  //This step is where the Arduino confirms an authorized user is present
  while (authUser == false) {
    lcd.clear();
    lcd.print("Please Scan ID");
    delay(3000);  //Allows time for user to get RFID card on reader
    getID();
    if ((tagID == MasterTag) || (tagID == MasterTag_2)) {
      lcd.clear();
      authUser = true;          //Valid user has scanned now waiting for Pi to confirm
      lcd.print("Valid User");  //Shows user ID is valid
      while (userAuthRec == false) {
        Serial.println("User Authorized");  //Sending to Pi for confirmation
        delay(500);
        lcd.clear();
        lcd.print("Need Pi to OK");  //Tells user arduino is waiting on pi to confirm
        if (Serial.available() > 0) {
          String message = Serial.readStringUntil('\n');  //Pi telling arduino it has confirmed
          Serial.println(message);
          if (message == "Auth User Received") {
            userAuthRec = true;  //Arduino won't run this loop anymore and moves to next step
            lcd.clear();
            lcd.print("Pi confirmed");
            delay(3000);
          }
        }
      }
    } else {  //Invalid user
      lcd.clear();
      lcd.print("Invalid/No User");
      delay(1500);
    }
  }

  //User tells robot what to do
  //1. Load Cell(L.Cell) - 2. Unload Cell(U.Cell) - 3. Where is robot going (select cell 1 or 2 then it sends that to Pi)
  //4. Begin Transport(Start)
  while (menu == false) {
    //Code below prints options to LCD
    lcd.clear();
    lcd.print("1) Load Cell");
    delay(1500);
    lcd.setCursor(0, 1);
    lcd.print("2) Unload Cell");
    delay(1500);
    lcd.clear();
    lcd.print("3) Where to?");
    delay(1500);
    lcd.setCursor(0, 1);
    lcd.print("4) Start");
    delay(1500);

    String room_1 = "";
    String room_2 = "";

    char key = keypad.waitForKey();  //gets keypad button pressed

    if (key) {           //if key is pressed
      if (key == '1') {  //load cell
        lcd.clear();
        lcd.print("Select Cell");
        delay(1500);
        lcd.setCursor(0, 1);
        lcd.print("Press 1 or 2");
        delay(1500);
        lcd.clear();
        lcd.print("Or B for menu");
        delay(1500);
        while (sendingRoom_1OR2 == false) {  //Telling pi to be looking for room number 1 or 2
          Serial.println("User Inputting Room 1 or 2");
          if (Serial.available() > 0) {
            String message = Serial.readStringUntil('\n');  //Pi telling arduino it has confirmed
            Serial.println(message);
            if (message == "Looking for Room 1 or 2") {
              sendingRoom_1OR2 = true;  //Arduino won't run this loop anymore and moves to next step
            }
          }
        }
        while (key_1Input == false) {
          lcd.clear();
          lcd.print("Please Press Key");
          char key_1 = keypad.waitForKey();
          if (key_1) {
            if (key_1 == '1') {
              //unlock cell 1 which would be sending a pulse to the solenoid
              key_1Input = true;
              lcd.clear();
              lcd.print("Cell 1 open");
              cell_1 = true;  // cell now holds something
              delay(1500);
              //There would then be code here telling user to hit a button when ready to lock the Cell again
              lcd.clear();
              lcd.print("What Room #?");  //assigns room number to cell 1
              delay(1500);
              lcd.clear();
              lcd.print("Input room #");
              delay(1500);
              lcd.clear();
              lcd.print("Hit D when done");
              delay(1500);
              lcd.clear();
              lcd.print("Press B to erase");
              delay(1500);
              lcd.clear();
              lcd.print("Limit of 8 #s");  //To ensure memory space
              delay(2000);
              lcd.clear();
              String room_1 = inputRoom();

              while (room_1_received == false) {               //ensures while loop does not repeat instruction above again and again
                Serial.println("Room Number 1 Is:" + room_1);  //Sending room number to Pi for open CV
                if (Serial.available() > 0) {
                  String message = Serial.readStringUntil('\n');  //Pi telling arduino it has confirmed
                  Serial.println(message);
                  if (message == "Pi stored Room 1") {
                    room_1_received = true;  //Arduino won't run this loop anymore and moves to next step
                    lcd.clear();
                    lcd.print("Room 1 Saved");
                    delay(1500);
                  }
                }
              }
            } else if (key_1 == '2') {
              //unlock cell 2
              key_1Input = true;
              lcd.clear();
              lcd.print("Cell 2 open");
              cell_2 = true;
              delay(1000);
              //There would then be code here telling user to hit a button when ready to lock the Cell again
              lcd.clear();
              lcd.print("What Room #?");  //assigns room number to cell 2
              delay(1500);
              lcd.clear();
              lcd.print("Input room #");  //Instructions for
              delay(1500);
              lcd.clear();
              lcd.print("Hit D when done");
              delay(1500);
              lcd.clear();
              lcd.print("Press B to erase");
              delay(1500);
              lcd.clear();
              lcd.print("Limit of 8 #s");  //To ensure memory space
              delay(2000);
              lcd.clear();

              room_2 = inputRoom();

              while (room_2_received == false) {               //ensures while loop does not repeat instruction above again and again
                Serial.println("Room Number 2 Is:" + room_2);  //Sending room number to Pi for open CV
                if (Serial.available() > 0) {
                  String message = Serial.readStringUntil('\n');  //Pi telling arduino it has confirmed
                  Serial.println(message);
                  if (message == "Pi stored Room 2") {
                    room_2_received = true;  //Arduino won't run this loop anymore and moves to next step
                    lcd.clear();
                    lcd.print("Room 2 Saved");
                    delay(1500);
                  }
                }
              }
            } else if (key_1 == 'B') {
              //"goes back to menu" in reality it just exits the if statement
              key_1Input = true;
              lcd.clear();
              lcd.print("Back to menu");
              delay(1000);
              backToMenu();  //Ensures Pi is on Same Page
            } else {
              lcd.clear();
              lcd.print("Invalid Input");
            }
          }
        }
      } else if (key == '2') {  //unload cell
        lcd.clear();
        lcd.print("Select Cell");
        delay(1000);
        lcd.print("Press 1 or 2");
        delay(1000);
        lcd.print("Or B for menu");
        delay(1000);
        while (key_2Input == false) {
          lcd.print("Please Press Key");
          char key_2 = keypad.waitForKey();
          if (key_2) {
            if (key_2 == '1') {
              //Unlocks Cell 1
              lcd.clear();
              lcd.print("Unload Cell 1");
              key_2Input = true;
              room_1 = "";
              cell_1 = false;  //Cell Empty
              //No need to tell Pi because Pi can just overwrite the information
              //User would push button to lock cell back
            } else if (key_2 == '2') {
              //Unlocks Cell 2
              lcd.clear();
              lcd.print("Unload Cell 2");
              key_2Input = true;
              room_2 = "";
              cell_2 = false;  //Cell Empty
              //No need to tell Pi because Pi can just overwrite the information
              //User would push button to lock cell back
            } else if (key_2 == 'B') {
              //"goes back to menu" in reality it just exits the if statement
              lcd.clear();
              lcd.print("Back to menu");
              key_2Input = true;
              delay(1000);
            } else {
              lcd.clear();
              lcd.print("Invalid Input");
            }
          }
        }
      } else if (key == '3') {  //set which cell to deliver to
        while (looking_for_confirmed == false) {
          Serial.println("User will confirm Room");  //Telling pi room is about to be set
          if (Serial.available() > 0) {
            String message = Serial.readStringUntil('\n');  //Pi telling arduino it has confirmed
            //Serial.println(message);
            if (message == "Pi waiting for final confirmation") {
              lcd.clear();
              lcd.print("Which Cell");
              delay(1000);
              lcd.setCursor(0, 1);
              lcd.print("Cell 1 or Cell 2");
              delay(1000);
              looking_for_confirmed = true;  //Arduino won't run this loop anymore and moves to next step
            }
          }
        }
        
        while (key_3Input == false) {
          lcd.clear();
          lcd.print("Please Press Key");
          char key_3 = keypad.waitForKey();
          if (key_3) {
            if (key_3 == '1') {
              while (confirm1 == false) {
                lcd.clear();
                lcd.print("1 selected");
                Serial.println("Cell 1 Selected");
                delay(1000);
                if (Serial.available() > 0) {
                  String message = Serial.readStringUntil('\n');  //Pi telling arduino it has confirmed
                  Serial.println(message);
                  if (message == "CV will look for Room 1") {
                    confirm1 = true;  //Arduino won't run this loop anymore and moves to next step
                    key_3Input = true;
                    lcd.clear();
                    lcd.print("Cell 1 Chosen");
                    delay(1500);
                  }
                }
              }
            } else if (key_3 == '2') {
              while (confirm2 == false) {
                Serial.println("Cell 2 Selected");
                if (Serial.available() > 0) {
                  String message = Serial.readStringUntil('\n');  //Pi telling arduino it has confirmed
                  Serial.println(message);
                  if (message == "CV will look for Room 2") {
                    confirm2 = true;  //Arduino won't run this loop anymore and moves to next step
                    key_3Input = true;
                    lcd.clear();
                    lcd.print("Cell 2 Chosen");
                    delay(1500);
                  }
                }
              }
            } else if (key_3 == 'B') {
              key_3Input = true;
              backToMenu();
              lcd.clear();
              lcd.print("Back to menu");
              delay(1000);
            } else {
              lcd.clear();
              lcd.print("Invalid Button");
              delay(1000);
            }
          }
        }

      } else if (key == '4') {  //Tell Pi to start OpenCV and move to ultrasonic sensor detection
       /*if ((cell_1 != true) || (cell_2 != true)) {
          lcd.clear();
          lcd.print("No Valid Room");
          delay(1000);
          lcd.clear();
          lcd.print("Please press '1'");
          delay(1000);
          lcd.print("On Menu");
          delay(1000);
        } else if ((confirm1 != true) || (confirm2 != true)) {
          lcd.clear();
          lcd.print("Which Cell?");
          delay(1000);
          lcd.clear();
          lcd.print("Please press '3'");
          delay(1000);
          lcd.print("On Menu");
          delay(1000);
        } else {
          menu = true;  //Will cause menu to not run anymore
          lcd.clear();
          lcd.print("Waiting for Pi");
          while (command_recieved == false) {
            Serial.println("Start Open CV");
            if (Serial.available() > 0) {
              String message = Serial.readStringUntil('\n');
              if (message == "Open CV will start")
                ;
              {
                command_recieved = true;
                lcd.clear();
                lcd.print("Starting in 5s");
                delay(5000);
                lcd.print("Vroom");
              }
            }
          }
        }*/
        lcd.print("Moving Soon");
        menu = true;
        delay(5000);
      } else {
        lcd.clear();
        lcd.print("Invalid Input");
        delay(1000);
      }
    }
    //Some while loop variable need to be set back to false incase user decides to change something
    sendingRoom_1OR2 = false;
    room_1_received = false;
    room_2_received = false;
    confirm1 = false;
    confirm2 = false;
    key_1Input = false;
    key_2Input = false;
    key_3Input = false;
  }

  
}

String backToMenu() {
  bool response = false;
  while (response == false) {
    Serial.println("Back to Menu");
    if (Serial.available() > 0) {
      String message = Serial.readStringUntil('\n');  //Pi telling arduino it has confirmed
      Serial.println(message);
      if (message == "Back to Menu Recieved") {
        response = true;  //Arduino won't run this loop anymore and moves to next step
        lcd.clear();
      }
    }
  }
}

boolean getID() {
  // Getting ready for Reading PICCs
  if (!mfrc522.PICC_IsNewCardPresent()) {  //If a new PICC placed to RFID reader continue
    return false;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {  //Since a PICC placed get Serial and continue
    return false;
  }
  tagID = "";
  for (uint8_t i = 0; i < 4; i++) {  // The MIFARE PICCs that we use have 4 byte UID
    //readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));  // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();  // Stop reading
  return true;
}

String inputRoom() {
  bool done = false;
  String room = "";
  lcd.print("Please Press Key");
  delay(1500);
  lcd.clear();
  while (done == false) {
    char key = keypad.waitForKey();
    if (key) {
      if (key == 'B') {
        unsigned int stringLength = room.length();  //Total number of characters in string
        room.remove(stringLength - 1);              //C Counts from 0 so last key entered index is 1 less than stringLength
        lcd.clear();
        lcd.print(room);
      } else if (key == 'D') {
        if (room == NULL) {
          lcd.clear();
          lcd.print("Room Invalid");
          lcd.setCursor(0, 1);
          lcd.print("Please put a #");
        } else {
          unsigned int stringLength = room.length();
          lcd.print(room);
          done = true;
        }
      } else if ((key == ('A')) || (key == ('C')) || (key == ('*')) || (key == ('#'))) {
        lcd.clear();
        lcd.print("Invalid button");
        lcd.setCursor(0, 1);
        lcd.print("Please put num");
      } else {
        room = (room += key);
        lcd.clear();
        lcd.print(room);
      }
    } else if (room.length() >= 8) {
      done = true;
      if (room.length() > 8) {
        unsigned int stringLength = room.length();
        room.remove(stringLength - 1);  //removes character 9th
      }
      lcd.clear();
      lcd.print(room);
      delay(1500);
      lcd.print("done");
    }
  }
  lcd.clear();
  lcd.print("#" + room);
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("done");
  delay(1500);
  return room;
}