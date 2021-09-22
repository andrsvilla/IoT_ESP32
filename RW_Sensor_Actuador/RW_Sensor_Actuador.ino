#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID                  "72df1985-2530-4604-b046-b131277fab48"
#define ACTUATOR_CHARACTERISTIC_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define TrigPin     33
#define EchoPin     32
#define SoundSpeed  0.034  //cm/uS
long    Duracion;
float   DistCm;
String  DistDeseada = "150";
String  Dato;

#define LedEnPin    23
#define LedOutPin   15
char    ledStatus   = 48; // 0 in ASCII

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *ActuatorCharacteristic) {
    std::string Value = ActuatorCharacteristic->getValue();
    String valStr = Value.c_str();
    if (valStr.toInt() != DistDeseada.toInt()) {
      digitalWrite(LedEnPin, HIGH);
      Serial.println("Value changed ...");
      Serial.print("New value: ");
      Serial.println(valStr);
      DistDeseada = valStr;
      delay(600);
      digitalWrite(LedEnPin, LOW);
    }
  }
};

// New characteristic with object to manage it
BLECharacteristic ActuatorCharacteristic(
  ACTUATOR_CHARACTERISTIC_UUID,
  BLECharacteristic::PROPERTY_READ |
  BLECharacteristic::PROPERTY_WRITE
);

BLECharacteristic *SensorCharacteristic;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE!");
  Serial.println("Initializing device");
  BLEDevice::init("Alarma Distancia"); // Initializing the device with its name
  Serial.println("Creating server");
  BLEServer *pServer = BLEDevice::createServer(); // Create the server
  Serial.println("Adding service UUID");
  BLEService *pService = pServer->createService(SERVICE_UUID); // Creating a new service into server
  
  // Adding a characteristic with the object name (official UUID), without object (this characteristic will not change)
  Serial.println("Adding sensor characteristic");
  SensorCharacteristic = pService->createCharacteristic(
                                 BLEUUID((uint16_t)0x2A00),
                                 BLECharacteristic::PROPERTY_READ
  );
  
  SensorCharacteristic->setValue((uint8_t*)&Dato, 10);
  
  // Adding a characteristic to control the led with 0 and 1
  Serial.println("Adding actuator characteristic");
  pService->addCharacteristic(&ActuatorCharacteristic);

  ActuatorCharacteristic.setCallbacks(new MyCallbacks());
  ActuatorCharacteristic.setValue(DistDeseada.c_str());
  
  Serial.println("Starting...");
  pService->start();
  
  Serial.println("Creating advertising");
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
  
  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  pinMode(LedEnPin, OUTPUT);
  pinMode(LedOutPin, OUTPUT);
}

void loop() {
  digitalWrite(TrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(TrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);
  Duracion = pulseIn(EchoPin, HIGH);
  DistCm = Duracion * SoundSpeed/2;
  Dato = String(DistCm, 2) + " cm";
  SensorCharacteristic->setValue((uint8_t*)&Dato, 10);
  if (DistCm >= DistDeseada.toInt()) {
    digitalWrite(LedOutPin, LOW); // LED Off
    ledStatus = 48;
  }
  else if (DistCm < DistDeseada.toInt()) {
    ledStatus = 49;
    digitalWrite(LedOutPin, HIGH); // LED On
  }
  ActuatorCharacteristic.setValue(DistDeseada.c_str());
  delay(900);
}
