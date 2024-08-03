#ifndef INC_HENSOR
#define INC_HENSOR

#define BLUETOOTH_DEVICE_NAME "Proteus G-2"
#define SENSORS_QUANTITY 3
#define SENSOR_MULTI_INDEX 0
#define SENSOR_NH3_INDEX 1
#define SENSOR_CO2_INDEX 2

#include <Arduino.h>
#include <Preferences.h>
#include <RTClib.h>

#include "Sensor.hpp"
#include "Datalogger.hpp"

/**
 * Hold multiplier constants to datagas values.
 */
struct Calibration {
	uint32_t co2 = 1;
	float_t nh3 = 1.0f;
	float_t temperature = 1.0f;
	float_t humidity = 1.0f;
	uint32_t pressure = 1;
};

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

		Calibration calibration;

		/**
		 * Simple storage for data needed in runtime.
		 */
		Preferences preferences;

		/**
		 * Name of device for identify it
		 */
		String deviceName;

		String deviceSerialNumber;
		/**
		 * To decide turning WiFi for production or BLE to configuration.
		 */
		bool inProductionMode;

		/**
		 * All data are resting here.
		 */
		Datagas currentDatagas;

		volatile bool sendingOut = false;

		RTC_DS3231 rtc;

		bool rtcAvailable = true;

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

		// About HTTP endpoints
		void setEndpointHostname(String hostname, bool persistent = true);
		void setEndpointPost(String post, bool persistent = true);

		// Initial parameters in boot
		void setDeviceName(String name, bool persistent = true);
		String getDeviceName() const;

		void setDeviceSerialNumber(String serialNumber, bool persistent = true);
		String getDeviceSerialNumber() const;

		void setNetworkInterval(uint32_t minutes, bool persistent = true);
		void setLocalInterval(uint32_t time, bool persistent = true);

		// Control
		void setProductionMode(bool mode = true);
		bool isProductionMode() const;

		void setCO2Multiplier(uint32_t multiplier, bool persistent = true);
		void setNH3Multiplier(float_t multiplier, bool persistent = true);
		void setTemperatureMultiplier(float_t multiplier, bool persistent = true);
		void setHumidityMultiplier(float_t multiplier, bool persistent = true);
		void setPressureMultiplier(uint32_t multiplier, bool persistent = true);
		uint32_t getCO2Multiplier() const;
		float_t getNH3Multiplier() const;
		float_t getTemperatureMultiplier() const;
		float_t getHumidityMultiplier() const;
		uint32_t getPressureMultiplier() const;

		float_t FunctionTemperatureCalibrated(float_t meassuredTemperature);
		float_t FunctionHumidityCalibrated(float_t mh);

		void holdCO2Value(uint32_t value);
		void holdNH3Value(float_t value);
		void holdTemperatureValue(float_t value);
		void holdHumidityValue(float_t value);
		void holdPressureValue(uint32_t value);

		Datagas getCurrentDatagas();

		void setSendingOut(bool sending = true);
		bool isSendingOut() const;

		void setTime(String dateTime);
		DateTime getRtcNow();

		void assemblySensorsStatus(std::string &jsonString) ;
};

#endif
