#include "Ble.hpp"

BleMessageListener * Ble::bleCallback = nullptr;

BLECharacteristic * Ble::resCharacteristic = nullptr;
BLECharacteristic * Ble::reqCharacteristic = nullptr;
BLECharacteristic * Ble::statusCharacteristic = nullptr;

BLEServer * Ble::bluetoothServer = nullptr;

Ble::Ble(const char * name){
	this->device.init(name); // name of esp32 ble.
	this->device.setMTU(MTU_SIZE);
	this->bluetoothServer = this->device.createServer();

	this->connectionListener = new BleConnectionListener();

	this->bluetoothServer->setCallbacks(this->connectionListener);

	this->dataService = this->bluetoothServer->createService(BLE_SERVICE_UUID);

	this->reqCharacteristic = this->dataService->createCharacteristic(
									BLE_READ_UUID,
									BLECharacteristic::PROPERTY_WRITE);

	Ble::bleCallback = new BleMessageListener();

	Ble::reqCharacteristic->setCallbacks(Ble::bleCallback);

	Ble::resCharacteristic = this->dataService->createCharacteristic(
									BLE_WRITE_UUID,
									BLECharacteristic::PROPERTY_NOTIFY);  //PROPERTY_NOTIFY

	Ble::resCharacteristic->addDescriptor(new BLE2902());

	Ble::statusCharacteristic = this->dataService->createCharacteristic(
									BLE_STATUS_UUID,
									BLECharacteristic::PROPERTY_NOTIFY);

	Ble::statusCharacteristic->addDescriptor(new BLE2902());

	this->dataService->start();

	Ble::bluetoothServer->getAdvertising()->start();

	Serial.print("BT server created\n");
}
