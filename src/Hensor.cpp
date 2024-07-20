#include "Hensor.hpp"
#include "Communicator.hpp"
#include "Multisensor.hpp"
#include "NH3sensor.hpp"
#include "CO2sensor.hpp"
#include "Network.hpp"

#include <Wire.h>

Hensor * Hensor::hensor = nullptr;

Hensor * Hensor::getInstance() {
	if(hensor == nullptr) {
		hensor = new Hensor();
	}

	return hensor;
}

Hensor::Hensor() {
	// Section for loading parameters
	this->preferences.begin("hensor", false); // Namespace for everything
	this->setEndpointHostname(this->preferences.getString("hostname", ""), false);
	this->setEndpointPost(this->preferences.getString("post", ""), false);
	this->setDeviceName(this->preferences.getString("name", ""), false);
	// To decide if we must turn on WiFi
	if( this->inProductionMode = this->preferences.getBool("inProduction", false) ) {
		Network::SSID = this->preferences.getString("netSsid", "");
		Network::PASSWORD = this->preferences.getString("netPassword", "");
		Network::getInstance()->connect();
	}

	// Only change pins because already started in master mode
	Wire.setPins(5, 4);

	this->sensors[SENSOR_MULTI_INDEX] = new Multisensor("multi");
	this->sensors[SENSOR_MULTI_INDEX]->connect(&Wire);

	this->sensors[SENSOR_CO2_INDEX] = new CO2sensor("co2");
	this->sensors[SENSOR_CO2_INDEX]->connect(&Wire);

	this->sensors[SENSOR_NH3_INDEX] = new NH3sensor("nh3");
	this->sensors[SENSOR_NH3_INDEX]->connect(&Wire);
}

void Hensor::processMessage(String message) {
	Communicator::getInstance()->addInstruction(message);
}

Sensor * Hensor::getSensor(unsigned int index) const {
	if (index < SENSORS_QUANTITY) {
		return this->sensors[index];
	}

	return nullptr;
}

bool Hensor::getBluetoothDeviceConnected() const {
	return this->bluetoothDeviceConnected;
}

bool Hensor::getOldBluetoothDeviceConnected() const {
	return this->oldBluetoothDeviceConnected;
}

void Hensor::setBluetoothDeviceConnected(bool connected) {
	this->bluetoothDeviceConnected = connected;
}

void Hensor::setOldBluetoothDeviceConnected(bool connected) {
	this->oldBluetoothDeviceConnected = connected;
}

void Hensor::setWifiCredentials(String &ssid, String &password) {
	Network::SSID = ssid;
	this->preferences.putString("netSsid", ssid);

	Network::PASSWORD = password;
	this->preferences.putString("netPassword", password);

	Network::getInstance()->connect();
}

void Hensor::setEndpointHostname(String hostname, bool persistent) {
	if (persistent) {
		this->preferences.putString("hostname", hostname);
	}

	// Pass it directly to the communicator
	Communicator::getInstance()->setEndpointHostname(hostname);
}

void Hensor::setEndpointPost(String post, bool persistent) {
	if (persistent) {
		this->preferences.putString("post", post);
	}

	// Pass it directly to the communicator
	Communicator::getInstance()->setEndpointPost(post);
}

void Hensor::setDeviceName(String name, bool persistent) {
	if (persistent) {
		this->preferences.putString("name", name);
	}

	this->deviceName = name;
}

String Hensor::getDeviceName() const {
	return this->deviceName;
}

void Hensor::setProductionMode(bool mode) {
	this->inProductionMode = mode;
	this->preferences.putBool("inProduction", mode);
}

bool Hensor::isProductionMode() const {
	return this->inProductionMode;
}

void Hensor::holdCO2Value(uint32_t value) {
	this->currentDatagas.co2 = value;
}

void Hensor::holdNH3Value(uint32_t value) {
	this->currentDatagas.nh3 = value;
}

void Hensor::holdTemperatureValue(uint32_t value) {
	this->currentDatagas.temperature = value;
}

void Hensor::holdHumidityValue(uint32_t value) {
	this->currentDatagas.humidity = value;
}

void Hensor::holdPressureValue(uint32_t value) {
	this->currentDatagas.pressure = value;
}

Datagas Hensor::getCurrentDatagas() const {
	return this->currentDatagas;
}

void Hensor::setSendingOut(bool sending) {
	this->sendingOut = sending;
}

bool Hensor::isSendingOut() const {
	return this->sendingOut;
}
