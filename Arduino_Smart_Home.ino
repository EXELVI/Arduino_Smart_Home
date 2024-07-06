
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Password.h>
#include <Keypad.h>
#include <Servo.h>
#include <dht_nonblocking.h>
#include <DS3231.h>

DS3231 clock;
RTCDateTime dt;

int display = 0;

// Servo
LiquidCrystal_I2C lcd(0x27, 20, 4);
Servo myservo;
int pos = 1;
int passwd_pos = 10;
int garageopen = 0;

// DHT
#define DHT_SENSOR_TYPE DHT_TYPE_11
#define DHT_SENSOR_PIN 8
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// Password
Password password = Password("0000");

const byte ROWS = 4;
const byte COLS = 4;
char keypressed;
char keys[ROWS][COLS] = {
    {'1', '4', '7', '*'},
    {'2', '5', '8', '0'},
    {'3', '6', '9', '#'},
    {'A', 'B', 'C', 'D'}};
byte rowPins[ROWS] = {14, 15, 16, 17};
byte colPins[COLS] = {18, 19, 22, 23};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// int blueLED = 36;
int greenLED = 37;
int redLED = 38;
int pirPin1 = 6;
int pirPin2 = 7;
int GarageLED = 10;
int BedroomLED = 12;
int HallwayLED = 11;
int reedPin1 = 42;
int reedPin2 = 41;

int activeBuzzer = 4;
int passiveBuzzer = 5;

int fireSensor = 3;

int alarmStatus = 0;
const int zonesCount = 4;
bool zones[zonesCount] = {false, false, false, false};
bool disabledZones[zonesCount] = {false, false, false, false};
int lastZoneDisplayed = 0;

int alarmActive = 0;
bool adminMenu = false;
bool fireDet = false;
bool enteringPassword = false;
bool selectingZones = false;

bool buzzerState = false;

// Fire sensor delay
unsigned long previousMillis = 0;
long interval = 1000;

// Zones delay
unsigned long previousMillisZones = 0;
long intervalZones = 1000;

// Siren delay
unsigned long previousMillisSiren = 0;
long intervalSiren = 1000;
unsigned long previousMillisSiren2 = 0;
long intervalSiren2 = 500;

void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.init();
  lcd.backlight();

  Wire.begin();

  myservo.attach(9);

  clock.begin();

  displayCodeEntryScreen();

  pinMode(activeBuzzer, OUTPUT);
  pinMode(passiveBuzzer, OUTPUT);

  pinMode(fireSensor, INPUT);

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  pinMode(pirPin1, INPUT);  // Bedroom
  pinMode(pirPin2, INPUT);  // Garage
  pinMode(reedPin1, INPUT); // Front door
  pinMode(reedPin2, INPUT); // Garage door

  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);

  pinMode(GarageLED, OUTPUT);
  pinMode(BedroomLED, OUTPUT);
  pinMode(HallwayLED, OUTPUT);

  keypad.addEventListener(keypadEvent);
  myservo.write(pos);
}

static bool measure_environment(float *temperature, float *humidity)
{
  static unsigned long measurement_timestamp = millis();

  if (millis() - measurement_timestamp > 3000ul)
  {
    if (dht_sensor.measure(temperature, humidity) == true)
    {
      measurement_timestamp = millis();
      return (true);
    }
  }

  return (false);
}

long startTime = 0;
int detected = 0;

float temperature;
float humidity;

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    if (fireDet == true)
    {
      tone(passiveBuzzer, buzzerState ? 1000 : 3000);
      buzzerState = !buzzerState;
      if (currentMillis - startTime >= 5000)
      {
        digitalWrite(activeBuzzer, !digitalRead(activeBuzzer));
      }
      if (currentMillis - startTime >= 60000)
      {
        if (digitalRead(fireSensor) == HIGH)
          interval = 300;
        else if (interval != 300)
          interval = 500;
      }
    }
  }

  // Fire sensor
  if (digitalRead(fireSensor) == HIGH && !fireDet)
  {
    detected++;
    if (detected >= 3)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("FIRE DETECTED!");
      lcd.setCursor(0, 1);
      lcd.print("Enter PIN: ");
      startTime = currentMillis;
      fireDet = true;
    }
    else
      delay(1000);
  }

  if (alarmStatus == 1)
  {
    if (currentMillis - previousMillisSiren >= intervalSiren)
    {
      previousMillisSiren = currentMillis;
      tone(passiveBuzzer, buzzerState ? 500 : 4000);
      buzzerState = !buzzerState;
    }
    if (currentMillis - previousMillisSiren2 >= intervalSiren2)
    {
      previousMillisSiren2 = currentMillis;
      digitalWrite(activeBuzzer, !digitalRead(activeBuzzer));
    }
  }

  if (!alarmActive && !adminMenu && !fireDet && !enteringPassword)
  {
    if (display == 0)
    {
      // Temp & Humidity
      if (measure_environment(&temperature, &humidity) == true)
      {
        lcd.setCursor(0, 3);
        lcd.print("T: ");
        lcd.print(temperature);
        lcd.print("C");
        lcd.print(" H: ");
        lcd.print(humidity);
        lcd.print("%");
      }
    }

    else if (display == 1)
    {
      // Date & Time
      dt = clock.getDateTime();
      lcd.setCursor(0, 3);
      lcd.print(dt.year);
      lcd.print("-");
      lcd.print(String(dt.month).length() == 1 ? "0" + String(dt.month) : dt.month);
      lcd.print("-");
      lcd.print(String(dt.day).length() == 1 ? "0" + String(dt.day) : dt.day);
      lcd.print(" ");
      lcd.print(String(dt.hour).length() == 1 ? "0" + String(dt.hour) : dt.hour);
      lcd.print(":");
      lcd.print(String(dt.minute).length() == 1 ? "0" + String(dt.minute) : dt.minute);
      lcd.print(":");
      lcd.print(String(dt.second).length() == 1 ? "0" + String(dt.second) : dt.second);
    }
  }

  keypad.getKey();

  if (alarmActive == 1)
  {
    if (digitalRead(pirPin1) == HIGH && !disabledZones[0])
    {
      zones[0] = true;
      alarmTriggered();
    }
    if (digitalRead(reedPin1) == LOW && !disabledZones[1])
    {
      zones[1] = true;
      alarmTriggered();
    }
    if (digitalRead(reedPin2) == LOW && !disabledZones[2])
    {
      zones[2] = true;
      alarmTriggered();
    }

    if (digitalRead(pirPin2) == HIGH && !disabledZones[3])
    {
      zones[3] = true;
      alarmTriggered();
    }
  }

  if (alarmStatus == 1)
  {
    if (currentMillis - previousMillisZones >= intervalZones)
    {
      previousMillisZones = currentMillis;

      lcd.setCursor(0, 3);
      lcd.print("                    ");

      int count = countTrue(zones, zonesCount);

      if (count == 1)
        lastZoneDisplayed = 40;

      if (zones[0] && lastZoneDisplayed != 0)
      {
        lcd.setCursor(0, 3);
        lcd.print("Bedroom: Motion");
        lastZoneDisplayed = 0;
      }
      else if (zones[1] && lastZoneDisplayed != 1)
      {
        lcd.setCursor(0, 3);
        lcd.print("Front Door: Open");
        lastZoneDisplayed = 1;
      }
      else if (zones[2] && lastZoneDisplayed != 2)
      {
        lcd.setCursor(0, 3);
        lcd.print("Garage: Open");
        lastZoneDisplayed = 2;
      }
      else if (zones[3] && lastZoneDisplayed != 3)
      {
        lcd.setCursor(0, 3);
        lcd.print("Garage: Motion");
        lastZoneDisplayed = 3;
      }
    }
  }
}

int countTrue(bool arr[], int size)
{
  int count = 0;

  for (int i = 0; i < size; i++)
  {
    if (arr[i])
    {
      count++;
    }
  }

  return count;
}

void keypadEvent(KeypadEvent eKey)
{
  if (keypad.getState() == PRESSED)
  {
    Serial.println("Key pressed");
    Serial.println(eKey);
    if (adminMenu)
    {
      adminMenuKeyPressed(eKey);
    }
    else if (alarmActive == 1 || fireDet == 1 || enteringPassword)
    {
      if (eKey == '#')
      {
        enteringPassword = false;
        checkPassword();
      }
      else if (eKey == '*')
      {
        password.reset();
        passwd_pos = 10;
        lcd.setCursor(0, (enteringPassword ? 0 : 1));
        lcd.print("                ");
        lcd.setCursor(0, (enteringPassword ? 0 : 1));
        lcd.print("Enter PIN:");
      }
      else
      {
        if (passwd_pos < 14)
        {
          if (password.append(eKey))
          {
            lcd.setCursor(passwd_pos, (enteringPassword ? 0 : 1));
            lcd.print("*");
            passwd_pos++;
          }
        }
      }
    }

    else
    {
      if (eKey == 'A')
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter PIN:");
        enteringPassword = true;
        password.reset();
        passwd_pos = 10;
      }
      else if (eKey == 'B')
      {
        password.reset();
        passwd_pos = 10;
        adminMenu = true;
        displayCodeEntryScreen();
      }
      else if (eKey == '1')
      {
        digitalWrite(GarageLED, !digitalRead(GarageLED));
      }
      else if (eKey == '2')
      {
        digitalWrite(HallwayLED, !digitalRead(HallwayLED));
      }
      else if (eKey == '3')
      {
        digitalWrite(BedroomLED, !digitalRead(BedroomLED));
      }
      else if (eKey == 'D')
      {
        if (garageopen == 0)
        {
          for (pos = 0; pos <= 155; pos += 1)
          {
            myservo.write(pos);
            delay(10);
            garageopen = 1;
            digitalWrite(GarageLED, HIGH);
          }
        }
        else if (garageopen == 1)
        {
          for (pos = 155; pos >= 0; pos -= 1)
          {
            myservo.write(pos);
            delay(10);
            garageopen = 0;
            digitalWrite(GarageLED, LOW);
          }
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        password.reset();
        passwd_pos = 10;
        displayCodeEntryScreen();
      }
    }
  }
}

void toggleLight(int pin)
{
  digitalWrite(pin, !digitalRead(pin));
}

String getDisplay(int display)
{
  switch (display)
  {
  case 0:
    return "Temp & Humi";
    break;
  case 1:
    return "Date & Time";
    break;
  default:
    return "N/A";
  }
}

String getZonesName(int display)
{
  switch (display)
  {
  case 0:
    return "Bedroom: Motion";
    break;
  case 1:
    return "Front Door: Open";
    break;
  case 2:
    return "Garage: Open";
    break;
  case 3:
    return "Garage: Motion";
    break;
  default:
    return "N/A";
  }
}

int zoneSelected = 0;

void adminMenuKeyPressed(char key)
{

  if (selectingZones)
  {

    if (key == '0')
    {
      disabledZones[zoneSelected] = !disabledZones[zoneSelected];
    }
    else if (key == '*')
    {
      zoneSelected--;
      if (zoneSelected < 0)
      {
        zoneSelected = 0;
      }
    }
    else if (key == '#')
    {
      zoneSelected++;
      if (zoneSelected > 3)
      {
        zoneSelected = 3;
      }
    }
    lcd.setCursor(0, 1);
    lcd.print("                   ");
    lcd.setCursor(0, 1);
    lcd.print(getZonesName(zoneSelected));
    lcd.setCursor(0, 2);
    lcd.print("                   ");
    lcd.setCursor(0, 2);
    lcd.print(disabledZones[zoneSelected] ? "Disabled" : "Enabled");
    if (key == 'D')
    {
      selectingZones = false;
      displayCodeEntryScreen();
    }
  }
  else if (key == 'C')
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Select Zones D. Exit");
    lcd.setCursor(0, 3);
    lcd.print("<- * (Sel: 0) # ->");
    lcd.setCursor(0, 1);
    lcd.print(getZonesName(zoneSelected));
    lcd.setCursor(0, 2);
    lcd.print(disabledZones[zoneSelected] ? "Disabled" : "Enabled");
    selectingZones = true;
  }
  else if (key == 'D')
  {
    adminMenu = false;
    displayCodeEntryScreen();
  }
  else if (key == 'A')
  {
    bool enteringPassword = true;
    bool changingPIN = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter PIN:");
    lcd.setCursor(0, 3);
    lcd.print("D. Cancel");

    while (enteringPassword)
    {
      char key = keypad.getKey();

      if (key)
      {
        if (key == '#')
        {

          passwd_pos = 10;
          if (password.evaluate())
          {
            changingPIN = true;
            password.reset();
            passwd_pos = 10;
            break;
          }
          else
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Invalid PIN!");
            password.reset();
            passwd_pos = 10;
            delay(2000);
            enteringPassword = false;
          }
        }
        else if (key == '*')
        {
          password.reset();
          passwd_pos = 10;
          lcd.setCursor(0, 0);
          lcd.print("                ");
          lcd.setCursor(0, 0);
          lcd.print("Enter PIN:");
        }
        else if (key == 'D')
        {
          enteringPassword = false;
        }
        else
        {
          if (passwd_pos < 14)
          {
            password.append(key);
            lcd.setCursor(passwd_pos, 0);
            lcd.print("*");
            passwd_pos++;
          }
        }
      }
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter New PIN:");
    lcd.setCursor(0, 3);
    lcd.print("D. Cancel");

    String newPIN = "";
    int pinPos = 0;

    while (changingPIN)
    {
      char key = keypad.getKey();

      if (key)
      {
        if (key == '#')
        {
          if (newPIN.length() == 4)
          {
            char char1 = newPIN.charAt(0);
            char char2 = newPIN.charAt(1);
            char char3 = newPIN.charAt(2);
            char char4 = newPIN.charAt(3);
            char *newPINChar = new char[4]{char1, char2, char3, char4};
            password = newPINChar;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("PIN changed!");
            delay(2000);
            changingPIN = false;
          }
          else
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Invalid PIN!");
            Serial.print("New PIN: ");
            Serial.println(newPIN);
            delay(2000);
            changingPIN = false;
          }
        }
        else if (key == 'D')
        {
          changingPIN = false;
        }
        else if (key == '*')
        {
          if (pinPos > 0)
          {
            pinPos--;
            newPIN = newPIN.substring(0, pinPos);
            lcd.setCursor(pinPos, 1);
            lcd.print(" ");
          }
        }
        else
        {
          if (pinPos < 4)
          {
            newPIN += key;
            lcd.setCursor(pinPos, 1);
            lcd.print("*");
            pinPos++;
          }
        }
      }
    }
    displayCodeEntryScreen();
  }
  else if (key == 'B')
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Choose a Display");
    lcd.setCursor(0, 1);
    lcd.print("Selected: " + getDisplay(display));
    lcd.setCursor(0, 3);
    lcd.print("<- * (Sel: 0) # ->");

    bool selecting = true;
    int selectedItem = display;

    while (selecting)
    {
      char key = keypad.getKey();

      if (key == '0')
      {
        display = selectedItem;
        selecting = false;
      }
      else if (key == '*')
      {
        selectedItem--;
        lcd.setCursor(0, 2);
        lcd.print("              ");
        if (selectedItem < 0)
        {
          selectedItem = 0;
        }
      }
      else if (key == '#')
      {
        selectedItem++;
        lcd.setCursor(0, 2);
        lcd.print("              ");
        if (selectedItem > 2)
        {
          selectedItem = 2;
        }
      }

      lcd.setCursor(0, 2);
      lcd.print(getDisplay(selectedItem));
    }
    displayCodeEntryScreen();
  }
}

void alarmTriggered()
{
  alarmStatus = 1;
  lcd.setCursor(0, 1);
  lcd.print("Enter PIN:");
  lcd.setCursor(0, 2);
  lcd.print("  SYSTEM TRIGGERED  ");
  lcd.setCursor(0, 3);
}

void checkPassword()
{
  Serial.println("Checking password");
  Serial.println(password.evaluate());
  if (password.evaluate())
  {
    if (fireDet)
    {
      fireDet = false;
      detected = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("FIRE ALARM OFF!");
      digitalWrite(activeBuzzer, LOW);
      noTone(passiveBuzzer);
      delay(2000);
      displayCodeEntryScreen();
    }
    else if (alarmActive == 0 && alarmStatus == 0)
    {
      activate();
    }
    else if (alarmActive == 1 || alarmStatus == 1)
    {
      deactivate();
    }
  }
  else
  {
    invalidCode();
  }
}

void invalidCode()
{

  lcd.clear();
  lcd.setCursor(5, 1);
  lcd.print("WRONG PIN!");
  lcd.setCursor(5, 2);
  lcd.print("TRY AGAIN!");
  digitalWrite(greenLED, LOW);
  password.reset();
  passwd_pos = 10;
  tone(passiveBuzzer, 1000);
  digitalWrite(redLED, HIGH);
  delay(100);
  digitalWrite(redLED, LOW);
  noTone(passiveBuzzer);
  delay(100);
  tone(passiveBuzzer, 1000);
  digitalWrite(redLED, HIGH);
  delay(100);
  digitalWrite(redLED, LOW);
  noTone(passiveBuzzer);
  delay(100);
  tone(passiveBuzzer, 1000);
  digitalWrite(redLED, HIGH);
  delay(100);
  digitalWrite(redLED, LOW);
  noTone(passiveBuzzer);
  delay(100);

  if (!alarmActive && !fireDet)
  {
    displayCodeEntryScreen();
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM ACTIVE!");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("Enter PIN:");
    lcd.setCursor(0, 2);
    lcd.print("                ");
  }
}

void activate()
{
  if (garageopen == 1)
  {
    for (pos = 155; pos >= 0; pos -= 1)
    {
      myservo.write(pos);
      delay(10);
      garageopen = 0;
      digitalWrite(GarageLED, LOW);
    }
  }

  Serial.print("reedPin1: ");
  Serial.println(digitalRead(reedPin1));
  Serial.print("reedPin2: ");
  Serial.println(digitalRead(reedPin2));
  if ((digitalRead(reedPin1) == HIGH || disabledZones[1]) && (digitalRead(reedPin2) == HIGH || disabledZones[2]))
  {
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    digitalWrite(2, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM ACTIVE!");

    alarmActive = 1;
    password.reset();
    passwd_pos = 10;
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("Enter PIN:");
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM NOT READY!");
    lcd.setCursor(0, 1);
    lcd.print("Close all doors!");
    delay(2000);
    password.reset();
    passwd_pos = 10;
    displayCodeEntryScreen();
  }
}

void deactivate()
{
  alarmStatus = 0;
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  digitalWrite(activeBuzzer, LOW);
  noTone(passiveBuzzer);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" SYSTEM DEACTIVATED!");
  zones[0] = false;
  zones[1] = false;
  zones[2] = false;
  zones[3] = false;
  alarmActive = 0;
  password.reset();
  passwd_pos = 10;
  delay(5000);

  displayCodeEntryScreen();
}

void displayCodeEntryScreen()
{
  if (adminMenu)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Admin Menu   D: Exit");
    lcd.setCursor(0, 1);
    lcd.print("A: Change PIN");
    lcd.setCursor(0, 2);
    lcd.print("B: Change Display");
    lcd.setCursor(0, 3);
    lcd.print("C. Disable Zones");
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("A: Activate");
    lcd.setCursor(0, 1);
    lcd.print("B: Admin Menu");
    lcd.setCursor(0, 2);
    lcd.print("1-3 Leds   D: Garage");
    if (display == 0)
    {
      lcd.setCursor(0, 3);
      lcd.print("T: ");
      lcd.print(temperature);
      lcd.print("C");
      lcd.print(" H: ");
      lcd.print(humidity);
      lcd.print("%");
    }

    else if (display == 1)
    {
      dt = clock.getDateTime();
      lcd.setCursor(0, 3);
      lcd.print(dt.year);
      lcd.print("-");
      lcd.print(String(dt.month).length() == 1 ? "0" + String(dt.month) : dt.month);
      lcd.print("-");
      lcd.print(String(dt.day).length() == 1 ? "0" + String(dt.day) : dt.day);
      lcd.print(" ");
      lcd.print(String(dt.hour).length() == 1 ? "0" + String(dt.hour) : dt.hour);
      lcd.print(":");
      lcd.print(String(dt.minute).length() == 1 ? "0" + String(dt.minute) : dt.minute);
      lcd.print(":");
      lcd.print(String(dt.second).length() == 1 ? "0" + String(dt.second) : dt.second);
    }
  }
}
