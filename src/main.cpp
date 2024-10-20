///  Ramiro Iván Ríos 2018-2024 ////
/// Este software tiene licencia abierta, pero no-comercial.
/// Para contactarse con el desarrollador, https://github.com/platon-actual
//// Créditos a https://github.com/tzapu/WiFiManager  ///
/////////////////////////////////////////////////////////
//////////////// INCLUSIONES ////////////////////////////
/////////////////////////////////////////////////////////

#include <Adafruit_BMP085_U.h>
//#include <Adafruit_BMP085.h>
#include <Wire.h>
//Adafruit_BMP085_Unified bmp;

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

// Para conectar a Wifi, el WifiManager de tzapu
#include <WiFiManager.h>

// para el NTP (network time protocol)
#include <WiFiUdp.h>
// LA PAGINA WEB QUE MUESTRA
#include "index.h"

/////////////////////////////////////////////////////////
////////////////  CONSTANTES   //////////////////////////
/////////////////////////////////////////////////////////

#define LED_1 4
#define LED_2 5
#define LED_3 14
#define LED_4 12

const long intervalo = 3000;
const long intervalo_ntp = 10000;

/////////////////////////////////////////////////////////
//////////  VARIABLES Y DECLARACIONES ///////////////////
/////////////////////////////////////////////////////////

unsigned int localPort = 2390;
IPAddress timeServerIP;
const char* ntpServerName = "0.south-america.pool.ntp.org";
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE ];

WiFiUDP udp;

const char* ssid = "Berto";
const char* password = "micabertateamo";

float temperatura;
int temp_limite;
bool minimo = LOW;
bool temp_auto = LOW;

unsigned long milis_anterior_temp = 0;
unsigned long milis_anterior_ntp = 0;
unsigned long milis_actual;

ESP8266WebServer server(80);

unsigned int hora_actual;
unsigned int minutos_actual;
const int UTC = -3; /// Argentina UTC -03:00

unsigned int hora_on;
unsigned int hora_off;
unsigned int minutos_on;
unsigned int minutos_off;

bool hora_auto = LOW;

/////////////////////////////////////////////////////////
//////////////// FUNCIONES //////////////////////////////
/////////////////////////////////////////////////////////

void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

void handleAUTOHORA(){
	String s_hora_on = server.arg("HORA_ON");
	String s_hora_off = server.arg("HORA_OFF");
	String s_minutos_on = server.arg("MINUTOS_ON");
	String s_minutos_off = server.arg("MINUTOS_OFF");
	String s_hora_auto = server.arg("CHECK_HORARIO");
	
	hora_on = s_hora_on.toInt();
	hora_off = s_hora_off.toInt();
	minutos_on = s_minutos_on.toInt();
	minutos_off = s_minutos_off.toInt();
	
	if (s_hora_auto == "true"){
		hora_auto = HIGH;
   Serial.println("activar auto hora");
	}
	if (s_hora_auto == "false"){
		hora_auto = LOW;
    Serial.println("desactivar auto hora");
	}
}

void handleAUTOTEMP(){
  String s_limite = server.arg("LIMITE");
  String s_temperatura = server.arg("TEMPERATURA");
  String s_auto = server.arg("CHECK_AUTO");
  

  temp_limite = s_temperatura.toInt();
  if( s_limite =="1")
    minimo = HIGH;
  if( s_limite =="0")
    minimo = LOW;

  if( s_auto =="true")
    temp_auto = HIGH;
  if( s_auto =="false")
    temp_auto = LOW;
  
  String retorno ("<BR>limite es " + s_limite + "<BR>temperatura es " + s_temperatura + "<BR>auto on = " + s_auto);

  server.send(200, "text/html", retorno);
}

void handleTEMP() {
 
 String s_temperatura = String(temperatura);
 
 server.send(200, "text/plane", s_temperatura); //Send ADC value only to client ajax request
}

void handleLED() {
 String ledState = "OFF";
 String t_state = server.arg("STATE");
 String t_output = server.arg("OUTPUT");
  int output;
  switch(t_output.toInt()){
    case 1:
      output = LED_1; break;
    case 2:
      output = LED_2; break;
    case 3:
      output = LED_3; break;
    case 4:
      output = LED_4; break;
  }
 
 Serial.println( "LED " + String(output) + " estado: " + t_state);
 
 if(t_state == "1")
 {
  digitalWrite(output,HIGH);
  ledState = "ON"; //Feedback parameter
 }
 else
 {
  digitalWrite(output,LOW);
  ledState = "OFF"; //Feedback parameter  
 }
 
 server.send(200, "text/plane", ledState); //Send web page
}

void handleGET(){
  int digital_1 = digitalRead(LED_1);
  int digital_2 = digitalRead(LED_2);
  int digital_3 = digitalRead(LED_3);
  int digital_4 = digitalRead(LED_4);
  server.send(200, "text/plane", String(digital_1) + ";" + String(digital_2) + ";" + String(digital_3) + ";" + String(digital_4) );
}


void updateControls(){
	String controles = "";
	controles += String(hora_auto) + ";";
	controles += String(hora_on) + ";";
	controles += String(minutos_on) + ";";
	controles += String(hora_off) + ";";
	controles += String(minutos_off) + ";";
	controles += String(minimo) + ";";
	controles += String(temp_limite) + ";";
	controles += String(temp_auto);
	server.send(200, "text/plane", controles);
}

void sendHora(){
	String hora_completa = String(hora_actual) + ":";
	if (minutos_actual < 10){
		hora_completa += "0" + String(minutos_actual);
	}else{
		hora_completa += String(minutos_actual);
	}
	server.send(200, "text/plane", hora_completa);
}

void AutoHorario(){
    if (hora_auto == HIGH){ 
      bool its_on = digitalRead(LED_3);
      
    	if ((hora_actual == hora_off) && (its_on == LOW)){
    		if (minutos_actual == minutos_off){
    			digitalWrite(LED_3, HIGH);
          Serial.println("HORA---------APAGA");
    		}
    	}
    	if ((hora_actual == hora_on) && (its_on == HIGH)){
    		if (minutos_actual == minutos_on){
    			digitalWrite(LED_3, LOW);
         Serial.println("HORA----------ENCIENDE");
    		}
    	}
    }
}

void sendNTPpacket(IPAddress& address){
  Serial.println("enviando paquete NTP....");
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  udp.beginPacket(address, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void ActualizarHorarioNTP(){
  if( milis_actual - milis_anterior_ntp >= intervalo_ntp){
    milis_anterior_ntp = millis();
    sendNTPpacket(timeServerIP);
      int cb = udp.parsePacket();
      if (!cb){
        Serial.println("no hay paquete horario aun");
      } else {
        Serial.print("paquete recibido, tamanio = ");
        Serial.println(cb);
        udp.read(packetBuffer, NTP_PACKET_SIZE);
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        
        Serial.print("Segundos desde 1-enero-1900 = ");
        Serial.println(secsSince1900);

        Serial.print("tiempo Unix =");
        const unsigned long seventyYears = 2208988800UL;
        unsigned long epoch = secsSince1900 - seventyYears;
        Serial.println(epoch);
		
		//guardar en las variables de horas y minutos
		hora_actual = (epoch %86400L) / 3600;
		minutos_actual = (epoch % 3600) / 60;
   if ((hora_actual >= 0) && (hora_actual <= 3)) hora_actual = 24 + hora_actual;
   Serial.print ("    --  variable hora: ");
   Serial.println(hora_actual); 
		hora_actual = hora_actual + UTC;
		
		Serial.print("horario guardado:");
		Serial.print(hora_actual);
		Serial.print(":");
		Serial.println(minutos_actual);
		
        Serial.print(" tiempo UTC BsAs es ");
        // la hora
        Serial.print((epoch % 86400L) / 3600); 
        Serial.print(':');
        // minutos
        if (((epoch % 3600) /60) < 10){
          Serial.print('0');
        }
        Serial.print((epoch % 3600) / 60);
        Serial.print(':');
        // segundos
        if ((epoch % 60) < 10){
          Serial.print('0');
        }
        Serial.println(epoch % 60);
      }
  }
}

void AutoTemperatura(){
	if (temp_auto == HIGH){
    if( milis_actual - milis_anterior_temp >= intervalo){
      milis_anterior_temp = millis();
      if (minimo == LOW){
          if (temperatura > temp_limite){
            digitalWrite(LED_4,HIGH);
          }else{
            digitalWrite(LED_4,LOW);
          }
      }
      if (minimo == HIGH){
          if (temperatura < temp_limite){
            digitalWrite(LED_4,HIGH);
          }else{
            digitalWrite(LED_4,LOW);
          }
      }
    }
  }
}

void setup(void){
  Serial.begin(9600);
  Wire.pins(0,2);
  Wire.begin(0,2);
  //bmp.begin();
  
  //WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
  WiFiManager wifimanager;
  wifimanager.resetSettings();

  wifimanager.autoConnect("Vagotrónico Wifi");

  //Onboard LED port Direction output
  pinMode(LED_1,OUTPUT);
  pinMode(LED_2,OUTPUT);
  pinMode(LED_3,OUTPUT);
  pinMode(LED_4,OUTPUT);
  
  // Wait for connection
  /*while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }*/

  //If connection successful show IP address in serial monitor
  // Serial.println("");
  // Serial.print("Connected to ");
  // Serial.println(ssid);
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  // Serial.println("Starting UDP");
  // udp.begin(localPort);
  // Serial.print("Local port: ");
  // Serial.println(udp.localPort());
  
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/setState", handleLED);
  server.on("/readTEMP", handleTEMP);
  
  server.on("/getOutputs", handleGET);
  server.on("/getHora", sendHora);
  server.on("/getControls", updateControls);
  
  server.on("/setAutoHorario", handleAUTOHORA);
  server.on("/setAutoTemp", handleAUTOTEMP);

  server.begin();                  //Start server
  Serial.println("HTTP server started");
}


void loop(void){
  milis_actual = millis();

  WiFi.hostByName(ntpServerName, timeServerIP);


  //temperatura = bmp.getTemperature(&temperatura);
  //bmp.getTemperature(&temperatura);
  server.handleClient();          //Handle client requests
	
	ActualizarHorarioNTP();
  AutoHorario();
  AutoTemperatura();
  
}
