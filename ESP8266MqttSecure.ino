#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> 

DHT dht(2,11,100); // (pin = D4, type, count)
//Connections: dataIn - D4, Vcc- 3.3V 

#define DHT11_PIN 2

//-------- Customise these values -----------
const char* ssid = "wired";
const char* password = "11223344";

#define ORG "owt6hh" // "quickstart" or use your organisation
#define DEVICE_ID "ESP8266"      
#define DEVICE_TYPE "ESP8266WiFiModule" // your device type or not used for "quickstart"
#define TOKEN "poon@m06" // your device token or not used for "quickstart"
//-------- Customise the above values --------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/evt/status/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

WiFiClientSecure wifiClient;
PubSubClient client(server, 8883, wifiClient);

void setup() {
  Serial.begin(115200); Serial.println();
  WiFi.mode(WIFI_STA); //added by me
  Serial.print("Connecting to "); Serial.print(ssid);
  if (strcmp (WiFi.SSID().c_str(), ssid) != 0) {
     WiFi.begin(ssid, password);
  }
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }  
  Serial.println("");
  Serial.print("WiFi connected, IP address: "); Serial.println(WiFi.localIP());

  Serial.println("View the published data on Watson at: "); 
  if (ORG == "quickstart") {
    Serial.println("https://quickstart.internetofthings.ibmcloud.com/#/device/" DEVICE_ID "/sensor/");
  } else {
    Serial.println("https://" ORG ".internetofthings.ibmcloud.com/dashboard/#/devices/browse/drilldown/" DEVICE_TYPE "/" DEVICE_ID);
  }
}

void loop() {
   if (!!!client.connected()) {
      Serial.print("Reconnecting client to "); Serial.println(server);
      while ( ! (ORG == "quickstart" ? client.connect(clientId) : client.connect(clientId, authMethod, token))) {
        Serial.print(".");
        delay(500);
     }
     Serial.println();
   }

  float temperature = dht.readTemperature(0,DHT11_PIN);
  float humidity = dht.readHumidity(DHT11_PIN);
  String payload = "{\"d\":{\"Temperature\":";
  payload += temperature;
  //payload += \"Humidity\":";
  //payload += humidity;
  payload += "}}";
  
  Serial.print("Sending payload: "); Serial.println(payload);
    
  if (client.publish(topic, (char*) payload.c_str())) {
    Serial.println("Publish ok");
  } else {
    Serial.println("Publish failed");
  }

  delay(5000);
}
