/**
 * A BLE client example that is rich in capabilities.
 */

#include "BLEDevice.h"
#include <iostream>
#include <regex>
#include <string>
#include <cstdlib>
using namespace std;
#define MAC_ADRESSE "45"        //MUST define mac adress of the server  //******* THIS ISN'T IN SERVER CODE, IS IT NECCESSARY?
#define DEVICE_NAME "MyESP32"  //OR the servers name  // DOES THIS NEED TO MATCH ON BOTH ARDUINOS?  
// The remote service we wish to connect to.
static BLEUUID serviceUUID("fc7ee20c-d9f2-4dd2-ad38-9d5462f7ce08");  //  I ASSUME THESE NEED TO MATCH ON BOTH ARDUINOS.  
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("0d563a58-196a-48ce-ace2-dfec78acc814"); //  I ASSUME THESE NEED TO MATCH ON BOTH ARDUINOS.  

static BLEAddress *pServerAddress;
static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
BLEScan* pBLEScan = BLEDevice::getScan();

int lastTransmissionTime=0;// not to use delays , we use this variable to check if a certain amount of time has passed , say 100ms

std::vector<int> stringToInt(std::string str){
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

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {  /* WHAT DOES THIS FUNCTION DO?
                                                                                                                                  it is called when the server notifies the client,
                                                                                                                                  now we are having the client asking and sending data
                                                                                                                                  to the server, but what if we want the server to tell the 
                                                                                                                                  client that there is new data, we use notifications ,
                                                                                                                                  this is not functional for now , but we can add it in the 
                                                                                                                                  future if necessary                                                                                                                                  */
  
  
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
} // End notifyCallback

bool connectToServer(BLEAddress pAddress) {   //  DOES THIS FUNCTION JUST CONNECT TO SERVER? it also finds the service and gets the characheteristic
    Serial.print("Forming a connection to ");
    Serial.println(pAddress.toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");
    // Connect to the remove BLE Server.
    pClient->connect(pAddress);
    Serial.println(" - Connected to server");
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);  
    
   
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());

    pRemoteCharacteristic->registerForNotify(notifyCallback);
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
 
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks { /* WHAT DOES THIS FUNCTION DO?
                                                                            these are callback functions called when there is a trigger for it,
                                                                            like the onResult function is called when there are scan results
                                                                            ( when the BLE scan finds BLE servers)
                                                                          */
  
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    String nameOfDevice=advertisedDevice.getName().c_str();
    String MacOfDevice=advertisedDevice.getName().c_str();
    if (nameOfDevice==DEVICE_NAME || MacOfDevice==MAC_ADRESSE ) {
    Serial.print("Found our device!  address: "); 
    advertisedDevice.getScan()->stop();

    pServerAddress = new BLEAddress(advertisedDevice.getAddress());
    doConnect = true;
    }
    // Found our server
  } // onResult
}; // End  MyAdvertisedDeviceCallbacks

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("MyEsp32");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  BLEScanResults pBLEScanResults =pBLEScan->start(30);
  Serial.println(pBLEScanResults.getCount());
} // End of setup.

// bool oneOnTwoTimes=true;

int Pitch;
float Throttle;
int Angle;

int RPM;
float M1_Temp;
float Current;
float Airspeed;

void loop() {

  Pitch = 60;
  Throttle = 75;
  Angle = 120;
  
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.

 if((millis()-lastTransmissionTime)>50){//  Transmit interval 
  lastTransmissionTime = millis();
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
      doConnect = false;
  } // End if 

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached with the current time since boot.
  
  if (connected) {
      // Store all Data to String Here  
      String newValue = String(Pitch) + "," + String(Throttle) + "," + String(Angle);// no spaces between variables , only ","
      // Serial.println("Data to Send: " + newValue);

      //HERE!!!!! is how you send data to server **********************************************
      pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());// use writeValue().c_str() to write data   --  Set the characteristic's value to be the array of bytes that is actually a string.
    

      //HERE!!!!! is how you get the data sent from server ******************************
      Serial.print("Reading data: ");
      std::vector<int> out=stringToInt(pRemoteCharacteristic->readValue().c_str());// use readValue().c_str() to read data and it's converted to an int vector.
      // HOW DO I GET THE DATA OUT OF THE STRING SO I CAN STORE THEM TO INTS AND FLOAT VARIABLES?  WHAT I WANT TO EXTRACT BELOW
      //here is how you can do it now 
      /*
      RPM = out[0];
      M1_Temp = out[1];
      Current = out[2];
      Airspeed = out[3];
*/
      RPM = out[0];
      Serial.print("RPM : ");
      Serial.println(RPM);

  }else{
    pBLEScan->start(30);
    doConnect=true;
  } // End else
 }  // End Transmission If
} // End of Main loop
