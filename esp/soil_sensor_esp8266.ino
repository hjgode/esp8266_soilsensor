
#define MYDEBUG
#undef MYDEBUG

//the analog port
#define analog_in  A0
//Naming analog input pin
int inputVal  = 0;        //Variable to store analog input values

//which output to use for power off (Active HIGH)
//D2 = GPIO4
//D1 = GPIO5
#define PIN_POWER D1

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
const char* ssid = "Horst1";
const char* password = "1234567890123";
const char* mqtt_server = "192.168.0.40";
WiFiClient espClient;
PubSubClient client(espClient);

//#################################################
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  
  Serial.print("] ");
  String msg;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    //display.clear();
    msg += (char)payload[i];
  }
  //do something with the received message  
  String topicS="";
  topicS+=topic;
  if(topicS.indexOf( "gethum" ) >= 0){
    Serial.println("gethum received");
    if(msg.indexOf( "1" ) >= 0){
      Serial.println("gethum requested");
      publishValue();
    }
  }
  Serial.println();

}

/*
 * check if MQTT connected, if not reconnect and publish IP and RSSI
 */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    uint32_t chipid=ESP.getChipId();
    char clientid[25];
    snprintf(clientid,25,"WiFi-SoilSensor1-%08X",chipid); //this adds the mac address to the client for a unique id
    Serial.print("Client ID: ");
    Serial.println(clientid);
    if (client.connect(clientid)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      String payload="";
      payload+=WiFi.localIP().toString();
      client.publish("soilsensor1/ip", (char*) payload.c_str(), true);

      char payloadC[8];
      //send RSSI
      snprintf(payloadC, 8, "%d", WiFi.RSSI());
      client.publish("soilsensor1/rssi", payloadC, true);
      
      // ... and resubscribe
      client.subscribe("soilsensor1/#",1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*
 * read the current light level
 */
uint16_t getSoilHumidity(){
  inputVal = analogRead (analog_in); // Analog Values 0 to 1023
  Serial.print("getSoilHumidity-Humidity: ");
  Serial.print(inputVal);
  Serial.println(" value");
  return inputVal;
}

/*
 * publish the current average
 */
bool publishValue(){
  bool bRet=false;
    uint32_t chipid=ESP.getChipId();
    char clientid[25];
    snprintf(clientid,25,"WiFi-SoilSensor1-%08X",chipid); //this adds the mac address to the client for a unique id
    if (client.connect(clientid)) {
          Serial.println("publishValue connected");
          // Once connected, publish value...
          char payloadC[8];
          snprintf(payloadC, 8, "%i", getSoilHumidity());
          bool bOK = client.publish("soilsensor1/humidity", payloadC, true);
          Serial.println("soilsensor1/humidity");
          if (bOK){
            bRet=true;
            Serial.println("publish OK");
          }
    }
    else{
      Serial.println("publishValue-MQTT client NOT connected!");
    }  
    return bRet;
}

void shutDown(){
  //power off
  Serial.println("Power set OFF");
  client.loop();
  delay(1000);
  Serial.flush();
  digitalWrite(PIN_POWER, LOW);
  //Nothing executed after the above line
}

//#################################################

void setup() {
  //Set power on
  pinMode(PIN_POWER, OUTPUT);
  digitalWrite(PIN_POWER, HIGH);
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  Serial.println("Power set ON");

  uint16_t hum=getSoilHumidity();

  Serial.println();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
//  publishValue();  
}

const unsigned long REFRESH_INTERVAL = 1 * 1000; // ms
unsigned long lastRefreshTime = 0;

int sendCount=0;
int MaxCount=2;

void loop() {
  // put your main code here, to run repeatedly:
  if( (millis() - lastRefreshTime >= REFRESH_INTERVAL) )
  {
    reconnect();
    if( publishValue() ){
      shutDown();
    }
    lastRefreshTime += REFRESH_INTERVAL;
    client.loop();
    sendCount++;
    if(sendCount>MaxCount)
    {
      shutDown();
      client.loop();
      sendCount=0; 
    }
  }
}

