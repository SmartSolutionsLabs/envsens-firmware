#include "Hensor.hpp"
#include "Communicator.hpp"
#include "Multisensor.hpp"
#include "NH3sensor.hpp"
#include "CO2sensor.hpp"
#include "Network.hpp"
#include <ArduinoJson.h>

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
	this->setNetworkInterval(this->preferences.getUInt("interval", 5), false);
	this->setLocalInterval(this->preferences.getUInt("intervalLocal", 15), false);
	this->setCO2Multiplier(this->preferences.getUInt("kCO2", 1), false);
	this->setNH3Multiplier(this->preferences.getFloat("kNH3", 1.0f), false);
	this->setTemperatureMultiplier(this->preferences.getFloat("kTemperature", 1.0f), false);
	this->setHumidityMultiplier(this->preferences.getUInt("kHumidity", 1), false);
	this->setPressureMultiplier(this->preferences.getUInt("kPressure", 1), false);
	Network::SSID = this->preferences.getString("netSsid", "");
	Network::PASSWORD = this->preferences.getString("netPassword", "");
	// To decide if we must turn on WiFi
	if( this->inProductionMode = this->preferences.getBool("inProduction", false) ) {
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

	if (! this->rtc.begin()) {
		Serial.println("Couldn't find RTC");
		this->rtcAvailable = false;
	}
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

void Hensor::getWifiCredentials(String &ssid, String &password) const {
	ssid = Network::SSID;
	password = Network::PASSWORD;
}

void Hensor::setWifiCredentials(String &ssid, String &password) {
	Network::SSID = ssid;
	this->preferences.putString("netSsid", ssid);

	Network::PASSWORD = password;
	this->preferences.putString("netPassword", password);
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

void Hensor::setNetworkInterval(uint32_t minutes, bool persistent) {
	if (persistent) {
		this->preferences.putUInt("interval", minutes);
	}

	Communicator::getInstance()->setNetworkInterval(minutes);
}

void Hensor::setLocalInterval(uint32_t time, bool persistent) {
	if (persistent) {
		this->preferences.putUInt("intervalLocal", time);
	}

	Communicator::getInstance()->setLocalInterval(time);
}

void Hensor::setProductionMode(bool mode) {
	this->inProductionMode = mode;
	this->preferences.putBool("inProduction", mode);
}

bool Hensor::isProductionMode() const {
	return this->inProductionMode;
}

void Hensor::setCO2Multiplier(uint32_t multiplier, bool persistent) {
	if (persistent) {
		this->preferences.putUInt("kCO2", multiplier);
	}

	this->calibration.co2 = multiplier;
}

void Hensor::setNH3Multiplier(float_t multiplier, bool persistent) {
	if (persistent) {
		this->preferences.putFloat("kNH3", multiplier);
	}

	this->calibration.nh3 = multiplier;
}

void Hensor::setTemperatureMultiplier(float_t multiplier, bool persistent) {
	if (persistent) {
		this->preferences.putFloat("kTemperature", multiplier);
	}

	this->calibration.temperature = multiplier;
}

void Hensor::setHumidityMultiplier(uint32_t multiplier, bool persistent) {
	if (persistent) {
		this->preferences.putUInt("kHumidity", multiplier);
	}

	this->calibration.humidity = multiplier;
}

void Hensor::setPressureMultiplier(uint32_t multiplier, bool persistent) {
	if (persistent) {
		this->preferences.putUInt("kPressure", multiplier);
	}

	this->calibration.pressure = multiplier;
}

uint32_t Hensor::getCO2Multiplier() const {
	return this->calibration.co2;
}

float_t Hensor::getNH3Multiplier() const {
	return this->calibration.nh3;
}

float_t Hensor::getTemperatureMultiplier() const {
	return this->calibration.temperature;
}

uint32_t Hensor::getHumidityMultiplier() const {
	return this->calibration.humidity;
}

uint32_t Hensor::getPressureMultiplier() const {
	return this->calibration.pressure;
}

void Hensor::holdCO2Value(uint32_t value) {
	this->currentDatagas.co2 = value;
}

void Hensor::holdNH3Value(float_t value) {
	this->currentDatagas.nh3 = value;
}

void Hensor::holdTemperatureValue(float_t value) {
	this->currentDatagas.temperature = value;
}

void Hensor::holdHumidityValue(uint32_t value) {
	this->currentDatagas.humidity = value;
}

void Hensor::holdPressureValue(uint32_t value) {
	this->currentDatagas.pressure = value;
}

Datagas Hensor::getCurrentDatagas() {
	this->currentDatagas.unixtime = this->rtc.now().unixtime();
	return this->currentDatagas;
}

void Hensor::setSendingOut(bool sending) {
	digitalWrite(CONFIG_STATUS_LED_PIN, sending ? HIGH : LOW);
	this->sendingOut = sending;
}

bool Hensor::isSendingOut() const {
	return this->sendingOut;
}

void Hensor::setTime(String dateTime) {
	if (this->rtcAvailable) {
		this->rtc.adjust(DateTime(
			dateTime.substring(0,   4).toInt(),
			dateTime.substring(4,   6).toInt(),
			dateTime.substring(6,   8).toInt(),
			dateTime.substring(8,  10).toInt(),
			dateTime.substring(10, 12).toInt(),
			dateTime.substring(12, 14).toInt()
		));
	}
}

DateTime Hensor::getRtcNow() {
	return this->rtc.now();
}

void Hensor::assemblySensorsStatus(std::string &jsonString) {
	Datagas currentDatagas = this->getCurrentDatagas();
	DateTime dateTime(currentDatagas.unixtime);

	JsonDocument jsonResponse;
	//jsonResponse["Equipo"] = deviceName;
	//jsonResponse["FechaHora"] = dateTime.timestamp(DateTime::TIMESTAMP_DATE) + " " + dateTime.timestamp(DateTime::TIMESTAMP_TIME);
	//jsonResponse["CO2"] = currentDatagas.co2;
	//jsonResponse["NH3"] = currentDatagas.nh3;
	//jsonResponse["Temp"] = currentDatagas.temperature;
	//jsonResponse["HR"] = currentDatagas.humidity;
	//jsonResponse["PR"] = currentDatagas.pressure;

	int i = 1;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 6;
	jsonResponse["data"][i]["name"] = "Equipo";
	jsonResponse["data"][i]["l1"] = deviceName;
	jsonResponse["data"][i]["l2"] = "";

	i++;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 6;
	jsonResponse["data"][i]["name"] = "Hora de Sistema";
	jsonResponse["data"][i]["l1"] = dateTime.timestamp(DateTime::TIMESTAMP_DATE);
	jsonResponse["data"][i]["l2"] = dateTime.timestamp(DateTime::TIMESTAMP_TIME);

	i++;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 6;
	jsonResponse["data"][i]["name"] = "CO2";
	jsonResponse["data"][i]["l1"] = String(currentDatagas.co2) + " PPM";
	jsonResponse["data"][i]["l2"] = "";

	i++;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 6;
	jsonResponse["data"][i]["name"] = "NH3";
	jsonResponse["data"][i]["l1"] = String(currentDatagas.nh3)+ " PPM";
	jsonResponse["data"][i]["l2"] = "";

	i++;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 6;
	jsonResponse["data"][i]["name"] = "Temperature";
	jsonResponse["data"][i]["l1"] = String(currentDatagas.temperature) + " °C";
	jsonResponse["data"][i]["l2"] = "";

	i++;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 6;
	jsonResponse["data"][i]["name"] = "Humidity";
	jsonResponse["data"][i]["l1"] = String(currentDatagas.humidity) + " %";
	jsonResponse["data"][i]["l2"] = "";

	i++;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 6;
	jsonResponse["data"][i]["name"] = "Pressure";
	jsonResponse["data"][i]["l1"] = String(currentDatagas.pressure) + " Pa";
	jsonResponse["data"][i]["l2"] = "";

	serializeJson(jsonResponse, jsonString);
}
