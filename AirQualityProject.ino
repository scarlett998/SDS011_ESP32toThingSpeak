/*
 * testing my ESP32-board for serial connections
 */
#include "WiFi.h"
#include "SDS011.h"
#include <ThingSpeak.h>
#define debugSerial Serial
#define DEBUG
#ifdef DEBUG
#define    DEBUG_PRINT( ... )   { debugSerial.print(__VA_ARGS__); }
#define    DEBUG_PRINTLN( ... ) { debugSerial.println(__VA_ARGS__); }
#else
#define    DEBUG_PRINT(...)
#define    DEBUG_PRINTLN(...)
#endif
HardwareSerial sdsSerial(2);
SDS011 sdsSensor;               // fine dust sensor
#define SDS_PIN_TX 22
#define SDS_PIN_RX 23


//wificonnection 

 char* ssid = "test";
const char* password = "12345678";

// ThingSpeak settings
unsigned long channelID = 1013888;
char server[] = "api.thingspeak.com";
char* writeAPIKey = "TU62UG1A85R8V0W6";
unsigned int dataFieldOne = 1;    
// Field to write temperature data
// Constants
const unsigned long postingInterval = 15L * 1000L;

// Global variables
unsigned long lastConnectionTime = 0;
int measurementNumber = 0;
WiFiClient client;

// the results of the sensor SDS011
float pm25;             
float pm10;
 float bpm;
void setup() {
  Serial.begin(115200);
  sdsSerial.begin(9600, SERIAL_8N1, SDS_PIN_RX, SDS_PIN_TX);
  sdsSensor.begin (&sdsSerial);
  connectWiFi();
}


void loop() {
  //first make sure there is wifi conneciton
  // In each loop, make sure there is an Internet connection.
    if (WiFi.status() != WL_CONNECTED) { 
        connectWiFi();
    }
char buffer[10+1];
//char bpm[10+1];

DEBUG_PRINTLN("in loop");
        int sdsErrorCode = sdsSensor.read(&pm25, &pm10);
        if (!sdsErrorCode)
        {
            DEBUG_PRINT("PM2.5: ");
            dtostrf( pm25, 6, 2, buffer);
            
            DEBUG_PRINTLN( buffer );
            // bpm=buffer;
            float bpm= strtod(buffer,NULL);
           //  httpRequest(buffer, measurementNumber);

writeTSData(channelID , dataFieldOne ,bpm);
             
            DEBUG_PRINT("PM10:  ");
            dtostrf( pm10, 6, 2, buffer);
            DEBUG_PRINTLN( buffer );
           
             DEBUG_PRINTLN("upload once");
             measurementNumber++;
        }
        else
        {
            pm25 = pm10 = 0;
            DEBUG_PRINT("SDS Error Code: ");
            DEBUG_PRINTLN(sdsErrorCode);
        }
        delay (10000);
        return;
}
void connectWiFi(){
  
WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
      
        Serial.println("Connecting");
          WiFi.begin(ssid, password);
        delay(300);
    }

    // Display a notification that the connection is successful. 
    Serial.println("Connected");
        ThingSpeak.begin( client );
  
}
void httpRequest(char* field1Data, int field2Data) {

    WiFiClient client;
    
    if (!client.connect(server, 80)){
      
        Serial.println("Connection failed");
        lastConnectionTime = millis();
        client.stop();
        return;     
    }
    
    else{
        
        // Create data string to send to ThingSpeak.
        String data = "field1=" + String(field1Data) + "&field2=" + String(field2Data); //shows how to include additional field data in http post
        
        // POST data to ThingSpeak.
        if (client.connect(server, 80)) {
          
            client.println("POST /update HTTP/1.1");
            client.println("Host: api.thingspeak.com");
            client.println("Connection: close");
            client.println("User-Agent: ESP32WiFi/1.1");
           // client.println("X-THINGSPEAKAPIKEY: "+writeAPIKey);
            client.println("Content-Type: application/x-www-form-urlencoded");
            client.print("Content-Length: ");
            client.print(data.length());
            client.print("\n\n");
            client.print(data);
            
            Serial.println("PM2.5 = " + String(field1Data));
            lastConnectionTime = millis();   
        }
    }
    client.stop();
}
int writeTSData( long TSChannel, unsigned int TSField, float data ){
  int  writeSuccess = ThingSpeak.writeField( TSChannel, TSField, data, writeAPIKey ); // Write the data to the channel
  if ( writeSuccess ){
    
    Serial.println( String(data) + " written to Thingspeak." );
    }
    
    return writeSuccess;
}
// Use this function if you want to write multiple fields simultaneously.
int write2TSData( long TSChannel, unsigned int TSField1, float field1Data, unsigned int TSField2, long field2Data, unsigned int TSField3, long field3Data ){

  ThingSpeak.setField( TSField1, field1Data );
  ThingSpeak.setField( TSField2, field2Data );
  ThingSpeak.setField( TSField3, field3Data );
   
  int writeSuccess = ThingSpeak.writeFields( TSChannel, writeAPIKey );
  return writeSuccess;
}
