/**
 * @file main.cpp
 * @brief main file of data receiver. This code lets a MCU to receive data via painless (WiFi) mesh and send data via serial communication.
 */

#include "namedMesh.h"
#include "IPAddress.h"
#include <Arduino.h>
#include <esp_wifi.h>
#include "WiFi.h"
#include <SPI.h>

#define   MESH_PREFIX     "WasmWifiMesh"
#define   MESH_PASSWORD   "WasmWasi"
#define   MESH_PORT       5555


IPAddress getlocalIP();
IPAddress espIP(0,0,0,0);

Scheduler userScheduler; // to control your personal task
namedMesh  mesh;

String nodeName = "receiverNode";


// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.println(msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    //Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  //Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    //Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  //SPIFFS.begin();

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  //mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6);
  mesh.setName(nodeName);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
  
}