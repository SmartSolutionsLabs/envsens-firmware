#include "Hensor.hpp"
#include "Multisensor.hpp"
#include "NH3sensor.hpp"
#include "CO2sensor.hpp"

#include <Wire.h>

Hensor * Hensor::hensor = nullptr;

Hensor * Hensor::getInstance() {
	if(hensor == nullptr) {
		hensor = new Hensor();
	}

	return hensor;
}

Hensor::Hensor() {
	TwoWire twoWire = TwoWire(0);

	this->sensors[SENSOR_MULTI_INDEX] = new Multisensor("multi");
	this->sensors[SENSOR_MULTI_INDEX]->connect(&twoWire);
	this->sensors[SENSOR_MULTI_INDEX]->start();

	this->sensors[SENSOR_CO2_INDEX] = new CO2sensor("co2");
	this->sensors[SENSOR_CO2_INDEX]->connect(&twoWire);
	this->sensors[SENSOR_CO2_INDEX]->start();

	this->sensors[SENSOR_NH3_INDEX] = new NH3sensor("nh3");
	this->sensors[SENSOR_NH3_INDEX]->connect(nullptr);
	this->sensors[SENSOR_NH3_INDEX]->start();
}

void Hensor::processMessage(String message) {
	Serial.println(message);
}

Sensor * Hensor::getSensor(unsigned int index) const {
	return this->sensors[index];
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
