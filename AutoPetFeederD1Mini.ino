//Created by Betsy Farris (betsyfarris.com)
//Main inspiration and hardware choices: https://www.instructables.com/id/Internet-Enabled-Raspberry-Pi-Pet-Feeder/
//codes used/edited  https://github.com/blynkkk/blynk-library/blob/master/examples/Widgets/RTC/RTC.ino
//Thanks to Kyle Greer (Robo-Farmer-Extraordinaire) for his stylistic comments and help with the Blynk code.
/*****setup on Blynk (manual tab)****/
//V5=slider for cups to feed (0-100)
//V3= time lcd
//V4= date lcd
//V1= push button for manual feeding
// include real-time clock with current timezone
/*****setup on Blynk (scheduler tab)****/
//V8= scheduler on/off push button
//V6 morning feed time (set to start/stop for 1sec)
//V7 evening feed time (set to start/stop for 1sec)
//include notification widget (hardware notify off, low priority)

#define BLYNK_PRINT Serial


#include <BlynkSimpleEsp8266.h>
#include <WidgetRTC.h>
#include <Servo.h>

Servo myservo;

/******* delcare functions****/
void schedulechecker();
void clockDisplay();

/********************* WIDGET SETUP **********************/

//LCDs
WidgetLCD lcd(V3);
//WidgetLCD lcd1(V4);

// RTC
BlynkTimer timer;
WidgetRTC rtc;


/********************* WifiSettings **********************/
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "AUTHKEY FOR BLYNK";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SSID";
char pass[] = "PASS";

/****numbers***/
int servoPin = D4;
int scheduler = 0;
int timeornot = 0; //initialize feed timer to be off
int motorturns = 500; //default time to dispense 10 s

const int buttonPin = D6;
const int BUILTINLED = 2;
long buttonTimer = 0;
long longPressTime = 1000;
int button_state = HIGH;
boolean buttonActive = false;
boolean longPressActive = false;






/******** CLOCK DISPLAY *******/


BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
  Blynk.syncVirtual(V5);
}

/******** CLOCK DISPLAY *******/
void clockDisplay() //not sure if I really need this but it updates the serial monitor
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();

}


/************** SETUP SETUP SETUP SETUP SETUP *****************/
void setup()
{
  Serial.begin(115200); //baude rate of monitor
  //pinMode(motorPin, OUTPUT); // digital pin (12) on huzzah
  pinMode(buttonPin, INPUT_PULLUP);
  myservo.write(180);
  myservo.attach(servoPin);

  myservo.write(180);
  //delay(1000);
  //myservo.detach();

  /******** Connect Blynk ***********/
  Blynk.begin(auth, ssid, pass);
  rtc.begin();

  setSyncInterval(10 * 60); // Sync interval in seconds (10 min)
  timer.setInterval(1000L, clockDisplay);
}

/************ LOOP  LOOP   LOOP   LOOP   LOOP********/


void loop()
{
  Blynk.run(); //runs all Blynk functions
  timer.run();
  schedulerchecker(); //looks for the button for using the schedules times and if the timers get pinged
  buttonHandler();
}


void buttonHandler() {
  int button_state = digitalRead(buttonPin);
  if (button_state == LOW) {
    digitalWrite(BUILTINLED, LOW);
    if (buttonActive == false) {
      buttonActive = true;
      buttonTimer = millis();
    }
  } else {
    digitalWrite(BUILTINLED, HIGH);
    if (buttonActive == true) {
      if (millis() - buttonTimer > 50) { //Debounce
        feed(motorturns);
        String currentTime = String(hour()) + ":" + minute() + ":" + second();
        String currentDate = String(day()) + "/" + month();
        Blynk.notify("Manual Fed via button @" + currentTime);
        lcd.clear();
        lcd.print(0, 0, "Time Last Fed: ");
        lcd.print(0, 1, currentTime + ", " + currentDate);


      }
      buttonActive = false;
    }
  }
}

BLYNK_WRITE(V1) //Button Widget is writing to pin V1, manual feed
{
  int food = param.asInt();
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + "/" + month();

  if (food == HIGH) {
    feed(motorturns);

    Serial.println("Fed");
    Blynk.virtualWrite(V1, LOW);
    lcd.clear();
    lcd.print(0, 0, "Time Last Fed: ");
    lcd.print(0, 1, currentTime + ", " + currentDate);


    Serial.println("Fed");
  }
}

///****Scheduler****/
BLYNK_WRITE(V6) //morning pin
{ if  (param.asInt() == 1)
  { timeornot = 1;
  }
  if (param.asInt() == 0)
  { timeornot = 0;
  }
}

BLYNK_WRITE(V7) //evening pin
{ if  (param.asInt() == 1)
  { timeornot = 1;
  }
  if (param.asInt() == 0)
  { timeornot = 0;
  }
}

BLYNK_WRITE(V8) // scheduler on or off
{
  if (param.asInt() == 1) {
    scheduler = 1;
    timeornot = 0;
    Serial.println("Scheduler is ON");
  }
  if (param.asInt() == 0) {
    scheduler = 0;
    Serial.println("Scheduler is Off");
  }
}

void schedulerchecker()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + "/" + month();

  if (scheduler == 1 && timeornot == 1) //autofeed times
  { Blynk.notify("Auto Fed @" + currentTime);

    feed(motorturns);

    Serial.println("Scheduled Feeding");
    Blynk.virtualWrite(V1, LOW);
    lcd.clear();
    lcd.print(0, 0, "Time Auto Fed: ");
    lcd.print(0, 1, currentTime + ", " + currentDate);
    Serial.println("Fed");

  }
  timeornot = 0;
}


/****FOR DEMO ONLY (dispense candy; not needed to feed dog)***/
BLYNK_WRITE(V5)
{ int cup_slider = param.asInt(); //percentage of 1 cup (e.g. 100=1cup)
  motorturns = cup_slider;
}

void feed(int motorturnsArg) {

  myservo.attach(servoPin);

  myservo.write(115);
  int delayTime = 1100.0 * (motorturnsArg / 1000.0);
  Serial.println(motorturns);
  Serial.println(motorturns / 1000.0);
  Serial.println(delayTime);
  delay(delayTime);
  myservo.write(180);
  //delay(1000);
  //myservo.detach();
}
