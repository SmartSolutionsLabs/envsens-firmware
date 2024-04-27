#ifndef INC_BLE
#define INC_BLE

#include "BleMessageListener.hpp"
#include "BleConnectionListener.hpp"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLE2902.h>

#define BLE_SERVICE_UUID  "a1e00001-8761-d3d4-e0ef-a81ab129baf0"
#define BLE_READ_UUID     "a1e00002-8761-d3d4-e0ef-a81ab129baf0"  // Listen
#define BLE_WRITE_UUID    "a1e00003-8761-d3d4-e0ef-a81ab129baf0"  // Send out
#define BLE_STATUS_UUID   "a1e00009-8761-d3d4-e0ef-a81ab129baf0"  // Notify sensor data

#define MTU_SIZE 517

class Ble {
	private:
		BLEDevice device;
		BLEService * dataService = nullptr;
		BleConnectionListener * connectionListener = nullptr;

	public:
		Ble(const char * name);

		static BleMessageListener * bleCallback;

		static BLECharacteristic *resCharacteristic;
		static BLECharacteristic *statusCharacteristic;
		static BLECharacteristic *reqCharacteristic;

		static BLEServer * bluetoothServer;
};

#endif
