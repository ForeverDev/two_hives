#include "HX711.h"
#include <Wire.h>
#include "MAX17043.h"
#include "SparkFun_Si7021_Breakout_Library.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

struct HiveSensorData {
  float t_internal;
  float h_internal;
  float weight;
};

float t_external;
float h_external;
float battery;

HiveSensorData hive_data[2];

WiFiClient client;
MySQL_Connection connection((Client *)&client);
Weather weather_sensor;
MAX17043 lipo_gauge(20);


HX711 scale_1(12, 13);
/*
HX711 scale_2(4, 0);
*/


// multiplexer 14,2
const char* TABLE_NAME_1 = "hive_01";
const char* TABLE_NAME_2 = "hive_02";
const char* SQL_COMMAND = "INSERT INTO govs_beehive_data.%s(t_internal, t_external, h_internal, h_external, weight, battery) values (%f, %f, %f, %f, %f, %f)"; 
char SQL_USER[] = "esp8266_user";
char SQL_PASSWORD[] = "h7xET4TYmeme!";
const int SQL_PORT = 3306;
IPAddress sql_server_addr(10, 0, 1, 175);

const int SI7021_PORT_H2 = 0x02;      // hive 2 internal temp/humid sensor
const int SI7021_PORT_H1 = 0x00;      // hive 1 internal temp/humid sensor 
const int HTU21D_PORT = 0x01;         // external temp/humid sensor
const int LIPO_GAUGE_PORT = 0x03;     // battery sensor

const char* WIFI_NAME = "IOT";
const char* WIFI_PASS = "17631763";

const int SCAN_RATE = 60000;

const int CALIBRATION_1 = -10400; //-10400
const int CALIBRATION_2 = -10400; // TBD

void toggle_sensor(uint8_t port) {
  Wire.beginTransmission(0x70);
  Wire.write(1 << port);
  Wire.endTransmission();
}

void connect_to_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  digitalWrite(5, HIGH);
  delay(2000);
  digitalWrite(5, LOW);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(5, HIGH);
    delay(100);
    digitalWrite(5, LOW);
    delay(100);
  }
}

void sql_send() {
  while (!connection.connect(sql_server_addr, SQL_PORT, SQL_USER, SQL_PASSWORD)) {
    delay(200);
  }
  
  MySQL_Cursor* cur = new MySQL_Cursor(&connection);
  char buf[1024];
  /*
  sprintf(buf, SQL_COMMAND, TABLE_NAME_1, hive_data[0].t_internal, t_external, hive_data[0].h_internal, h_external, hive_data[0].weight, battery);
  sprintf(buf, SQL_COMMAND, TABLE_NAME_2, hive_data[1].t_internal, t_external, hive_data[1].h_internal, h_external, hive_data[1].weight, battery);
  */
  sprintf(buf, SQL_COMMAND, TABLE_NAME_1, hive_data[0].t_internal, t_external, hive_data[0].h_internal, h_external, hive_data[0].weight, battery);
  //sprintf(buf, SQL_COMMAND, TABLE_NAME_1, 8.0, 8.0, 8.0, 8.0, hive_data[0].weight, 8.0);
  cur->execute(buf);

  sprintf(buf, SQL_COMMAND, TABLE_NAME_2, hive_data[1].t_internal, t_external, hive_data[1].h_internal, h_external, hive_data[1].weight, battery);
  //sprintf(buf, SQL_COMMAND, TABLE_NAME_2, 9.0, 9.0, 8.0, 8.0, hive_data[1].weight, 8.0);
  cur->execute(buf);
  
  delete cur;
}

void setup() {
  Wire.begin();
  pinMode(5, OUTPUT);
  connect_to_wifi();


  //scale_1.set_scale(CALIBRATION_1);
/*
  scale_2.set_scale(CALIBRATION_2);
  hive_data[1].weight = scale_2.get_units();
  */
  /*
  scale_1.set_scale();
  scale_2.set_scale();

  // get hive 1 temp/humid data
  toggle_sensor(SI7021_PORT_H1);
  hive_data[0].h_internal = weather_sensor.getRH();
  hive_data[0].t_internal = weather_sensor.getTempF();

  // get hive 2 temp/humid data
  toggle_sensor(SI7021_PORT_H2);
  hive_data[1].h_internal = weather_sensor.getRH();
  hive_data[1].t_internal = weather_sensor.getTempF();

  toggle_sensor(HTU21D_PORT);
  h_external = weather_sensor.getRH();
  t_external = weather_sensor.getTempF();

  toggle_sensor(LIPO_GAUGE_PORT);
  battery = lipo_gauge.getBatteryPercentage();

  sql_send();
  //ESP.deepSleep(240 * 1000000);
*/
}

void loop() {
  //hive_data[0].weight = scale_1.get_units();
  // get hive 1 temp/humid data

  digitalWrite(5, HIGH);
  delay(300);
  digitalWrite(5, LOW);
  delay(300);
  toggle_sensor(SI7021_PORT_H1);
  hive_data[0].h_internal = weather_sensor.getRH();
  hive_data[0].t_internal = weather_sensor.getTempF();
/*
  // get hive 2 temp/humid data
  toggle_sensor(SI7021_PORT_H2);
  hive_data[1].h_internal = weather_sensor.getRH();
  hive_data[1].t_internal = weather_sensor.getTempF();

  toggle_sensor(HTU21D_PORT);
  h_external = weather_sensor.getRH();
  t_external = weather_sensor.getTempF();

  toggle_sensor(LIPO_GAUGE_PORT);
  battery = lipo_gauge.getBatteryPercentage();
  */
  sql_send();
  
  delay(3500);
}

