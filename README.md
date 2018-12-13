# NodeMCU_MQTT_IBM_Watson

This project demonstrates how to use ESP8266 as IoT MQTT device which pushes data to IBM Watson IoT platform.

Basically, ESP8266 is used to publish data on IBM Watson. ESP8266 is set to read temperature and humidity using DHT sensor. Then it publishes data(temperature and humidity) along with the timestamp to IBM cloud.

The timestamp is fetched from internet time servers so that we need not to fix device timings every now and then. It automatically will fetch correct time from server using "timezone" and "dst".

When Wifi is not available to the device, it stores the data into a ring buffer. When WIFI is available, the data is published to cloud till the buffer gets empty. After that normal routine continues.


### Prerequisites

Install arduino libraries 

```
ESPTool
DHT
```

For ESPMQTT_WithBuffer&TimeStamp.ino, we need one more library.

```
RingBuffer
```

### Code Explaination

Please follow comments in code. They are explainatory.

### IBM Watson - IoT Platform

Create a device.
Refer :
* https://console.bluemix.net/docs/services/IoT/index.html#gettingstartedtemplate
* https://developer.ibm.com/recipes/tutorials/run-an-esp8266arduino-as-a-iot-foundation-managed-device/

You can create board and customize it with cards to display data at real time.

Refer:
* https://console.bluemix.net/docs/services/IoT/data_visualization.html



