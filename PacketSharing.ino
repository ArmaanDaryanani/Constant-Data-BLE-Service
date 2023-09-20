#include <ArduinoBLE.h>
BLEService microphoneService("19b10000-e8f2-537e-4f6c-d104768a1214");
BLEByteCharacteristic switchCharacteristic("19b10000-e8f2-537e-4f6c-d104768a1214", BLERead | BLEWrite);

const int microphonePin = A0;
const int number_of_bytes = 10;
const bool wait_for_serial = true; //for debug
void setup() 
{
  Serial.begin(115200);
  while(!Serial && wait_for_serial);

  pinMode(microphonePin, INPUT);
  
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
  Serial.println("Advertising");
}

void loop() 
{
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
      sendDataPacket(10, microphonePin, switchCharacteristic);
      digitalWrite(LEDB, LOW);
      digitalWrite(LEDB, HIGH);
    }
    

  }

}

uint8_t* recordBytes(int number_of_bytes, int pin)
{
  uint8_t* data_array = new uint8_t[number_of_bytes];
  for(int i = 0; i < number_of_bytes; i++)
  { 
    digitalWrite(LEDG, LOW);
    int val = analogRead(pin);
    data_array[i] = map(val, 300, 600, 0, 255);
    digitalWrite(LEDG, HIGH);
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
          Serial.print("Sent data packet: ");
          Serial.println(data_array[i]);
          delay(10);
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

