
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "secrets.h"


IPAddress target_ip(192, 168, 2, 64); // IPv4 address of desktop computer running the alarm application
unsigned int target_port = 64000; 
char alarm_message[] = "alarm received\r\n";
unsigned long last_send_time = 0;
const unsigned long min_time_between_ms = 100; 

WiFiUDP Udp;

unsigned long last_alarm_received_time = 0;
const unsigned long alarm_on_time_ms = 5000; // number of ms the alarm led should stay on after received an alarm

bool alarm_received = false;

const int activate_button_pin = 4; // use GPIO 4 == D2
const int relay_pin = 5; //use GPIO 5 == D1
const int disable_button_pin = 14; // use GPIO 14 = D5

void setup() {   
  pinMode(relay_pin, OUTPUT);
  pinMode(activate_button_pin, INPUT); 
  pinMode(disable_button_pin, INPUT);
  

  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    if (digitalRead(disable_button_pin)){
      digitalWrite(relay_pin, LOW);
      alarm_received = false;
    }
    else if (digitalRead(activate_button_pin)){
      // switch the relay
      digitalWrite(relay_pin, HIGH);
      // turn on timer for alarm reset
      last_alarm_received_time = millis();
      alarm_received = true;
    }
  
    delay(50);
  }
  // if alarm received before boot, immediately send udp
  if (alarm_received){
    last_send_time = millis();
    Udp.beginPacket(target_ip, target_port);
    Udp.write(alarm_message);
    Udp.endPacket();
  }
}

// loop function runs over and over  again forever
void loop() {
  unsigned long current_millis = millis();

  if (alarm_received){
    if (current_millis - last_alarm_received_time >= alarm_on_time_ms) {
      last_alarm_received_time = current_millis;
      digitalWrite(relay_pin, LOW);    // turn the LED off by making the  pin 13 LOW
      alarm_received = false;
    }
  }
  

  if (digitalRead(disable_button_pin)){
    digitalWrite(relay_pin, LOW);
    alarm_received = false;
  } else if (digitalRead(activate_button_pin)){
    // switch the relay
    digitalWrite(relay_pin, HIGH);
    // turn on timer for alarm reset
    last_alarm_received_time = current_millis;
    alarm_received = true;

    if (current_millis - last_send_time >= min_time_between_ms){
      last_send_time = current_millis;
      Udp.beginPacket(target_ip, target_port);
      Udp.write(alarm_message);
      Udp.endPacket();
    }
  }
}