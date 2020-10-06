/*
 *  Motor-Driver für 2 Motoren (Antrieb / Richtung eigener Motor
 *  WiFi-Steuerung (STA-Mode und AP-Mode)
 *  Sensoren
 *  Helligeitssuche mit Servo und LDR
 *  
 *  11.08.2020      status: working  (jedoch nicht am Auto ausprobiert!)
 *  MCU:            ESP8266 (ESP-12E Module)
 *  Funktion:       An / Aus sowie PWM-Beschleunigungs- und -Verzögerungsfunk. 
 *                  2 Motore
 *                  WiFi Station / WiFi Access-Point
 *                  OTA-Update                  
 *                  Sensoren: US, Kontakt
 *                  Steuerung mit APP (MIT-App-Inventor)
 *                  ein Servo bewegt sich in seinem Winkelbereich
 *                  ein LDR misst die Helligkeit und sichert den 
 *                  Winkel mit dem hellsten Wert                  
 *                  
 *  Achtung: HC-SR04 muss mit 5 Volt betrieben werden; 
 *  daher Spannungsteiler erforderlich !! (470 Ohm / 1k Ohm (Verhältnis ~ 1:2)
 */  
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266WebServer.h>
#define ENA             4     // GPIO4  = Pin D2  -  Motor-Treiber A EnableA      
#define IN1             0     // GPIO0  = Pin D3  -  Motor-Treiber A IN1
#define IN2             2     // GPIO2  = Pin D4  -  Motor-Treiber A IN2
#define IN3             13    // GPIO13 = Pin D7  -  Motor-Treiber B IN3
#define IN4             15    // GPIO15 = Pin D8  -  Motor-Treiber B IN4
#define INTERR_PIN      14    // GPIO14 = Pin D5  -  externer Interrupt an GPIO4
#define US_SENSOR_TRIG_PIN   5      // GPIO5  = Pin D1  -  Trigger-Pin für den Ultrasch.-Sensor 
#define US_SENSOR_ECHO_PIN   16     // GPIO16 = Pin D0  -  Echo-Pin für den Ultrasch.-Sensor ´
#define SERVO_PIN       12    // GPIO12 = Pin D6  -  Servo-Motor´
#define LDR_PIN         A0    // Pin A0  -  LDR
                                                                            
#include <WiFiUdp.h>          // User Datagram Protocol (UDP; minimales, verbindungsloses Netzwerkprotokoll)
#include <ArduinoOTA.h>       // fuer OTA-Update
#include <Servo.h>            // Servo-Bibliothek
#include "htmlSeiten.h"

#define WIFI_MODUS      'STA' // STA = Staion, AP = Access-Point 
#define MINIMUMSPEED    510   // "Minimal-Geschindigkeit" damit der Mototr läuft
#define SERVO_MINPW     520   // default 544
#define SERVO_MAXPW     2600  // default 2400
#define AUTONOM               // fuer autonomes Fahren

// Definition von Router-Zugangsdaten (credentials)
const char* ssid1      = "IhrRouterSSID";
const char* password1  = "IhrRouterPasswort";
IPAddress lclIP (192,168,2,207);
IPAddress gateway (192,168,2,1);
IPAddress subnet (255,255,255,0);
IPAddress primaryDNS (8, 8, 8, 8);          //optional
IPAddress secondaryDNS (8, 8, 4, 4);        //optional
// Definition der AP-Zugangsdaten (credentials)
const char *ssidAP     = "ESP_AP";
const char *passwordAP = "1234abcd";

// Instanz einer Klasse ESP8266WiFiMulti mit dem Namen 'wifiMulti' erstellen
ESP8266WiFiMulti wifiMulti;            
// webserver-Objekt erstellen, der auf HTTP-Anfragen an Port 80 horcht
ESP8266WebServer webServer(80);        

int motorSpeed = 1023;                 // fuer Geschwindigkeit (1023 = Maximum)
bool motorRun = 0;                     // 1 = true = Motor an
volatile bool interruptStatus;         // 1 = true = interrupt
volatile int alteZeit = 0, debounceZeit = 500;
float entfernungUS;                    // berechnete Entfernung Ultraschalls.

Servo myServo;                         // Servo-Objekt erstellen
bool rotation = true;                  // Rotations-Richtung des Servoss
int messWert = 0;                      // Messwert
int messWertAlt = 0;

// positionieren des Servos und lesen der Lichtmesswerte 
int lesenLDR(int i) {
  int pos = 0;   
  myServo.write(i);                    // Servo positionieren
  delay(500);   
  messWert = analogRead(LDR_PIN);                   
  if (messWert > messWertAlt) {
    messWertAlt = messWert;
    pos = i;
  }                   
  return pos;
}

// suche nach grösster Lichtquelle
int findeLicht(bool rotation) {
  int i;
  int pos = 0, posX;

  messWertAlt = 0;
  if (rotation) { 
    // geht von 0 Grad bis 180 Grad in 10 Grad-Schritten   
    for (i = 0; i <= 180; i += 10) {
      posX = lesenLDR(i);              // Servo positionieren und Helligkeissensor lesen      
      if (posX) {
        pos = posX;
      }
    }            
  } else { 
    // geht von 180 Grad bis 0 Grad in 10 Grad-Schritten   
    for (i = 180; i >= 0; i -= 10) {
      posX = lesenLDR(i);              // Servo positionieren und Helligkeissensor lesen      
     if (posX) {
      pos = posX;
     }
    }         
  }            
  return pos;
}

// autonomes Fahren
void autonomFahren() {
  rotation = !rotation;                // Rotationsrichtung ändern
  int pos = findeLicht(rotation); 
  Serial.println();   
  Serial.print  ("groesste Helligkeit in Winkel: ");    
  Serial.println(pos); 

  if (pos < 61) {
    Serial.println("links");
    links();
    vorwaerts();
    delay (1000);
    motorFahrtStop ();
  } else {
    if (pos < 121) {
      Serial.println("geradeaus");
      vorwaerts();
      delay (1000);
      motorFahrtStop ();      
    } else {
      Serial.println("rechts");
      rechts();
      vorwaerts();
      delay (1000);
      motorFahrtStop ();      
    }
  }  
  delay (2000);
}


// überprüfen, ob Stoßstangenkontakt (Interrupt) vorliegt und ggf. Asuweichmanöver durchführen 
void checkInterrupt() {
  if (interruptStatus == true) { 
    Serial.println("interruptStatus = true");       
    interruptStatus = false;        
    if (motorRun) {
      ausweichenVor(0);
    }
  }  
}

// ISR-Routine 
void IRAM_ATTR isrHandler() { 
  if  ( (millis() - alteZeit) > debounceZeit) {
    interruptStatus = true;  
    alteZeit = millis();                      // letzte Zeit merken
  }   
}


// lesen der HCSR04 Messwerte und Aktion ausführen (hier Serial Monitor)
void lesenHCSR04() {
  float dauer;                               // Zeitspanne des Echos  
  digitalWrite(US_SENSOR_TRIG_PIN, LOW);     // definierten Zustand schaffen
  delayMicroseconds(5);                      // warten auf defin. PIN-Zustand
  digitalWrite(US_SENSOR_TRIG_PIN, HIGH);    // Trigger-Pin für 10 Microsk. an
  delayMicroseconds(10);
  digitalWrite(US_SENSOR_TRIG_PIN, LOW); 
  dauer = pulseIn(US_SENSOR_ECHO_PIN, HIGH); // Pulslänge in Microse. messen
  entfernungUS = dauer * 0.034 / 2;          // Entfernung berechnen
  //Serial.println("Entfernung (cm): " + String (entfernungUS));
  if (entfernungUS < 3.5) {
    Serial.println("Entfernung (cm) zu kurz: " + String (entfernungUS)); 
    ausweichenRueck(0);                     // nach rechts ausweeichen
  }
  delay(5);                                 // Selbstverwaltung 
}


//  Ausweichmanöver
// @ri : Ausweichrichtung: 0 = rechts, 1 = links
void ausweichenRueck(int ri) {
  Serial.println("ausweichenRueck");   
  rueckwaerts();
  delay (1000);                              // 1 sek. rueckwaerts fahren  
  if (ri == 0) {
    links();  
  } else {
    rechts ();
  }
  delay (500);                               // 0,5 sek. links fahren  
  motorRichtungStop ();
  vorwaerts();
}
// @ri : Ausweichrichtung: 0 = rechts, 1 = links
void ausweichenVor(int ri) {
  Serial.println("ausweichenVor");   
  vorwaerts();
  delay (1000);                              // 1 sek. rueckwaerts fahren  
  if (ri == 0) {
    links();  
  } else {
    rechts ();
  }
  delay (500);                               // 0,5 sek. links fahren  
  motorRichtungStop ();
  rueckwaerts();
}

// Motoren Fahrt und Richtung schalten
void motorenStop () {
  Serial.println("stop");  
  motorFahrtStop();
  motorRichtungStop(); 
  motorRun = false; 
}


void motorFahrtStop () {  
  digitalWrite(IN1, LOW);  
  digitalWrite(IN2, LOW);       
}  
void motorRichtungStop () {
  digitalWrite(IN4, LOW);     
}  
void vorwaerts () {
  Serial.println("vor");  
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);    
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);    
  motorRun = true;        
}
void rueckwaerts () {
  Serial.println("rueck"); 
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);  
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);    
  motorRun = true;         
}
void links () {
  Serial.println("links");  
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);     
}
void rechts () {
  Serial.println("rechts");  
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);         
}


// senden der Start HTML-Seite, bei einem URI '/' oder 
// bei einem URI 'motor' nach Auswertung der Parameter
void handleRoot() {   
  Serial.println("handleRoot");
  String htmlPage;  
  htmlPage.concat(motorHTML_Seite1);             // HTML-Seite aufbauen
  String s = (String("width: ") + String((motorSpeed*100)/1023) + String("%;"));   
  htmlPage.concat(s);  
  htmlPage.concat(motorHTML_Seite2);    
  webServer.send(200, "text/html", htmlPage);    // HTML-Seite senden 
}


// Auswertung einer HTML-Anfrage einem URI '/motor' 
void handleMotor() {                          
  Serial.println("handleMotor"); 
  controlMotors(webServer.arg("fahr"));   
  handleRoot(); 
}


// Auswertung einer HTML-Anfrage einem URI '/motorAI' 
void handleMotorAI() {                          
  Serial.println("handleMotorAI");    
  controlMotors(webServer.arg("fahr"));
  String s = String ((motorSpeed*100)/1023) ;   
  webServer.send(200, "text/plain", s); 
}


// Steuerung der Motoren
void controlMotors (String fahrAuftrag) {                          
  Serial.println("controlMotors");      
  if(webServer.arg("fahr") == "langsamer") { 
    Serial.println("langsamer");  
    motorSpeed-=20;
    if (motorSpeed < MINIMUMSPEED) {
      motorSpeed = MINIMUMSPEED; 
    } 
    analogWrite(ENA, motorSpeed);
  } 
  if(webServer.arg("fahr") == "schneller") { 
    Serial.println("schneller");      
    motorSpeed+=20;   
    if (motorSpeed > 1023) {
      motorSpeed = 1023; 
    } 
    analogWrite(ENA, motorSpeed); 
  }
  if(webServer.arg("fahr") == "stop") { 
    motorenStop ();    
  } 
  if(webServer.arg("fahr") == "vor") { 
    vorwaerts ();    
  }
  if(webServer.arg("fahr") == "rueck") {
    rueckwaerts ();    
  } 
  if(webServer.arg("fahr") == "links") { 
    links ();        
  }    
  if(webServer.arg("fahr") == "rechts") { 
    rechts ();    
  }  

}


// sendet HTTP status 404 (Not Found) 
// wenn es keinen Handler fuer den URI im HTTP-Request gibt
void handleNotFound(){
  Serial.println("handleNotFound");  
  webServer.send(404, "text/plain", "404: Not found"); 
}

void setup(void){
  Serial.begin(115200);         
  delay(10);
  // Motor-Control-Pins als Output öffnen
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);   
  pinMode(ENA, OUTPUT);

  pinMode(INTERR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERR_PIN), isrHandler, FALLING);
  // Ultraschall-Pins als Output öffnen
  pinMode(US_SENSOR_TRIG_PIN, OUTPUT);
  pinMode(US_SENSOR_ECHO_PIN, INPUT);  

  myServo.attach(SERVO_PIN);          // Servo-Steuerung an SERVO_PIN
  //myServo.attach(SERVO_PIN,SERVO_MINPW,SERVO_MAXPW);           

  analogWrite(ENA, motorSpeed);

#if (WIFI_MODUS  == 'STA')
  // WiFi-Netzwerke hinzufügen, mit denen eine Verbindung aufgebaut werden soll 
  wifiMulti.addAP(ssid1, password1); 
  wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");
  WiFi.mode(WIFI_STA);  
  if (!WiFi.config(lclIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA konnte nicht konfiguriert werden");    
  }
  
  Serial.println("WiFi Station-Modus Verbindungsaufbau ");
  int i = 0;
  // auf WiFi-Verbindung warten: d.h. scan WiFi-Netzwerke und verbinden 
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("verbunden mit ");
  Serial.println(WiFi.SSID());              
  Serial.print("IP-Adresse:\t");
  Serial.println(WiFi.localIP());    
#else 
  Serial.println("WiFi Access-Point-Modus ");
  WiFi.softAP(ssidAP, passwordAP);             
  Serial.print("Access Point \"");
  Serial.print(ssidAP);
  Serial.println("\" gestartet");
  Serial.print("IP Addresse:\t");
  Serial.println(WiFi.softAPIP());               // IP-Adresse ESP8266  
  delay (500);
#endif 

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  // Call der 'handleRoot' Funktion bei einer Client-Amforderung URI "/"
  webServer.on("/", HTTP_GET, handleRoot);     
  // Call der 'handleRoot' Funktion bei einer Client-Amforderung URI "/motor"
  webServer.on("/motor", HTTP_GET, handleMotor);    
  // Call der 'handleMotorAI' Funktion bei einer Client-Amforderung URI "/motorAI"
  webServer.on("/motorAI", HTTP_GET, handleMotorAI);  
  webServer.onNotFound(handleNotFound);          // bei einem unbekannten URI  
  webServer.begin();                             // Start des Webservers
  Serial.println("HTTP server gestartet");  
}


void loop(void){
  ArduinoOTA.handle();  
  webServer.handleClient();                      // horchen auf HTTP-Anfragen von Clients 
  checkInterrupt();  
  lesenHCSR04();     
#if defined AUTONOM  
  autonomFahren();
#endif  
  delay (5);
}
