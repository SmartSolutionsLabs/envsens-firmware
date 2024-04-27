#ifndef INC_BLE_CONNECTION_LISTENER
#define INC_BLE_CONNECTION_LISTENER

#include <BLEServer.h>
#include <Arduino.h>

class BleConnectionListener : public BLEServerCallbacks {
	public:
		void onConnect(BLEServer * bluetoothServer);
		void onDisconnect(BLEServer * bluetoothServer);
};
#endif
