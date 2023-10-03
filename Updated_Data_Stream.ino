#include <ArduinoBLE.h>

BLEService microphoneService("19B10010-E8F2-537E-4F6C-D104768A1214"); 

BLEByteCharacteristic microphoneCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEByteCharacteristic buttonCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);

const int microphonePin = A0;
const int buttonPin = 4;

void setup() 
{
  Serial.begin(9600);
  while (!Serial);

  pinMode(microphonePin, INPUT); 
  pinMode(buttonPin, INPUT);

  if (!BLE.begin()) 
  {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  BLE.setLocalName("MicrophoneTest");

  BLE.setAdvertisedService(ledService);

  microphoneService.addCharacteristic(microphoneCharacteristic);
  microphoneService.addCharacteristic(buttonCharacteristic);

  BLE.addService(microphoneService);

  microphoneCharacteristic.writeValue(0);
  buttonCharacteristic.writeValue(0);

  BLE.advertise();

  Serial.println("Bluetooth® device active, waiting for connections...");
}

void loop() 
{
    BLE.poll();

    char buttonValue = digitalRead(buttonPin);
    int val = analogRead(microphonePin);
    bool buttonChanged = (buttonCharacteristic.value() != buttonValue);

    while(buttonChanged)
    {
      digitalWrite(LEDG, LOW);
      microphoneCharacteristic.writeValue(val);
      buttonCharacteristic.writeValue(buttonValue);
      Serial.println(val);
    }
    digitalWrite(LEDG, HIGH);

    if(microphoneCharacteristic.written())
    {
      digitalWrite(LEDR, LOW);
      delay(100);
    }
    digitalWrite(LEDR, HIGH);

}
