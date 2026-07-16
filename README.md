# Alarm Circuit

This repository contains the circuit scematics, esp8266 code and desktop code for an automatic alarm detector.

## Circuit Diagram
The subdirectory `alarm_circuit` contains a KiCAD project for detecting an incoming alarm and turning a relay on for a short while.

## ESP8266 Code
The subdirectory `esp_alarm_udp` contains an Arduino-IDE project for the esp software that detects an incoming alarm, controls the relay and sends information about the alarm over WIFI.

## Desktop Software
The subdirectory `desktop_app` contains a CMAKE-C++ project for a Windows desktop app that can receive the alarm information from the ESP and shows a big pop up.


