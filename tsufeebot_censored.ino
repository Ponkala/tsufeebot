/*//////////////////////////////////////////
 * 
 * This is THE tsufeebot
 * Just set all the network stuff well
 * and you're set. It gets data from
 * the website that shows live coffee
 * status in our guildrooms coffee maker.
 * If someone starts brewing coffee it sends
 * a telegram message through a telegram bot
 * to the chat indetified by chat_id. 
 *
 //////////////////////////////////////////*/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#ifndef STASSID
#define STASSID "XXXXXXXXXXXXXXX" //your wifi id
#define STAPSK  "YYYYYYYYYYY" //your wifi password
#endif

String BOTtoken = "XXXXXXXXX:YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY"; //your bot token
WiFiClientSecure client;


bool flipflop = true;

String c_chat_id; //changing chat_id
short int led1 = 5; //led pins
short int led2 = 4;
const char* ssid = STASSID;
const char* password = STAPSK;
const char* chat_id = "XXXXXXXXX"; //chat_id where the messages will be sent

const char* host = "coffee.eero.tech"; //site where the coffee machine data is
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char *fingerprint = "F9 9C 0A 42 09 02 37 42 C8 A8 79 27 A6 7C 87 F9 3B 6D 3E 40"; //coffee.eero.tech fingerprint
const char *fingerprintTG = "BB DC 45 2A 07 E3 4A 71 33 40 32 DA BE 81 F7 72 6F 4A 2B 6B"; //telegram fingerprint

void setup() {
  //setup pins and wifi connection to ssid
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(led2, OUTPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  connect_sequence();
}

void loop() {
  
  //if wifi disconnects it will try to reconnect to the old network
  while(WiFi.status()!=WL_CONNECTED){
    Serial.println("Connection error, retrying connection");
    connect_sequence();
  }
    
  Serial.print("connecting to ");
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  //gets data from host
  String url = "/api/";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  String cups = line.substring(9, 13);

  //connects to telegram
  Serial.println(cups);
  Serial.printf("Using telegram fingerprint '%s'\n", fingerprintTG);
  client.setFingerprint(fingerprintTG);
  if (line.endsWith("\"brewing\":\"0\"}")) {
    Serial.println("no one is brewing coffee");
    flipflop = true;
    //connect to telegram
    if (client.connect("api.telegram.org", 443)) { 
      Serial.println(".... connected to Telegram");
      
      client.println("GET /bot"+ BOTtoken + "/getUpdates?offset=-1&limit=1" + " HTTP/1.1\r\n" +
                     "Host: api.telegram.org\r\n" + "Connection: close\r\n\r\n");
     
    }
    //debug led to know it works
    vilkku(LED_BUILTIN); //blinks the led
  
  //if there is gonna be tsufee
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("To the guildroom!");
    if(flipflop == true){
      client.connect("api.telegram.org", 443);
      //sends message to chat_id
      client.println("GET /bot" + BOTtoken + "/sendMessage?chat_id=" + chat_id + "&text=tsufee%20on%20tulilla"+ " HTTP/1.1\r\n" +
                     "Host: api.telegram.org\r\n" + "Connection: close\r\n\r\n");
      flipflop = false; //so that it doesn't spam me with messages
    }
    
    
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  
  delay(3000);
}

void vilkku(int led){
  for(short int i = 0; i<10; i++){
    digitalWrite(led, !digitalRead(led));
    delay(100);
  }
}

void connect_sequence(){ //connects to set wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

