#include <RingBuf.h>
#include <DHT.h>
#include<time.h> //for timestamp
#include <ESP8266WiFi.h>
#include <PubSubClient.h> 

DHT dht(2,11,100); // (pin = D4, type, count)
//Connections: dataIn - D4, Vcc- 3.3V 

#define DHT11_PIN 2

int timezone = 5.30 * 3600; //+5.30 for India 
int dst = 1800; //daylight saving timing

int statusWifi = WL_IDLE_STATUS;
//-------- Customise these values -----------
const char* ssid = "WiFi_ssid";
const char* password = "Password";

#define ORG "organisation_ID" // use your organisation
#define DEVICE_ID "device_id"      
#define DEVICE_TYPE "device_type" // your device type 
#define TOKEN "device_token" // your device token 

//-------- Customise the above values --------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/evt/status/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

WiFiClientSecure wifiClient;
PubSubClient client(server, 8883, wifiClient);

struct data
{
  float temperature, humidity;
  int day, month,year,hr,minutes;
};

typedef struct data data;
data instance,out;

// Create a RinBuf object designed to hold a "SizeofRingBuff" of data
int SizeofRingBuff = 20;
//RingBuf *my_buf = RingBuf_new(sizeof(struct data), SizeofRingBuff); //stores 255 entries, In this case can store 36 entries

RingBuf *my_buf;

void setup() {
  memset(&instance, 0, sizeof(struct data));// Zero them out
  memset(&out, 0, sizeof(struct data));// Zero them out
  
  Serial.begin(115200); Serial.println();
  
  //Ring buffer define
  my_buf = RingBuf_new(sizeof(struct data), SizeofRingBuff); //stores 255 entries, In this case can store 36 entries

  
  if (!my_buf)// Check if null pointer
  {
    Serial.println("Not enough memory");
    while (1);
  }
  
  WiFi.mode(WIFI_STA); //added by me
  Serial.print("Connecting to "); Serial.print(ssid);
  if (strcmp (WiFi.SSID().c_str(), ssid) != 0) 
  {
     WiFi.begin(ssid, password);
  }
  while (WiFi.status() != WL_CONNECTED) 
  {
     delay(500);
     Serial.print(".");
  }  
  Serial.println("");
  Serial.print("WiFi connected, IP address: "); Serial.println(WiFi.localIP());

  Serial.println("View the published data on Watson at: "); 
  if (ORG == "quickstart") 
  {
    Serial.println("https://quickstart.internetofthings.ibmcloud.com/#/device/" DEVICE_ID "/sensor/");
  } 
  else 
  {
    Serial.println("https://" ORG ".internetofthings.ibmcloud.com/dashboard/#/devices/browse/drilldown/" DEVICE_TYPE "/" DEVICE_ID);
  }

  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  
  Serial.println("\n Waiting for Internet to get time");

  while(!time(nullptr))
  {
     Serial.print("*");
     delay(1000);
  }
Serial.println("\nTime response....OK"); 
}
void loop() 
{
   if (!!!client.connected()) //client is considered connected if the connection has been closed but there is still unread data. 
   {
      Serial.print("Reconnecting client to "); Serial.println(server);
      while ( ! (ORG == "quickstart" ? client.connect(clientId) : client.connect(clientId, authMethod, token)))  
      {
        Serial.print("."); // when wifi is not available
        time_t now = time(nullptr);
        struct tm* p_tm = localtime(&now);  
        memset(&instance, 0, sizeof(struct data));
        instance.temperature = dht.readTemperature(0,DHT11_PIN);
        instance.humidity = dht.readHumidity(DHT11_PIN);
        instance.day =p_tm->tm_mday;
        instance.month = p_tm->tm_mon + 1;
        instance.year = p_tm->tm_year + 1900;
        instance.hr=p_tm->tm_hour;
        instance.minutes=p_tm->tm_min;
        
        if (my_buf->numElements(my_buf) == SizeofRingBuff){ // The library is FIFO not ringbuffer. So, using this, I used it as RingBuffer
          my_buf->pull(my_buf, &out);
          }

        my_buf->add(my_buf, &instance); 
        delay(60*60*1000); // Update per hour -> change this delay to control elements in buffer
      }
     Serial.println("............................Client connected....................................");
   }
  Serial.print("Buffer elements");
  Serial.println(my_buf->numElements(my_buf));
  
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  // Zero them out
  memset(&instance, 0, sizeof(struct data));
  instance.temperature = dht.readTemperature(0,DHT11_PIN);
  instance.humidity = dht.readHumidity(DHT11_PIN);
  instance.day =p_tm->tm_mday;
  instance.month = p_tm->tm_mon + 1;
  instance.year = p_tm->tm_year + 1900;
  instance.hr=p_tm->tm_hour;
  instance.minutes=p_tm->tm_min;
  my_buf->add(my_buf, &instance); 
  

  while(!(my_buf->isEmpty(my_buf))) //when wifi is available and there is data in buffer to be sent to cloud
  {
      Serial.print("Buffer elements");
      Serial.println(my_buf->numElements(my_buf));
      my_buf->pull(my_buf, &out);
      String payload = "{\"d\":{\"dev\":\"NodeMCU\",\"tmp\":";
      payload += out.temperature;
      payload += ",\"h\":"; //humidity
      payload += out.humidity;
      payload += ",\"d\":"; //day
      payload += out.day;
      payload += ",\"m\":"; //month
      payload += out.month;
      payload += ",\"yr\":";//year
      payload += out.year;
      payload += ",\"hr\":";//hr
      payload += out.hr;
      payload += ",\"min\":";//minutes
      payload += out.minutes;
      payload += "}}";
      
      Serial.print("Sending payload in ring buffer: "); Serial.println(payload);
      Serial.print("Payload length:"); Serial.println(payload.length());
      if (client.publish(topic, (char*) payload.c_str())) 
      {
        Serial.println("Publish ok");
      } 
      else 
      {
        Serial.println("Publish failed");//publish failed, either connection lost, or message too large
      }
       //Serial.println("Buffer memory check");
       Serial.println("");
       delay(1000); // Contineously sends data till buffer becomes empty
  }
  delay(60*60*1000);// 60 minutes delay

}
