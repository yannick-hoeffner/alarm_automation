
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "secrets.h"

//#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINTLN(...)
#endif // DEBUG

WiFiUDP Udp;

const int activate_button_pin = 4; // use GPIO 4 == D2
const int relay_pin = 5; //use GPIO 5 == D1
const int disable_button_pin = 14; // use GPIO 14 = D5

typedef enum {
    AS_IDLE       = 0, // waiting for alarm
    AS_CHECK      = 1, // checking incomming alarm
    AS_ALARM_RECV = 2, // alarm received
    AS_ALARM_KEEP = 3, // alarm cleared shortly after received
    AS_ALARM_IDLE = 4, // alarm does not dissapear after timeout
} alarm_state_t;


alarm_state_t alarm_state = AS_IDLE;
unsigned long alarm_start_time = 0;
const unsigned long alarm_check_time_ms = 150; // number of ms the pin shoul be HIGH to be considered a real alarm
const unsigned long alarm_on_time_ms = 5000; // number of ms the alarm light should stay on after an recv alarm

IPAddress target_ip(192, 168, 2, 64); // IPv4 address of desktop computer running the alarm application
unsigned int target_port = 64000; 
char alarm_message[] = "alarm received\r\n";
char clear_message[] = "alarm cleared\r\n";

void udp_send_mssg(char* mssg){
  if(WiFi.isConnected()){
    Udp.beginPacket(target_ip, target_port);
    Udp.write(mssg);
    Udp.endPacket();
  }
}

unsigned long last_send_time = 0;
const unsigned long min_time_between_ip_ms = 100; // ms between two ip messages to prevent network flooding
void udp_send_mssg_timeout(char* mssg, unsigned long current_millis){
  if(WiFi.isConnected()){
    if(current_millis - last_send_time > min_time_between_ip_ms){
      Udp.beginPacket(target_ip, target_port);
      Udp.write(mssg);
      Udp.endPacket();
      last_send_time = current_millis;
    }
  }
}

void setup() {   
  pinMode(relay_pin, OUTPUT);
  pinMode(activate_button_pin, INPUT); 
  pinMode(disable_button_pin, INPUT);
#ifdef DEBUG
  Serial.begin(9600);
#endif //DEBUG

  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
}

// loop function runs over and over  again forever
void loop() {
  unsigned long current_millis = millis();
  int alarm_pin = digitalRead(activate_button_pin);
  int disable_pin = digitalRead(disable_button_pin);

  switch(alarm_state){
    case AS_IDLE:
      //waiting for an incomming alarm
      DEBUG_PRINTLN(AS_IDLE);
      digitalWrite(relay_pin, LOW);
      if (alarm_pin && !disable_pin){
        alarm_state = AS_CHECK;
        alarm_start_time = current_millis;
      }
      break;    
    case AS_CHECK:
      //waiting whether pin HIGH indicates an incomming alarm
      DEBUG_PRINTLN(AS_CHECK);
      digitalWrite(relay_pin, LOW);
      if(!alarm_pin || disable_pin){
        alarm_state = AS_IDLE;
      }else if(current_millis - alarm_start_time > alarm_check_time_ms){
        alarm_state = AS_ALARM_RECV;
      }
      break;     
    case AS_ALARM_RECV:
      // handle real alarm
      DEBUG_PRINTLN(AS_ALARM_RECV);
      digitalWrite(relay_pin, HIGH);
      udp_send_mssg_timeout(alarm_message, current_millis);
      if(disable_pin){
        udp_send_mssg(clear_message);
        alarm_state = AS_IDLE;
      }else if(!alarm_pin){
        alarm_state = AS_ALARM_KEEP;
      }else if(current_millis - alarm_start_time > alarm_on_time_ms){
        udp_send_mssg(clear_message);
        alarm_state = AS_ALARM_IDLE;
      }
      break;
    case AS_ALARM_KEEP:
      // keep the ligth on, but do not send IP-messages
      // this state usually means, that the pager was removed from the docking station
      // thus the operator already reacts to the alarm
      DEBUG_PRINTLN(AS_ALARM_KEEP);
      digitalWrite(relay_pin, HIGH);
      if(current_millis - alarm_start_time > alarm_on_time_ms || disable_pin){
        udp_send_mssg(clear_message);
        alarm_state = AS_IDLE;
      }
      break;
    case AS_ALARM_IDLE:
      // turn everything off, but do not restart 
      // this state usually means, that the page was forgotten in the docking station
      // thus the pin stays HIGH even though the reaction time for an alarm is way over
      DEBUG_PRINTLN(AS_ALARM_IDLE);
      digitalWrite(relay_pin, LOW);
      if(!alarm_pin || disable_pin){
        alarm_state = AS_IDLE;
      }
      break;
  }
}