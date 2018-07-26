/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <regex>
#include <string>
#include <cstdlib>
using namespace std;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "fc7ee20c-d9f2-4dd2-ad38-9d5462f7ce08"
#define CHARACTERISTIC_UUID "0d563a58-196a-48ce-ace2-dfec78acc814"

String dataToSend=""; // to send data you only have to change this variable value
String dataRecieved="";// you can access data recieved from this variable

int RPM = 6000;
float M1_Temp = 72.5;
float Current = 98;
float Airspeed = 32;

int Pitch;
float Throttle;
int Angle;

std::vector<int> stringToInt(std::string str){// the function that takes a string and turn it into int vector
    std::vector<int> out;
    std::regex rgx(",+");
    std::sregex_token_iterator iter(str.begin(),str.end(),rgx,-1);
    std::sregex_token_iterator last{};
    std::stringstream buffer;
    // declarations could be put in a global environment to speed up the execution of the code.
    for(;iter!=last;++iter){
        buffer << NULL;
        buffer << *iter << '\n';
        string s = "10";
        istringstream converter(str);
        int converted;
        converter >> converted;
        out.push_back(converted);
    }
    return out;
}

class MyCallBacks: public BLECharacteristicCallbacks{
  
  void onWrite(BLECharacteristic *pCharacteristic){
    //HERE!!! is how you recieve data
    if(pCharacteristic->getLength()>0){
      dataRecieved = pCharacteristic->getValue().c_str();  // HOW DO I GET THE DATA OUT OF THE STRING SO I CAN STORE THEM TO INTS AND FLOAT VARIABLES?  answer in line below
      std::vector<int> out=stringToInt(pCharacteristic->getValue().c_str());// it's converted to an int vector.
      /* pCharacteristic->getValue().c_str() could be accessible everywhere in the code if you remove the declaration part 
        from the setup() function and declare it global, but it's not necessary as it's the client that decides when reads and writes occur
       // NEED TO UNDERSTAND BELOW SO I CAN ADD MORE VARIABLES TO BE SENT IN FUTURE
      Pitch = out[0];
      Throttle = out[1];
      Angle = out[2];
      */
      Pitch = out[0];
      Serial.print("Pitch : ");
      Serial.println(Pitch);
    }  // End if
  } // End onWrite
  
  void onRead(BLECharacteristic *pCharacteristic){
    //HERE!!! is how you send data
    char charBuf[dataToSend.length()+1];  /* WHAT IS DATA BEING SENT IN A CHAR FORMAT YET IN THE CLIENT SIDE IT'S IN A STRING FORMAT?
                                             it's not a char , it's a char array, which when affected to string becomes a string.
                                           */
    dataToSend.toCharArray(charBuf, dataToSend.length()+1);
    pCharacteristic->setValue(charBuf);
  } // End onRead
};  // End MyCallBacks

void setup() {
  Serial.begin(115200);

  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallBacks());
  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  
} // End of setup




void loop() {
  // put your main code here, to run repeatedly:
  dataToSend = String(RPM) + "," + String(M1_Temp) + "," + String(Current) + "," + String(Airspeed);

  //Serial.println(dataRecieved);
}
