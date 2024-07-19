#include "Ble.hpp"
#include "Hensor.hpp"
#include "Datalogger.hpp"
#include "Communicator.hpp"

Hensor * hensor;
Datalogger * datalogger;

TickType_t xDelay;

// Controlling motor acceleration
void IRAM_ATTR interruptDataSaver(void* arg) {
	datalogger->setSaving();
}

void setup() {
	Serial.begin(115200);
	xDelay = 500 / portTICK_PERIOD_MS;

	hensor = Hensor::getInstance();

	datalogger = Datalogger::getInstance();
	datalogger->start();

	Communicator::getInstance()->start();

	Ble * ble = new Ble(BLUETOOTH_DEVICE_NAME);
}

void loop() {

	// disconnecting
	if(!hensor->getBluetoothDeviceConnected() && hensor->getOldBluetoothDeviceConnected()) {
		vTaskDelay(xDelay); // give the bluetooth stack the chance to get things ready
		Ble::bluetoothServer->startAdvertising(); // restart advertising
		Serial.println("restart BT advertising");
		hensor->setOldBluetoothDeviceConnected(hensor->getBluetoothDeviceConnected());
	}

	// connecting
	if(hensor->getBluetoothDeviceConnected() && !hensor->getOldBluetoothDeviceConnected()) {
		// do stuff here on connecting
		hensor->setOldBluetoothDeviceConnected(hensor->getBluetoothDeviceConnected());
	}
}
