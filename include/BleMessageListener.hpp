#ifndef INC_BLE_LISTENER
#define INC_BLE_LISTENER

#include <BLEServer.h>
#include <Arduino.h>

class BleMessageListener : public BLECharacteristicCallbacks {
	public:
		BleMessageListener();

		void onWrite(BLECharacteristic * pCharacteristic);

		void writeLargeText(BLECharacteristic * characteristic, std::string largeText);
};
#endif
