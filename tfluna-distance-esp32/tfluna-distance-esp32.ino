#include "WiFi.h"
#include "EspMQTTClient.h"
#include <driver/uart.h>

const char* ssid = "<SSID>";
const char* password = "<PASS>";

const bool debug = true;

EspMQTTClient client(
  ssid,
  password,
  "192.168.10.75",       // MQTT Broker server ip
  "",                   // MQTTUsername, Can be omitted if not needed
  "",                   // MQTTPassword, Can be omitted if not needed
  "esp32_septic_tank",  // Client name that uniquely identify your device
  1883                  // The MQTT port, default to 1883. this line can be omitted
);

void setup_wifi()
{
  bool just_connected = false;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");

    WiFi.begin(ssid, password);
    just_connected = true;
  }

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  if (debug) {
    Serial.println("");
    Serial.print("WiFi connected: ");
    Serial.println(WiFi.localIP());
  }

  if (just_connected) client.publish("sensors/septic/wifi/ip", WiFi.localIP().toString());
}

void setup_mqtt()
{
  if (debug) {
    client.enableDebuggingMessages();
    client.enableHTTPWebUpdater();
    client.enableLastWillMessage("sensors/septic/lastwill", "I am going offline");
  }
}

void onConnectionEstablished()
{
  client.publish("sensors/septic/mqtt/status", "ready");
}

void setup_uart_sensor()
{
  uart_set_pin(UART_NUM_0, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  pinMode(21, OUTPUT);
}

void setup()
{
  Serial.begin(115200);

  setup_uart_sensor();
  setup_wifi();
  setup_mqtt();
}

void measure() {
  int dist; //actual distance measurements of LiDAR
  int strength; //signal strength of LiDAR
  float temprature;
  int check; //save check value
  int i;
  int uart[9]; //save data measured by LiDAR
  const int HEADER = 0x59; //frame header of data package

//  Serial.println("check if serial port has data input");
  if (Serial.available()) { //check if serial port has data input
    if (Serial.read() == HEADER) { //assess data package frame header 0x59
      uart[0] = HEADER;
      if (Serial.read() == HEADER) { //assess data package frame header 0x59
        uart[1] = HEADER;
        for (i = 2; i < 9; i++) { //save data in array
          uart[i] = Serial.read();
        }
        check = uart[0] + uart[1] + uart[2] + uart[3] + uart[4] + uart[5] + uart[6] + uart[7];
        if (uart[8] == (check & 0xff)) { //verify the received data as per protocol
          dist = uart[2] + uart[3] * 256; //calculate distance value
          strength = uart[4] + uart[5] * 256; //calculate signal strength value
          temprature = uart[6] + uart[7] * 256; //calculate chip temprature
          temprature = temprature / 8 - 256;
          Serial.print("dist = ");
          Serial.print(dist); //output measure distance value of LiDAR
          Serial.print('\t');
          Serial.print("strength = ");
          Serial.print(strength); //output signal strength value
          Serial.print("\t Chip Temprature = ");
          Serial.print(temprature);
          Serial.println(" celcius degree"); //output chip temperature of Lidar
        }
      }
    }
  }
}

void loop()
{
//  Serial.println("loop");
//  setup_wifi();
  measure();
//  client.loop();
//  delay(1000);
}
