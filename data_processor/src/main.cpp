/**
 * @file main.cpp
 * @brief example data processor using Wasm. This code lets the MCU to process received data. 
*/

#include "namedMesh.h"
#include "IPAddress.h"
#include <Arduino.h>
#include <esp_wifi.h>
#include "WiFi.h"
#include <SPI.h>

//wasm3
#include "wasm3.h"
#include "m3_env.h"
#include "wasm3_defs.h"

#define   MESH_PREFIX     "WasmWifiMesh"
#define   MESH_PASSWORD   "WasmWasi"
#define   MESH_PORT       5555

#define WASM_STACK_SLOTS    4000
#define FATAL(func, msg) { Serial.print("Fatal: " func " "); Serial.println(msg); return; }

IM3Environment env;
IM3Runtime runtime;
IM3Module module;
IM3Function calcWasm;
IM3Function getDataReadyFlag;
double wasmResult = 0;
int dataReadyFlag = 0;//If dataReadyFlag=1, data is ready to send.

IPAddress getlocalIP();
IPAddress espIP(0,0,0,0);

Scheduler userScheduler; // to control your personal task
namedMesh  mesh;

String nodeName = "processorNode";
String destNode = "generatorNode"; //message destination, TOOO: SET root node ID!!
String msg = "";

/**
 * @fn 
 * Get WiFi channel
 * @param ssid SSID of the router (const char*)
 */
int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

/**
 * @fn 
 * WASM setup using wasm3
 */

static void run_wasm(void*)
{
  // load wasm from SPIFFS
  /* If default CONFIG_ARDUINO_LOOP_STACK_SIZE 8192 < wasmFile,
  a new size must be given in  \Users\<user>\.platformio\packages\framework-arduinoespressif32\tools\sdk\include\config\sdkconfig.h
  https://community.platformio.org/t/esp32-stack-configuration-reloaded/20994/2
  */
  File wasmFile = SPIFFS.open("/main.wasm", "r");
  unsigned int build_main_wasm_len = wasmFile.size();
  Serial.println("wasm_length:");
  Serial.println(build_main_wasm_len);
  // read file
  unsigned char build_main_wasm[build_main_wasm_len];
  wasmFile.readBytes((char *) build_main_wasm, build_main_wasm_len);

  Serial.println("Loading WebAssembly was successful");

  M3Result result = m3Err_none;

  //it warks also without using variable
  //uint8_t* wasm = (uint8_t*)build_app_wasm;

  env = m3_NewEnvironment ();
  if (!env) FATAL("NewEnvironment", "failed");

  runtime = m3_NewRuntime (env, WASM_STACK_SLOTS, NULL);
  if (!runtime) FATAL("m3_NewRuntime", "failed");

  #ifdef WASM_MEMORY_LIMIT
    runtime->memoryLimit = WASM_MEMORY_LIMIT;
  #endif

   result = m3_ParseModule (env, &module, build_main_wasm, build_main_wasm_len);
   if (result) FATAL("m3_ParseModule", result);

   result = m3_LoadModule (runtime, module);
   if (result) FATAL("m3_LoadModule", result);

   // link
   //result = LinkArduino (runtime);
   //if (result) FATAL("LinkArduino", result);


   result = m3_FindFunction (&calcWasm, runtime, "calcWasm");
   if (result) FATAL("m3_FindFunction(calcWasm)", result);

   result = m3_FindFunction (&getDataReadyFlag, runtime, "getDataReadyFlag");
   if (result) FATAL("m3_FindFunction(getDataReadyFlag)", result);

   Serial.println("Running WebAssembly...");

}

  /**
 * @fn 
 * Call WASM task
 */

  void wasm_task(byte* inputBytes, int len){
    const void *i_argptrs[len];
    M3Result result = m3Err_none;
    
    for(int i=0; i<len ;i++){
      i_argptrs[i] = &inputBytes[i];
    }

    Serial.print("Input length: ");
    Serial.println(len);

    /*
    m3_Call(function, number_of_arguments, arguments_array)
    To get return, one have to call a function with m3_Call first, then call m3_GetResultsV(function, adress).
    (Apparently) m3_Call stores the result in the liner memory, then m3_GetResultsV accesses the address.
    */
    result = m3_Call(calcWasm,len,i_argptrs);                       
    if(result){
      FATAL("m3_Call(calcWasm):", result);
    }

    result = m3_GetResultsV(calcWasm, &wasmResult);
      if(result){
      FATAL("m3_GetResultsV(calcWasm):", result);
    }

    result = m3_Call(getDataReadyFlag,0,NULL);                       
    if(result){
      FATAL("m3_Call(isDataReady):", result);
    }

    result = m3_GetResultsV(getDataReadyFlag, &dataReadyFlag);
      if(result){
      FATAL("m3_GetResultsV(isDataReady):", result);
    }

  }

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  int msgInt = msg.toInt();
  byte* buffer = (byte*)malloc(sizeof(msgInt));
  memcpy(buffer,(int *)&msgInt,sizeof(int));
  wasm_task(buffer, sizeof(buffer));
  msg = String(wasmResult);
  Serial.println(msg);
  if(dataReadyFlag == 1){
    mesh.sendSingle(destNode, msg);
    dataReadyFlag = 0;
  }
  free(buffer);
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  //setupWifi();

  SPIFFS.begin();

  //set up for wasm
  run_wasm(NULL);


//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6);
  mesh.setName(nodeName);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();

}

