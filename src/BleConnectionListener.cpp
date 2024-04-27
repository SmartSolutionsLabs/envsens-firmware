#include "BleConnectionListener.hpp"
#include "Hensor.hpp"

void BleConnectionListener::onConnect(BLEServer * bluetoothServer) {
	Serial.print("\tBle dev. connected\n");
	Hensor::getInstance()->setBluetoothDeviceConnected(true);
}

void BleConnectionListener::onDisconnect(BLEServer * bluetoothServer) {
	Serial.print("\tBle dev. disconnected\n");
	Hensor::getInstance()->setBluetoothDeviceConnected(false);
}
