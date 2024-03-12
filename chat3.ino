#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint16_t potValue = 0; // Change data type to uint16_t

const int potPin = 15; // Replace with the actual pin connected to your potentiometer

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

void setup() {
    Serial.begin(115200);

    BLEDevice::init("ESP32");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(BLEUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b"));

    pCharacteristic = pService->createCharacteristic(
                      BLEUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8"),
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
}

void loop() {
    if (deviceConnected) {
        int sensorValue = analogRead(potPin);

        // Map the sensor value to the full range of uint16_t (0 - 65535)
        potValue = map(sensorValue, 0, 4095, 0, 65535); // Adjust the mapping based on your potentiometer range

        // Convert the potentiometer value to a string and send as ASCII characters
        String stringValue = String(potValue);
        pCharacteristic->setValue(stringValue.c_str());
        pCharacteristic->notify();
        Serial.println(potValue);


        delay(10); // Adjust the delay as needed
    }
}
