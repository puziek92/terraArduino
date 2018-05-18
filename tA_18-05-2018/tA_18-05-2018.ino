#include <LiquidCrystal_I2C.h>
#include <DS18B20.h>
#include <OneWire.h>
#include <DHT.h>
#include <Wire.h>
#include <DS3231.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27

#define DHT_DL_PIN 4 //czujnik DL
#define DHT_DL_TYPE DHT22
#define ONEWIRE_PIN 5
//---------------Znaki Specjalne----------------
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
byte address[8] = {0x28, 0x29, 0x86, 0x1E, 0x0, 0x0, 0x80, 0xC8};
OneWire onewire(ONEWIRE_PIN);
DS18B20 sensors(&onewire);
DHT dht(DHT_DL_PIN, DHT_DL_TYPE);
DS3231 clock;
RTCDateTime dt;
int lampOn = 7;
int lampOff = 16;
int statusLed = 0;
int statusHalogen = 0;
int statusFan = 0;
int statusCable = 0;
int buttonKeyboardValue;
int i = 0;


void setup()
{
  Serial.begin(9600);
  lcd.begin(16,2);   // Inicjalizacja LCD 2x16
  sensors.begin(9);
  sensors.request(address);
  // stany wyjściowe oświetlenia
  //przekaźnik sterowany stanem niskim
  pinMode(2, OUTPUT); //halogen
  pinMode(3, OUTPUT); //led
  // inicjalizacja czujnika
  // Inicjalizacja DS3231
  dht.begin();
  Serial.println("Initialize DS3231");
  clock.begin();

  // Ustawiany date i godzine kompilacji szkicu
  //clock.setDateTime(__DATE__, __TIME__);
}

void loop()
{
  // Odczytujemy i wyswietlamy czas
  dt = clock.getDateTime();
  //odczytanie stanu przycisku i przypisanie go do zmiennej sensorVal
  ekran();
  keyboard();
  sensor();

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
  }
  if (dt.hour >= lampOff || dt.hour < lampOn)
  {
    digitalWrite(2, LOW);
  }

  delay(1000);
}

void ekran()
{
  //---------------Znaki Specjalne----------------
  lcd.createChar(0, moon);
  lcd.createChar(1, sun);
  lcd.createChar(2, lampOFF);
  lcd.createChar(3, lampON);
  lcd.createChar(4, lampNIGHT);
  //------------Koniec Znaki Specjalne------------
  lcd.backlight(); // zalaczenie podwietlenia 
  lcd.setCursor(0,0); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna)
  lcd.print(dt.day);    lcd.print("-");
  lcd.print(dt.month);
  lcd.print(" ");  lcd.print(dt.hour);   
  lcd.print(":");  lcd.print(dt.minute); 
  lcd.print(":");  lcd.print(dt.second);
  lcd.setCursor(0,1); //Ustawienie kursora w pozycji 0,0 (drugi wiersz, pierwsza kolumna)

  if (dt.hour >= lampOn && dt.hour < lampOff)
  {
    lcd.print((char)1);
  }
  if (dt.hour >= lampOff || dt.hour < lampOn)
  {
    lcd.print((char)0);
  }
  lcd.print(" ");
  if (statusHalogen==1)
  {lcd.print("H:"); lcd.print((char)3);}
  else
  {{lcd.print("H:"); lcd.print((char)2);}}
  lcd.print(" ");
  if (statusLed==1)
  {lcd.print("L:"); lcd.print((char)3);}
  else
  {{lcd.print("L:"); lcd.print((char)2);}}
  
}

void keyboard()
{
  buttonKeyboardValue = analogRead(0);
  Serial.print(buttonKeyboardValue);
  if ( buttonKeyboardValue > 955 && buttonKeyboardValue < 1010 )
  {
    //halogen
    if (statusHalogen == 1)
    {
      statusHalogen = 0;
    } else
    {
      statusHalogen = 1;
    }
    Serial.print("halogen");
  } else if ( buttonKeyboardValue > 475 && buttonKeyboardValue < 520 )
  {
    //led
    if (statusLed == 1)
    {
      statusLed = 0;
    } else
    {
      statusLed = 1;
    }
    Serial.print("led");
  } else if ( buttonKeyboardValue > 315 && buttonKeyboardValue < 350 )
  {
    //kabel
    if (statusLed == 1)
    {
      //statusLed = 0;
    } else
    {
      //statusLed = 1;
    }
    Serial.print("kabel");
  } else if ( buttonKeyboardValue > 246 && buttonKeyboardValue < 266 )
  {
    //fan
    if (statusLed == 1)
    {
      // statusLed = 0;
    } else
    {
      //statusLed = 1;
    }
    Serial.print("fan");
  }
}

void sensor()
{


 if (sensors.available())
  {
    float temperature = sensors.readTemperature(address);

    Serial.print("wyspa ciepla: ");
    Serial.print(temperature);
    Serial.println(F(" 'C"));

    sensors.request(address);
  }
  // Odczyt temperatury i wilgotności powietrza
  float t = dht.readTemperature();
  float h = dht.readHumidity();
 
  // Sprawdzamy czy są odczytane wartości
  if (isnan(t) || isnan(h))
  {
    // Jeśli nie, wyświetlamy informację o błędzie
    Serial.println("Blad odczytu danych z czujnika");
  } else
  {
    // Jeśli tak, wyświetlamy wyniki pomiaru
    Serial.print("Wilgotnosc: ");
    Serial.print(h);
    Serial.print(" % ");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" *C");
  }
 
}

/*void wyswietl(byte i)
  {
  lcd.print(":");
  if(i < 10)
  lcd.print('0');
  lcd.print(i,DEC);
  }

  void zegar()
  {
  lcd.clear();          // czyszczenie LCD
  lcd.begin(16,2);       // parametry wyświetlacza
  lcd.setCursor(3,0);          // ustawienie kursora na wyświetlaczu LCD

  if(dt.day <10)
  lcd.print('1');
  lcd.print(dt.day,DEC);
  lcd.print("/");

  if(dt.month <10)
  lcd.print('0');
  lcd.print(dt.month,DEC);
  lcd.print("/");
  lcd.print(dt.year,DEC);

  if(dt.hour <10)
  lcd.setCursor(5,1);   // ustawienie kursora na wyświetlaczu LCD
  lcd.setCursor(4,1);   // ustawienie kursora na wyświetlaczu LCD
  lcd.print(dt.hour,DEC);
  wyswietl(dt.minute);   // wywołanie funkcji wyświetl
  wyswietl(dt.second);   // wywołanie funkcji wyświetl
  }*/
