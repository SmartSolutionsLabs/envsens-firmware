#include "Ble.hpp"
#include "Hensor.hpp"
#include "Datalogger.hpp"
#include "Communicator.hpp"

#define BOOT_PIN 0

Hensor * hensor;
Datalogger * datalogger;

TickType_t xDelay;

// Controlling motor acceleration
void IRAM_ATTR interruptDataSaver(void* arg) {
	datalogger->setSaving();
}

void setup() {
	pinMode(BOOT_PIN, INPUT); // Button to change mode BLE/WiFi

	Serial.begin(115200);
	xDelay = 500 / portTICK_PERIOD_MS;

	hensor = Hensor::getInstance();
	hensor->getSensor(SENSOR_MULTI_INDEX)->start();
	hensor->getSensor(SENSOR_CO2_INDEX)->start();
	hensor->getSensor(SENSOR_NH3_INDEX)->start();

	datalogger = Datalogger::getInstance();
	datalogger->start();

	Communicator::getInstance()->start();

	// We can't enable BLE & WiFi at the same time: https://github.com/espressif/esp-idf/issues/12414
	// and that's why we enable just one
	if ( !hensor->isProductionMode() ) {
		// We turn on safetly
		Ble * ble = new Ble(BLUETOOTH_DEVICE_NAME);
	}
}


void loop() {
	// Following lines are for change mode config/production

	static bool pushedButton = false;
	static TickType_t startInstant = 0;

	static bool buttonStatus;
	buttonStatus = digitalRead(BOOT_PIN);

	// If button is pushed
	if (buttonStatus == LOW) {
		if (!pushedButton) {
			// Record push start time
			startInstant = xTaskGetTickCount();
			pushedButton = true;
		}

		// Verify if button is pressed and hold for 3 seconds
		if (xTaskGetTickCount() - startInstant >= 3000) {
			// Execute the function we need to change if BLE or WiFi in next start
			hensor->setProductionMode( !hensor->isProductionMode() ); // toggle

			esp_restart(); // Everything is ended
		}
	}
	else {
		// Reset button status if is released before the 3 seconds
		pushedButton = false;
	}

	if ( hensor->isProductionMode() ) {
		// If we are in production mode it means that BLE is not available
		return;
	}

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
