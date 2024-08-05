#ifndef INC_HENSOR
#define INC_HENSOR

#define BLUETOOTH_DEVICE_NAME "Proteus G-1"
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
	float_t co2_a = 1.0f;
	float_t co2_b = 0.0f;
	float_t nh3_m = -1.800f;
	float_t nh3_n = -0.163f;
	uint32_t nh3_R0 = 98400;
	float_t nh3_maxV = 5.193f;
	float_t t_a = 1.0f;
	float_t t_b = 0.0f;
	float_t h_a = 1.0f;
	float_t h_b = 0.0f;
	float_t p_a = 1.0f;
	float_t p_b = 0.0f;
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

		float_t FunctionTemperatureCalibrated(float_t mT);
		float_t FunctionHumidityCalibrated(float_t mh);
		float_t FunctionPressureCalibrated(float_t mp);
		uint32_t FunctionNH3Calibrated(float_t nh3);
		uint32_t FunctionCO2Calibrated(float_t co2);

		void holdCO2Value(uint32_t value);
		void holdNH3Value(uint32_t value);
		void holdTemperatureValue(float_t value);
		void holdHumidityValue(float_t value);
		void holdPressureValue(uint32_t value);

		Datagas getCurrentDatagas();

		void setSendingOut(bool sending = true);
		bool isSendingOut() const;
		bool hasSentOnTime(int time);

		void setTime(String dateTime);
		DateTime getRtcNow();

		void assemblySensorsStatus(std::string &jsonString) ;

		void setTemperatureCalibration(float_t t_a, float_t t_b, bool persistent = true);
		void setHumidityCalibration(float_t h_a, float_t h_b, bool persistent = true);
		void setPressureCalibration(float_t p_a, float_t p_b, bool persistent = true);
		void setNH3Calibration(float_t nh3_m ,float_t nh3_n ,uint32_t nh3_R0 ,float_t nh3_maxV , bool persistent);
		void setCO2Calibration(float_t co2_a, float_t co2_b, bool persistent = true);

		float_t getTemperatureCalibration(int index);
		float_t getHumidityCalibration(int index);
		float_t getPressureCalibration(int index);
		float_t getNH3Calibration(int index);
		float_t getCO2Calibration(int index);
};

#endif
