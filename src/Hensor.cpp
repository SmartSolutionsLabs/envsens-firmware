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

	// Only change pins because already started in master mode
	Wire.setPins(5, 4);

	this->sensors[SENSOR_MULTI_INDEX] = new Multisensor("multi");
	this->sensors[SENSOR_MULTI_INDEX]->connect(&Wire);
	this->sensors[SENSOR_MULTI_INDEX]->start();

	this->sensors[SENSOR_CO2_INDEX] = new CO2sensor("co2");
	this->sensors[SENSOR_CO2_INDEX]->connect(&Wire);
	this->sensors[SENSOR_CO2_INDEX]->start();

	this->sensors[SENSOR_NH3_INDEX] = new NH3sensor("nh3");
	this->sensors[SENSOR_NH3_INDEX]->connect(&Wire);
	this->sensors[SENSOR_NH3_INDEX]->start();
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
	this->preferences.putString("netSsid", password);

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
