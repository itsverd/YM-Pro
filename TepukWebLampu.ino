#include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
#include <ESPAsyncWebServer.h>
#include "HTML_LocalWeb.h"

//Setup Pin yang digunakan
#define sensorSuara D0
#define lampu D1
#define buzzer D2
#define LED1 D3
#define LED2 D4
#define sensorLDR A0
#define LEDRelay D8

bool bacaSuara; // Membaca apakah ada suara yang terdeteksi oleh sensor
int bacaLDR; // Untuk membaca nilai yang diberikan sensor LDR
short count = 0; // Untuk menyalakan atau memetikan Relay
short switchSensor = 3; // Untuk mengubah mode yang digunakan Ket: 1 = LDR, 2 = Suara, 3 = Manual
short switchSensorAwal = 3; //Untuk menyimpan mode sebelumnya Ket: 1 = LDR, 2 = Suara, 3 = Manual
const short intensitasMAX = 55; //Intensitas Maksimal untuk menyalakan relay

//Timer untuk mengecek nilai LDR
unsigned long prevMillis = 0;
unsigned long currMillis = 0;
const long interval = 30000; //Interval untuk mengirim perintah dari LDR ke relay ket: 1000 = 1sec

//Timer pembacaan sensor Suara
int statusAwalSS = 0;
long startOnSS = 0;

bool restartStatus; // Mengecek apakah ada permintaan restart dari website

// Kode Untuk AsyncWebServer 
AsyncWebServer server(80); // server port 80

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
}

void startSystem(){
  Serial.println("Welcome to YM-Pro v1 Sir..");
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LEDRelay, LOW);
  tone(buzzer, 2000);
  delay(500);
  noTone(buzzer);
  digitalWrite(LED1, HIGH);
  delay(1000);
  digitalWrite(LED2, HIGH);
  delay(1000);
  digitalWrite(LEDRelay, HIGH);
  delay(1000);
  tone(buzzer, 2000);
  delay(100);
  noTone(buzzer);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LEDRelay, LOW);
  Serial.println("");
  Serial.print("This your IP: ");
  Serial.println(WiFi.softAPIP());
}

void setup(void){
  
  Serial.begin(115200);
  pinMode(lampu,OUTPUT);
  pinMode(sensorSuara, INPUT);
  pinMode(sensorLDR, INPUT);
  pinMode(LEDRelay, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(lampu, LOW);

  WiFi.softAP("YM-Pro v1", ""); //Access point name & password, Contoh("Nama", "Password")

  startSystem(); //Animasi lampu dan teks serial monitor

// Kode untuk AsyncwebServer
  server.on("/", [](AsyncWebServerRequest * request)
  { 
  request->send_P(200, "text/html", webpage);
  });

   server.on("/kirim", HTTP_GET, [](AsyncWebServerRequest * request) 
  { 
    if(switchSensor == 1){
      switchSensor = 3;
    }
    count++;
    request->send_P(200, "text/html", webpage);
  });

  server.on("/clap", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    switchSensor = 2;
    Serial.println("Sensor Tepuk Aktif");
    request->send_P(200, "text/html", sukses);
  });

  server.on("/ldr", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    switchSensor  =1;
    Serial.println("Sensor Cahaya Aktif");
    request->send_P(200, "text/html", sukses);
  });

  server.on("/manual", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    switchSensor  = 3;
    Serial.println("Dialihkan ke kontrol manual");
    request->send_P(200, "text/html", sukses);
  });

  server.on("/rst", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
    restartStatus = 1;
    // request->send_P(200, "text/html", sukses);
    
  });

  server.onNotFound(notFound);

  server.begin();  // memulai web server
 
}


void loop(void){

// Untuk merestart sistem berdasarkan perintah dari web
  if(restartStatus){
    Serial.println("Request to Restart Device...");
    tone(buzzer, 2000);
    delay(200);
    noTone(buzzer);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LEDRelay, HIGH);
    digitalWrite(lampu, HIGH);
    delay(2000);
    Serial.println("Restart begin");
    ESP.restart();
  }


// Untuk membaca data dari sensor
    bacaSuara = digitalRead(sensorSuara); // Baca data sensor Suara


// Menentukan apakah yang dipilih merupakan sensor Suara atau sensor LDR
    switch(switchSensor){
      case 1: 
      bacaLDR = analogRead(sensorLDR);  // Baca data sensor Cahaya
      // Serial.println(bacaLDR);
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, LOW);

      currMillis=millis();

      if(currMillis - prevMillis >= interval){
        prevMillis = currMillis;
        if(bacaLDR <= intensitasMAX){
          count = 1;
        } else{
          count = 2;
        }
      }
      delay(500);
      break;
      
      case 2:
      digitalWrite(LED2, HIGH);
      digitalWrite(LED1, LOW);
    
      if(bacaSuara){
        count++;
        tone(buzzer, 2000, 500);
        delay(3500);
      } else {
          noTone(buzzer);
        }
      break;

      case 3:
      digitalWrite(LED2, LOW);
      digitalWrite(LED1, LOW);

      break;

      default:
        switchSensor = 3;
        break;
    }

    if(switchSensorAwal != switchSensor){
    tone(buzzer, 2000);
    delay(200);
    noTone(buzzer);
    switchSensorAwal = switchSensor;
  }

    // Serial.println(count);
  
// Untuk menyalakan atau mematikan lampu
  switch(count){
    case 1:
    digitalWrite(lampu, HIGH);
    digitalWrite(LEDRelay, HIGH);
    // Serial.println("Lampu Nyala");
    break;
    case 2:
    digitalWrite(lampu, LOW);
    digitalWrite(LEDRelay, LOW);
    // Serial.println("Lampu Mati");
    break;
    default:
    if(count==3){
      count=1;
    }
  }
  delay(20);
  
}