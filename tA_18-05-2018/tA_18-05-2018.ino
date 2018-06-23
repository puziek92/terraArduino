#include <LiquidCrystal_I2C.h>
#include <DS18B20.h>
#include <OneWire.h>
#include <DHT.h>
#include <Wire.h>
#include <DS3231.h>

//---------------Czujnik lewy dolny----------------
#define DHT_DL_PIN 4 //czujnik DL
#define DHT_DL_TYPE DHT22
DHT dhtDL(DHT_DL_PIN, DHT_DL_TYPE);
//-------------------------------------------------

//---------------Czujnik prawy gora----------------
#define DHT_GP_PIN 7 //czujnik DL
#define DHT_GP_TYPE DHT11
DHT dhtGP(DHT_GP_PIN, DHT_GP_TYPE);
//-------------------------------------------------

//---------------Czujnik prawy dol-----------------
#define DHT_DP_PIN 8 //czujnik DL
#define DHT_DP_TYPE DHT11
DHT dhtDP(DHT_DP_PIN, DHT_DP_TYPE);
//-------------------------------------------------

//---------------Czujnik wyspa---------------------
#define ONEWIRE_PIN 5
byte address[8] = {0x28, 0x29, 0x86, 0x1E, 0x0, 0x0, 0x80, 0xC8};
OneWire onewire(ONEWIRE_PIN);
DS18B20 sensors(&onewire);
//-------------------------------------------------

//---------------ekran I2C-------------------------
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27
//-------------------------------------------------

DS3231 clock;
RTCDateTime dt;


//---------------Znaki Specjalne----------------
byte degreesC[8] = {
  0b01000,
  0b10100,
  0b01000,
  0b00110,
  0b01001,
  0b01000,
  0b01001,
  0b00110
};
byte moon[8] = {
  0b00110,
  0b01111,
  0b11100,
  0b11000,
  0b11000,
  0b11100,
  0b01111,
  0b00110
};
byte sun[8] = {
  0b10001,
  0b01010,
  0b01110,
  0b11111,
  0b01110,
  0b01010,
  0b10001,
  0b00000
};
byte lampOFF[8] = {
  0b00100,
  0b10001,
  0b00100,
  0b01010,
  0b10001,
  0b01010,
  0b00100,
  0b01110
};
byte lampON[8] = {
  0b00100,
  0b10001,
  0b00100,
  0b01110,
  0b11111,
  0b01110,
  0b00100,
  0b01110
};
byte lampNIGHT[8] = {
  0b11011,
  0b01110,
  0b11011,
  0b10001,
  0b00000,
  0b10001,
  0b11011,
  0b10001
};
//------------Koniec Znaki Specjalne-------------

int lampOn = 7;
int lampOff = 16;
int statusLed = 0;
int statusHalogen = 0;
int statusFan = 0;
int statusCable = 0;
int statusSensorScreen = 0;
int buttonKeyboardValue;
int i = 0;


void setup()
{
  Serial.begin(9600);
  lcd.begin(16,2);   // Inicjalizacja LCD 2x16
  sensors.begin(9);
  sensors.request(address);
  //------------FAN PWM--------------
  pinMode(11, OUTPUT);
  //----------END FAN PWM------------
  //przekaźnik sterowany stanem niskim
  pinMode(2, OUTPUT); //halogen
  pinMode(3, OUTPUT); //led
  // inicjalizacja czujnika
  dhtDL.begin();
  dhtGP.begin();
  dhtDP.begin();
  // Inicjalizacja DS3231
  clock.begin();

  // Ustawiany date i godzine kompilacji szkicu
  //clock.setDateTime(__DATE__, __TIME__);
}

void loop()
{
  // Odczytujemy i wyswietlamy czas
  dt = clock.getDateTime();

  if (statusLed == 1)
  {
    digitalWrite(3, HIGH);
  }
  else
  {
    digitalWrite(3, LOW);
  }

  if (dt.hour >= lampOn && dt.hour < lampOff)
  {
    digitalWrite(2, HIGH);
    statusHalogen = 1;
  }
  if (dt.hour >= lampOff || dt.hour < lampOn)
  {
    digitalWrite(2, LOW);
    statusHalogen = 0;
  }

  if (statusFan == 1)
  {
    analogWrite(11, 255);
  } else
  {
    analogWrite(11, 0);
  }
  //statusInfo();
  keyboard();
  screen();

  delay(1000);
}

/*void statusInfo()
{
  Serial.print("wartosc przycisku: "); Serial.println(buttonKeyboardValue);
   if (sensors.available())
  {
    int temperature = sensors.readTemperature(address);
    Serial.println("IH: ");
    Serial.print(temperature);
    sensors.request(address);
  }
  Serial.println("DL: "); showSensorStatus(dhtDL);
  Serial.println("GP: "); showSensorStatus(dhtGP);
  Serial.println("DP: "); showSensorStatus(dhtDP);
  Serial.println("");
}*/

void screen()
{
  //---------------Znaki Specjalne----------------
  lcd.createChar(0, moon);
  lcd.createChar(1, sun);
  lcd.createChar(2, lampOFF);
  lcd.createChar(3, lampON);
  lcd.createChar(4, lampNIGHT);
  lcd.createChar(5, degreesC);
  //------------Koniec Znaki Specjalne------------
  lcd.backlight(); // zalaczenie podwietlenia 
  lcd.setCursor(0,0); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna)
  if (dt.hour >= lampOn && dt.hour < lampOff) // znaczek slonce - ksiezyc
  {
    lcd.print((char)1);
  } else {
    lcd.print((char)0);
  }
  lcd.print(clock.dateFormat("d-m H:i:s",dt));
  
  lcd.setCursor(0,1); //Ustawienie kursora w pozycji 0,0 (drugi wiersz, pierwsza kolumna)
  if (statusHalogen==1)
  {lcd.print("H:"); lcd.print((char)3);}
  else
  {{lcd.print("H:"); lcd.print((char)2);}}
  if (statusLed==1)
  {lcd.print("L:"); lcd.print((char)3);}
  else
  {{lcd.print("L:"); lcd.print((char)2);}}
  screenSwitcher(statusSensorScreen);
  
}

void keyboard()
{
  buttonKeyboardValue = analogRead(0);
  if ( buttonKeyboardValue > 955 && buttonKeyboardValue < 1030 )
  {
    //halogen
    if (statusHalogen == 1)
    {
      statusHalogen = 0;
    } else
    {
      statusHalogen = 1;
    }
    //Serial.print("halogen");
  } else if ( buttonKeyboardValue > 475 && buttonKeyboardValue < 530 )
  {
    //led
    if (statusLed == 1)
    {
      statusLed = 0;
    } else
    {
      statusLed = 1;
    }
    //Serial.print("led");
  } else if ( buttonKeyboardValue > 315 && buttonKeyboardValue < 370 )
  {
   if(statusSensorScreen == 3)
   {
    statusSensorScreen = 0;
   }else{
    statusSensorScreen++;
   }
  } else if ( buttonKeyboardValue > 246 && buttonKeyboardValue < 275 )
  {
    if (statusFan == 1)
    {
      statusFan = 0;
    } else
    {
      statusFan = 1;
    }
  }
}

void showSensor(DHT sensor)
{
  int t = sensor.readTemperature();
  int h = sensor.readHumidity();
 
  // Sprawdzamy czy są odczytane wartości
  if (isnan(t) || isnan(h))
  {
    // Jeśli nie, wyświetlamy informację o błędzie
    lcd.print("ERR");
  } else
  {
    // Jeśli tak, wyświetlamy wyniki pomiaru
    lcd.print(t);
    lcd.print((char)5);
    lcd.print(",");
    lcd.print(h);
    lcd.print("%");
  }
}

/*void showSensorStatus(DHT sensor)
{
  int t = sensor.readTemperature();
  int h = sensor.readHumidity();
 
  // Sprawdzamy czy są odczytane wartości
  if (isnan(t) || isnan(h))
  {
    // Jeśli nie, wyświetlamy informację o błędzie
    Serial.println("ERR");
  } else
  {
    // Jeśli tak, wyświetlamy wyniki pomiaru    
    Serial.print("T:");
    Serial.print(t);
    Serial.print("*C");
    Serial.print("H:");
    Serial.print(h);
    Serial.println("5%");
  }

    if (sensors.available())
  {
    int temperature = sensors.readTemperature(address);

        Serial.println("wyspa ciepla: ");
    Serial.print(temperature);
    sensors.request(address);
  }
}*/

void screenSwitcher(int stat)
{
  switch (stat) {
  case 0:
    lcd.print("DL:");
    showSensor(dhtDL);
    break;
  case 1:
    lcd.print("GP:");
    showSensor(dhtGP);
    break;
  case 2:
    lcd.print("DP:");
    showSensor(dhtDP);
    break;
  case 3:
    lcd.print("IH:");
    if (sensors.available())
  {
    int temperature = sensors.readTemperature(address);

    lcd.print(temperature);
    lcd.print("      ");
    sensors.request(address);
  }
    break;
}
}

