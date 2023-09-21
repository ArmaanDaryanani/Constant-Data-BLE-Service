#include <ArduinoBLE.h>
#include <Arduino.h>
#include "RPC.h"

BLEService microphoneService("19b10000-e8f2-537e-4f6c-d104768a1214");
BLEByteCharacteristic switchCharacteristic("19b10000-e8f2-537e-4f6c-d104768a1214", BLERead | BLEWrite);

const int microphonePin = A0;
const int number_of_bytes = 10;
const bool wait_for_serial = true; //for debug
int recording_duration = 5; //seconds
bool isRecording = false;

void setup() 
{
  //------------------------------CORE M7: Setup------------------------------//
  #ifdef CORE_CM7
  Serial.begin(115200);
  while(!Serial && wait_for_serial);

  pinMode(microphonePin, INPUT);
  
  //starts M4 core using RPC 
  RPC.begin(); 
  bootM4();

  //creates bind between cores
  //RPC.bind("shareBool", shareBool);

  if(!BLE.begin())
  {
    Serial.println("BLE failed");
    while(true)
    {
      digitalWrite(LEDR, LOW);
      delay(1000);
      digitalWrite(LEDR, HIGH);
      delay(1000);
      if(BLE.begin()){break;}
    }
  }
  BLE.setAdvertisedService(microphoneService);

  microphoneService.addCharacteristic(switchCharacteristic);

  BLE.addService(microphoneService);

  BLE.advertise();
  Serial.println("Advertising now");
  #endif

  //------------------------------CORE M4: Setup------------------------------//
  #ifdef CORE_CM4
  pinMode(LEDG, OUTPUT);

  RPC.bind("receiveValue", receiveValueFromM7);
  #endif
}

void loop() 
{
  //------------------------------CORE M7: Loop------------------------------//
  #ifdef CORE_CM7
  digitalWrite(LEDB, LOW);
  BLEDevice central = BLE.central();

  if(central)
  {
    digitalWrite(LEDB, HIGH);
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    customLEDScript1();
    customLEDScript1();
    delay(1000);

    while(central.connected())
    {
      for(int i = 0; i < calculate_cycles_from_duration(recording_duration); i++)
      {
        if(i == 0)
        {
          int result = RPC.call("receiveValue", 1).as<int>();
          if (result != 1) 
          {
            Serial.println("RPC call failed.");
          } 
          else 
          {
            Serial.println("RPC call successful");
          }
        }
        
        sendDataPacket(10, microphonePin, switchCharacteristic);
        //other core (m4) is blinking whenever recording
        //each cycle is 5ms
      }
      RPC.call("receiveValue", 0).as<int>();
    }
    Serial.println(" Finished with " + central.address());
  }
  #endif
  //------------------------------CORE M4: Loop------------------------------//
  #ifdef CORE_CM4
  if(isRecording)
  {
    digitalWrite(LEDG, LOW);
    delay(30);
    digitalWrite(LEDG, HIGH);
    delay(30);
  }
    
  #endif
}

//------------------------------FUNCTIONS------------------------------//
#ifdef CORE_CM7
uint8_t* recordBytes(int number_of_bytes, int pin)
{
  uint8_t* data_array = new uint8_t[number_of_bytes];
  for(int i = 0; i < number_of_bytes; i++)
  { 
    int val = analogRead(pin);
    data_array[i] = map(val, 300, 600, 0, 255);
  }
  return data_array;
}
void sendDataPacket(int number_of_bytes, int pin, BLEByteCharacteristic& swtch)
{
    uint8_t* data_array = recordBytes(number_of_bytes, pin);

    for (int i = 0; i < number_of_bytes; i++) 
    {
        if (!swtch.writeValue(data_array[i])) 
        {
            Serial.println("Failed to send data packet");
            break;
        }
        else
        {
          //Serial.print("Sent data packet: ");
          //Serial.println(data_array[i]);
          //delay(10);
        }
    }
    delete[] data_array;
}

void customLEDScript1()
{
  delay(100);
  digitalWrite(LEDR, LOW);
  delay(500);
  digitalWrite(LEDG, LOW);
  delay(500);
  digitalWrite(LEDB, LOW);
  delay(500);
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
}

long calculate_cycles_from_duration(int seconds)
{
  int loop_time = 5; //ms, how long it takes to complete one write loop to ble
  return((seconds * 1000) / loop_time);
}
#endif
#ifdef CORE_CM4
int receiveValueFromM7(int value) 
{
  if(value == 1)
  {
    isRecording = true;
  }
  else if(value == 0)
  {
    isRecording = false;
  }
  return 0;
}
#endif

