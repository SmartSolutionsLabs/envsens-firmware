#ifndef INC_HENSOR
#define INC_HENSOR

#define BLUETOOTH_DEVICE_NAME "Hensor Env"
#define SENSORS_QUANTITY 3
#define SENSOR_MULTI_INDEX 0
#define SENSOR_NH3_INDEX 1
#define SENSOR_CO2_INDEX 2

#include <Arduino.h>

#include "Sensor.hpp"

/**
 * This class has all data and logic.
 * It's singleton.
 */

class Hensor {
	protected:
		static Hensor * hensor;
		Hensor();

		bool bluetoothDeviceConnected = false;
		bool oldBluetoothDeviceConnected = false;

		Sensor * sensors[SENSORS_QUANTITY];

	public:
		static Hensor * getInstance();

		Hensor(Hensor &other) = delete;

		void operator=(const Hensor &) = delete;

		void processMessage(String message);

		/**
		 * Pointer of any sensor.
		 */
		Sensor * getSensor(unsigned int index) const;

		// About Bluetooth
		bool getBluetoothDeviceConnected() const;
		bool getOldBluetoothDeviceConnected() const;
		void setBluetoothDeviceConnected(bool connected);
		void setOldBluetoothDeviceConnected(bool connected);

		// About WiFi
		void getWifiCredentials(String &ssid, String &password) const;
		void setWifiCredentials(String &ssid, String &password);
};

#endif
