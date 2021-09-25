#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
//#define relayPin 15

int scanTime = 1; //In seconds
BLEScan* pBLEScan;
String pegaMac;
String pegaUUID;
String pegaTexto;
String pegaTexto0;
String pegaTexto1;
int pegaRSSI;
int achou;
int estaLigado;
int iniciaBLE;

const char* ssid = "GuinaElastic";
const char* password = "guinaelasticblue";

String PARAM_INPUT_1 = "input1";
String PARAM_INPUT_2 = "input2";
String PARAM_INPUT_3 = "input3";

String inputMessage1;
String inputParam1;
String inputMessage2;
String inputParam2;
String inputMessage3;
String inputParam3;

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP32 Guina Elastic</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    UUID: <input type="text" name="input1">
    Time/seconds: <input type="text" value = "60" name="input2">
    Distance/meters: <input type="text" value="-91" name="input3">
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      pegaRSSI = (advertisedDevice.getRSSI());
      pegaMac = (advertisedDevice.getAddress().toString().c_str());

      pegaTexto = ("%s", advertisedDevice.toString().c_str());
      pegaTexto0 = (inputMessage1);
      pegaTexto1 = (pegaTexto0.substring(0,8)+pegaTexto0.substring(9,13)+pegaTexto0.substring(14,18)+pegaTexto0.substring(19,23)+pegaTexto0.substring(24,36));
      pegaUUID = (pegaTexto.substring(63,95));

//      Serial.println("UUID");
//      Serial.println(pegaUUID);
//      Serial.println("pegaTexto1");      
//      Serial.println(pegaTexto1);
      
      if (pegaUUID.equals(pegaTexto1)){
//        Serial.println("MAC:");
//        Serial.println(pegaMac);
//        Serial.println(pegaRSSI);        
        if (pegaRSSI >= inputMessage3.toInt()){
          achou = 0;
          estaLigado = 1;
          digitalWrite(2, HIGH);
          digitalWrite(15, HIGH);
          Serial.println("Entrou em HIGH");
        }
      }
    }
};

void setup() {

  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  pinMode(15, OUTPUT);

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  pinMode(2, OUTPUT);

  iniciaBLE = 0;

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputParam1 = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      inputMessage3 = request->getParam(PARAM_INPUT_3)->value();
      inputParam3 = PARAM_INPUT_3;

      iniciaBLE = 1;
      
    }
    else {
      inputMessage1 = "No message sent";
      inputParam1 = "none";
    }
//    Serial.println(inputMessage1);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam1 + ") with value: " + inputMessage1 +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);

  server.begin();

}

void loop() {
  
  if (iniciaBLE == 1) {
    resultsBLEScan();
  }
  
}

void resultsBLEScan(){
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
//  Serial.print("Devices found: ");
//  Serial.println(foundDevices.getCount());
  if ((foundDevices.getCount()==0)&&(estaLigado == 1)){
    achou = achou + 1;
    Serial.println(achou);
//    Serial.println(inputMessage2);
  }

  if (achou == inputMessage2.toInt()){
    digitalWrite(2, LOW);
    estaLigado = 0;
    digitalWrite(15, LOW);
//    Serial.println("Desligar algo");
  }
    
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(100);
}
