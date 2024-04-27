#ifndef INC_HENSOR
#define INC_HENSOR

#define BLUETOOTH_DEVICE_NAME "Hensor Env"

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

	public:
		static Hensor * getInstance();

		Hensor(Hensor &other) = delete;

		void operator=(const Hensor &) = delete;

		// About Bluetooth
		bool getBluetoothDeviceConnected() const;
		bool getOldBluetoothDeviceConnected() const;
		void setBluetoothDeviceConnected(bool connected);
		void setOldBluetoothDeviceConnected(bool connected);
};

#endif
