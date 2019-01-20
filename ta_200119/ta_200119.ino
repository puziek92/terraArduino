
//+CIPSTAMAC:"5c:cf:7f:41:37:7d"


#include <Adafruit_Sensor.h> 
#include <DHT.h>
#include <SoftwareSerial.h>
#define DEBUG true
#define DHTPIN 8
#define DHTTYPE DHT11
SoftwareSerial ESPserial(2, 3); // RX | TX

DHT dht(DHTPIN, DHTTYPE);

void ESP_Init();
float higro = 0, temp = 0;
void setup() 
{
    Serial.begin(9600);     // communication with the host computer
 
    // Uruchom oprogramowanie seryjne do komunikacji z ESP8266
    ESPserial.begin(9600);
      dht.begin();
 ESP_Init();
}
 
void loop() 
{
   ESP_Send();
}

void ESP_Init()
{
  sendData("AT+RST\r\n", 1000, DEBUG); //ESP reset
  delay(1000);
  sendData("AT+CWMODE_CUR=3\r\n", 1000, DEBUG); // AP mode select
  delay(1000);
  sendData("AT+CWJAP_CUR=\"UPC249092735\",\"TFKK9EGH\"\r\n", 1000, DEBUG); //wifi connect
  while(1)
  {
    if(ESPserial.available() && ESPserial.find("OK"))
    {
      Serial.print("CONNECTED\n\r");
      break;
    }
  }
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);
  delay(100);
}

void sensor(float &higro, float &temp){
    // Odczyt temperatury i wilgotności powietrza
  temp = dht.readTemperature();
   higro = dht.readHumidity();
  // Sprawdzamy czy są odczytane wartości
  if (isnan(temp) || isnan(higro))
  {
    // Jeśli nie, wyświetlamy informację o błędzie
    Serial.println("Blad odczytu danych z czujnika");
    temp = 0;
    higro = 0;
  }
}

String sendData(String command,const int waitForResponse, boolean debug)
{
  String response = "";
  ESPserial.print(command);
  long int time = millis();
  while((time + waitForResponse) > millis())
  {
    while(ESPserial.available())
    {
      char znak = ESPserial.read();
      response += znak;
    }
  }
  if(debug)
  {
    Serial.print(response);
  }
  return response;
}

void ESP_Send(){
   ESPserial.write("AT+CIPSTART=0,\"TCP\",\"89.161.255.210\",80\r\n"); //dedicated ip address
  delay(100);
  sensor(higro, temp);
 String html = "GET /terrasensors/add.php?";
 html += "a=";
 html += temp;
 html += "&";
 html += "b=";
 html += higro;
 html += "\r\n";

  
    String sendCommand = "AT+CIPSEND=";
    sendCommand += "0";
    sendCommand += ",";
    sendCommand += html.length();
    sendCommand += "\r\n";
    sendData(sendCommand, 1000, DEBUG);
    sendData(html, 1000, DEBUG);
    delay(1000);
    String closeCommand = "AT+CIPCLOSE=";
    closeCommand += "0";
    closeCommand += "\r\n";
    sendData(closeCommand, 3000, DEBUG);
    delay(3600);
}
