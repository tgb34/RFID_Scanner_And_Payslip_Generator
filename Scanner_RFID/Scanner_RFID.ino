//Created by Timothy Birkner, 2017

#include <MFRC522.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

String ID, currDay;

int dayCount = 0, rowCount = 2, dayBegin = 0, employeeEndRow;

void setup() {

  Serial.begin(9600);
  Serial.println("LABEL,ID,Time,Day,Shift,Start/End");      //Add ID, current Date, Time, shift number, and start/end of shift to excel columns
  setRow();
  setDay();
  setdayBegin();
  getEmployeeCount();
  Serial.print("ROW,SET,");
  Serial.println(rowCount);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  lcd.begin(16, 2);
  lcd.clear();
  //Serial.println("Approximate your card to the reader...");
  //Serial.println();
}

void loop() {

  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    lcd.clear();
    lcd.print("No ID Found");

    delay(1500);

    lcd.clear();
    lcd.print("Please Scan ID");
    delay(1500);

    return;
  }
  else {
    lcd.clear();
    lcd.print("Scanning...");

    generateID();

    lcd.clear();
    lcd.print("OK To Remove ID");

    if (!ID.equals("")) {
      if (getName().length() > 2) {

        //Add ID, current Date, Time, shift number, and start/end of shift to excel row locaed at rowCount
        Serial.print("DATA," + ID);
        Serial.print(",TIME");
        Serial.print(",DATE,");
        Serial.print("10,");   //arbitrary values for shift and enter/exit because we don't know those yet
        Serial.println("null");
        setStringTime();

        //Now update for real by checking shift #, enter/exit, day, and more
        doubleCheck();
      }
    }
    else {
      delay(700);
      lcd.clear();
      lcd.print("Please Rescan ID");
      lcd.setCursor(0, 1);
      lcd.print("Try Other Side");
      delay(1500);
      lcd.clear();
    }

    //}
    ID = "";
  }
}

//Add appropriate info to Excel sheet
void doubleCheck() {
  Serial.print("CELL,GET,C");
  Serial.println(rowCount);

  String checkDay = Serial.readStringUntil(10);

  //if the newly entered day doesn't match the currentDay,
  //that means this new ID entry is the start of a new day
  Serial.println(currDay);
  Serial.println(checkDay);
  if (!checkDay.equals(currDay)) {
    currDay = checkDay;
    dayBegin = rowCount;
    Serial.println("New Day");
  }

  String checkName;
  String enter;

  //Iterate through all the names in a day, beginning
  //from the row of the first entry of the day
  for (int i = rowCount - 1; i >= dayBegin; i--) {
    Serial.print("CELL,GET,A");
    Serial.println(i);
    checkName = Serial.readStringUntil(10);

    //If the name from one row matches the ID you are trying
    //to enter, there is a duplicate, so enter this check
    if (checkName.equals(ID)) {
      Serial.print("CELL,GET,E");
      Serial.println(i);
      enter = Serial.readStringUntil(10);

      //If the most recent entry from the employee was the
      //start of a shift, then this current entry must be
      //the end of the same shift, so update accordingly
      if (enter.equals("Start")) {
        Serial.println("Enter = Start");
        updateRow(true, i);
        rowCount = rowCount + 1;
        return;
      }

      //If the most recent entry from the employee was the
      //end of a shift, then this current entry must be
      //the start of a new shift, so update accordingly
      if (enter.equals("End")) {
        Serial.println("Enter = End");

        updateRow(false, i);
        rowCount = rowCount + 1;
        return;
      }
    }
  }

  //If we leave the loop without finding either an Enter or Exit
  //entry, then we know this is the first entry of the day
  Serial.print("CELL,SET,D");
  Serial.print(rowCount);
  Serial.println(",1");
  Serial.print("CELL,SET,E");
  Serial.print(rowCount);
  Serial.println(",Start");
  Serial.println("First Entry of Day");
  rowCount++;
  return;
}

//Used so the program knows what row to start adding info to
void setRow() {
  int i = 2;
  Serial.println("CELL,GET,A2");
  int nullCheck = Serial.readStringUntil(10).toInt();
  Serial.println(nullCheck);
  Serial.println("CELL,GET,B2");
  String check00000 = Serial.readStringUntil(10);
  String check = "CELL,GET,A";
  String check000002 = "CELL,GET,B";

  while (nullCheck != 0 || check00000.length() > 2) {
    i = i + 1;
    Serial.println(check + i);
    nullCheck = Serial.readStringUntil(10).toInt();
    Serial.println(nullCheck);
    Serial.println(check000002 + i);
    check00000 = Serial.readStringUntil(10);
    Serial.println(check00000);

  }
  rowCount = i;
  Serial.print("The row is:");
  Serial.println(rowCount);
  return;
}

//Used so the program knows what the last day entered was
void setDay() {
  String check = "CELL,GET,C";
  Serial.println(check + (rowCount - 1));
  String day = Serial.readStringUntil(10);
  Serial.print("The day is:");
  Serial.println(day);
  currDay = day;
  return;
}

//Used to know what row the current day from setDay() starts from, so the program doesn't
//need to scan the entire document from every single day
void setdayBegin() {
  String check = currDay;
  String pull = "CELL,GET,C";
  int i = rowCount - 1;
  while (check.equals(currDay)) {
    Serial.println(pull + (i - 1));
    check = Serial.readStringUntil(10);
    i = i - 1;
  }
  dayBegin = i + 1;
  Serial.print("Day begins on row:");
  Serial.println(dayBegin);
  return;
}

//Checks what the last shift for an ID was, and adds the proper information (start or end, shift 1, 2, 3... etc)
void updateRow(bool start, int i) {
  Serial.println("The method was visited");
  int shift;
  Serial.print("CELL,SET,E");
  Serial.print(rowCount);
  if (!start) {
    Serial.println(",Start");
    Serial.println("Start was visited");

  }
  else {
    Serial.println(",End");
    Serial.println("End was visited");

  }
  Serial.print("CELL,GET,D");
  Serial.println(i);
  shift = Serial.readStringUntil(10).toInt();
  if (!start) {
    shift = shift + 1;

  }
  Serial.println("Shift = ");
  Serial.println(shift);
  Serial.print("CELL,SET,D");
  Serial.print(rowCount);
  Serial.print(",");
  Serial.println(shift);
  return;
}

void getEmployeeCount() {
  int j = 2;
  String thing;
  Serial.print("CELL,GET,FROMSHEET,IDs,A,");
  Serial.println(j);
  thing = Serial.readStringUntil(10);

  while (thing != "") {
    j = j + 2;
    Serial.print("CELL,GET,FROMSHEET,IDs,A,");
    Serial.println(j);
    thing = Serial.readStringUntil(10);
  }

  employeeEndRow = j;
}

String getName() {
  String thing;
  Serial.print("CELL,GET,E");
  Serial.println(rowCount - 1);
  String enter = Serial.readStringUntil(10);

  for (int i = 2; i <= employeeEndRow; i++) {
    Serial.print("CELL,GET,FROMSHEET,IDs,A,");
    Serial.println(i - 1);
    thing = Serial.readStringUntil(10);
    if (thing.equals(ID)) {
      lcd.clear();
      if (enter.equals("Start/End") || enter.equals("End")) {
        lcd.print("Welcome");
      }
      else {
        lcd.print("Thank you");
      }

      Serial.print("CELL,GET,FROMSHEET,IDs,A,");
      Serial.println(i);
      String employee;
      employee = Serial.readStringUntil(10);
      i = 0;

      lcd.setCursor(0, 1);
      lcd.print(employee);
      delay(3000);
      lcd.clear();
      return employee;
    }
  }
  
  lcd.clear();
  lcd.println("Error");
  lcd.println("Scan again");
  delay(2000);
  
  return "";
}

//Converts integer to string (i.e. from a number to letters since computers
//"interpret" text and numbers differently
void generateID() {

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  Serial.print("UID tag :");
  ID = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    ID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    ID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  ID = ID.substring(1, ID.length());
  Serial.println();
  ID.toUpperCase();

  return;
}

void setStringTime() {
  String getter = "CELL,GET,H";
  String setter1 = "CELL,SET,H";
  // String setter2 = ",\"=TEXT(B16,\"hh:mm AM/PM\")"

  Serial.println(getter + rowCount);
  String stringTime = Serial.readStringUntil(10);

  String fakeminutes = stringTime.substring(stringTime.length() - 5, stringTime.length() - 3);
  int minutes = fakeminutes.toInt();
  String rounded = "";
  bool roundUp = false;


    if(minutes < 8){
      rounded = ":00";
    }
    else if(minutes < 23){
      rounded = ":15";
    }
    else if(minutes < 38){
      rounded = ":30";
    }
    else if(minutes < 53){
      rounded = ":45";
    }
    else{
      rounded = ":00";
      roundUp = true;
  }

  String total1 = stringTime.substring(0, stringTime.length() - 6);

  if (roundUp) {
    if (total1.equals("12")) {
      total1 = "01";
    }
    else {
      total1 = String(total1.toInt() + 1);
    }
  }
  
  String timeOfDay = stringTime.substring(stringTime.length() - 3, stringTime.length());

  if(timeOfDay.equals(" AM") && roundUp && total1.equals("12")){
    timeOfDay = " PM";
  }
  else if(timeOfDay.equals(" PM") && roundUp && total1.equals("12")){
    timeOfDay = " AM";
  }
  Serial.println(setter1 + rowCount + ',' + total1 + rounded + timeOfDay);
}


