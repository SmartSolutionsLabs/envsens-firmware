#include "Hensor.hpp"
#include "Communicator.hpp"
#include "Multisensor.hpp"
#include "NH3sensor.hpp"
#include "CO2sensor.hpp"
#include "Network.hpp"
#include <ArduinoJson.h>

#include <Wire.h>
static const char * HENSOR_TAG = "Hensor";

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
	this->setType(this->preferences.getUChar("nodeType", NODE_HELPER_TYPE), false);
	this->setNetworkHostname(this->preferences.getString("networkHostName", ""), false);
	this->setEndpointHostname(this->preferences.getString("hostname", ""), false);
	this->setEndpointPost(this->preferences.getString("post", ""), false);
	this->setDeviceName(this->preferences.getString("name", ""), false);
	this->setDeviceSerialNumber(this->preferences.getString("serialNumber", ""), false);
	this->setBluetoothName(this->preferences.getString("btName", "Proteus"), false);
	this->setNetworkInterval(this->preferences.getUInt("interval", 2), false);
	this->setLocalInterval(this->preferences.getUInt("intervalLocal", 2), false);
	this->setCO2Calibration (this->preferences.getFloat("co2_a", 1.0f), this->preferences.getFloat("co2_b", 1.0f), false);
	//(float_t nh3_m ,float_t nh3_n ,uint32_t nh3_R0 ,float_t nh3_maxV
	this->setNH3Calibration  (this->preferences.getFloat("nh3_m", -1.800f), this->preferences.getFloat("nh3_n", -0.163f),
							this->preferences.getUInt("nh3_R0", 98400),this->preferences.getFloat("nh3_maxV", 5.193f),
							 false);
	this->setTemperatureCalibration(this->preferences.getFloat("t_a", 1.0f), this->preferences.getFloat("t_b", 1.0f), false);
	this->setHumidityCalibration   (this->preferences.getFloat("h_a", 1.0f), this->preferences.getFloat("h_b", 1.0f), false);
	this->setPressureCalibration   (this->preferences.getFloat("p_a", 1.0f), this->preferences.getFloat("p_b", 1.0f), false);
	Network::SSID = this->preferences.getString("netSsid", "");
	Network::PASSWORD = this->preferences.getString("netPassword", "");
	// To decide if we must turn on WiFi
	this->inProductionMode = this->preferences.getBool("inProduction", false);

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

void Hensor::setNetworkHostname(String newNetworkHostName, bool persistent){
	if (persistent) {
		this->preferences.putString("networkHostName", newNetworkHostName);
	}
	String networkhostname = "Envsens " + newNetworkHostName;
	Network::getInstance()->setNetworkHostname(networkhostname);
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

void Hensor::setType(uint8_t type, bool persistent) {
	if (persistent) {
		this->preferences.putUChar("nodeType", type);
	}

	this->type = type;
}

uint8_t Hensor::getType() const {
	return this->type;
}

void Hensor::setDeviceName(String name, bool persistent) {
	if (persistent) {
		this->preferences.putString("name", name);
	}

	this->deviceName = name;
}

String Hensor::getDeviceName() const {
	return this->deviceSerialNumber;
}

void Hensor::setDeviceSerialNumber(String serialNumber, bool persistent) {
	if (persistent) {
		this->preferences.putString("serialNumber", serialNumber);
	}
	this->deviceSerialNumber = serialNumber;
}

String Hensor::getDeviceSerialNumber() const {
	return this->deviceSerialNumber;
}

void Hensor::setBluetoothName(String bluetoothName, bool persistent){
	if (persistent) {
		this->preferences.putString("btName", bluetoothName);
	}
	this->deviceBluetoothName = bluetoothName;
}

String Hensor::getBluetoothName() const{
	return this->deviceBluetoothName;
}

void Hensor::setNetworkInterval(uint32_t minutes, bool persistent) {
	if (minutes < 2) {
		minutes = 2;
	}
	if (minutes > 59) {
		minutes = 59;
	}

	if (persistent) {
		this->preferences.putUInt("interval", minutes);
	}

	Communicator::getInstance()->setNetworkInterval(minutes);
}

void Hensor::setLocalInterval(uint32_t time, bool persistent) {
	if (time < 2) {
		time = 2;
	}
	if (time > 59) {
		time = 59;
	}

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

void Hensor::setTemperatureCalibration(float_t t_a, float_t t_b, bool persistent) {
	if (persistent) {
		this->preferences.putFloat("t_a", t_a);
		this->preferences.putFloat("t_b", t_b);
	}

	this->calibration.t_a = t_a;
	this->calibration.t_b = t_b;
}

void Hensor::setHumidityCalibration(float_t h_a, float_t h_b, bool persistent) {
	if (persistent) {
		this->preferences.putFloat("h_a", h_a);
		this->preferences.putFloat("h_b", h_b);
	}

	this->calibration.h_a = h_a;
	this->calibration.h_b = h_b;
}

void Hensor::setPressureCalibration(float_t p_a, float_t p_b, bool persistent) {
	if (persistent) {
		this->preferences.putFloat("p_a", p_a);
		this->preferences.putFloat("p_b", p_b);
	}

	this->calibration.p_a = p_a;
	this->calibration.p_b = p_b;
}

void Hensor::setCO2Calibration(float_t co2_a, float_t co2_b, bool persistent){
	if (persistent) {
		this->preferences.putFloat("co2_a", co2_a);
		this->preferences.putFloat("co2_b", co2_b);
	}

	this->calibration.co2_a = co2_a;
	this->calibration.co2_b = co2_b;
}

void Hensor::setNH3Calibration(float_t nh3_m ,float_t nh3_n ,uint32_t nh3_R0 ,float_t nh3_maxV , bool persistent){
	if (persistent) {
		this->preferences.putFloat("nh3_m", nh3_m);
		this->preferences.putFloat("nh3_n", nh3_n);
		this->preferences.putUInt("nh3_R0", nh3_R0);
		this->preferences.putFloat("nh3_maxV", nh3_maxV);
	}

	this->calibration.nh3_m = nh3_m;
	this->calibration.nh3_n = nh3_n;
	this->calibration.nh3_R0 = nh3_R0;
	this->calibration.nh3_maxV = nh3_maxV;
}

float_t Hensor::FunctionTemperatureCalibrated(float_t mt){
	// LINEAR FUNCTION T = A * X + B
	//float_t T = 1.10493 *  meassuredTemperature - 16.30762;
	//float_t T =  1.0315 * meassuredTemperature - 0.9325; // G-1
	//float_t T = 1.0079 * meassuredTemperature + 0.4623; // G-3
	//float_t  T = mt; // G-2
	float_t  T = mt * this->calibration.t_a + this->calibration.t_b;
	return T;
}

float_t Hensor::FunctionHumidityCalibrated(float_t mh){
	// POLINOMIAL GRADE 3 FUNCTION H = A * X^3 + B * X^2 + C * X + D
	//float_t H = 0.00623 * mh * mh * mh - 0.52882 * mh * mh + 15.18842 * mh - 86.82952; // G-3
	//float_t H = 0.0073 * mh * mh * mh - 0.7689 * mh * mh - 272.2; // G-1
	//float_t H = 0.00533 * mh * mh * mh - 0.42805 * mh * mh + 11.90744 * mh - 51.83372;// G-2
	float_t H = mh * this->calibration.h_a + this->calibration.h_b;
	if(H < 0) H = 0;
	if(H > 100 ) H = 100;
	return H;
}

float_t Hensor::FunctionPressureCalibrated(float_t mp){
	float_t P = mp * this->calibration.p_a + this->calibration.p_b;
	return P;
}

uint32_t Hensor::FunctionNH3Calibrated(float_t nh3){
	//this->calibration.nh3_R0 = 98400;
	//this->calibration.nh3_maxVoltage = 5.193;

	float_t nh3_read_value = (nh3 * this->calibration.nh3_R0) / (this->calibration.nh3_maxV - nh3);

	// uint32_t nh3_in_ppm = pow(10, -1.8 * log(nh3_read_value/this->calibration.R0)/log(10) - 0.163);
	uint32_t nh3_in_ppm = pow(10, this->calibration.nh3_m * log(nh3_read_value/this->calibration.nh3_R0)/log(10) + this->calibration.nh3_n);

	return nh3_in_ppm;
}

uint32_t Hensor::FunctionCO2Calibrated(float_t co2){
	uint32_t co2_in_ppm = uint32_t(this->calibration.co2_a * co2 + this->calibration.co2_b);
	return co2_in_ppm;
}

float_t Hensor::getTemperatureCalibration(int index){
	float_t aux;
	if(index == 0){
		aux = this->calibration.t_a;
		return aux;
	}
	if(index == 1){
		aux = this->calibration.t_b;
		return aux;
	}
	return 0;
}

float_t Hensor::getHumidityCalibration(int index){
	if(index == 0){
		return this->calibration.h_a;
	}
	if(index == 1){
		return this->calibration.h_b;
	}
	return 0;
}

float_t Hensor::getPressureCalibration(int index){
	float_t aux;
	if(index == 0){
		aux = this->calibration.p_a;
		return aux;
	}
	if(index == 1){
		aux = this->calibration.p_b;
		return aux;
	}
	return 0;
}

float_t Hensor::getNH3Calibration(int index){
	float_t aux;
	if(index == 0){
		aux = this->calibration.nh3_m;
		return aux;
	}
	if(index == 1){
		aux = this->calibration.nh3_n;
		return aux;
	}
	if(index == 2){
		aux = this->calibration.nh3_R0;
		return aux;
	}
	if(index == 3){
		aux = this->calibration.nh3_maxV;
		return aux;
	}
	return 0;
}

float_t Hensor::getCO2Calibration(int index){
	float_t aux ;
	if(index == 0){
		aux = this->calibration.co2_a;
		return aux;
	}
	if(index == 1){
		aux = this->calibration.co2_b;
		return aux;
	}
	return 0;
}

void Hensor::holdCO2Value(uint32_t value) {
	if(value < 0){
		value = 0;
	}
	this->currentDatagas.co2 = value;
}

void Hensor::holdNH3Value(uint32_t value) {
	if(value < 0){
		value = 0;
	}
	this->currentDatagas.nh3 = value;
}

void Hensor::holdTemperatureValue(float_t value) {
	this->currentDatagas.temperature = value;
}

void Hensor::holdHumidityValue(float_t value) {
	if(value < 0){
		value = 0;
	}
	else if(value > 100){
		value = 100;
	}
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

bool Hensor::hasSentOnTime(int sendingInterval) {
	static int currentTime;

	// Check what time is it because we must send data no matter what mode
	DateTime now = this->rtc.now();

	// To do it with seconds for BLE or minutes for WiFi
	currentTime = this->inProductionMode ? now.minute() : now.second();

	if (currentTime % sendingInterval != 0) {
		// True because this interval of time is not needed
		return true;
	}

	// Get the previous recorded time
	int sentTime = this->preferences.getInt("sentTime", -1);

	// When the times are the same, means we sent previously
	if (sentTime == currentTime) {
		return true;
	}

	this->preferences.putInt("sentTime", currentTime);
	return false;
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

	int i = 0;
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
	jsonResponse["data"][i]["l1"] = String(currentDatagas.temperature) + " C";
	jsonResponse["data"][i]["l2"] = "";

	i++;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 6;
	jsonResponse["data"][i]["name"] = "Humidity";
	jsonResponse["data"][i]["l1"] = String(abs(currentDatagas.humidity)) + " %";
	jsonResponse["data"][i]["l2"] = "";

	i++;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 6;
	jsonResponse["data"][i]["name"] = "Pressure";
	jsonResponse["data"][i]["l1"] = String(currentDatagas.pressure) + " Pa";
	jsonResponse["data"][i]["l2"] = "";

	//i++;
	//jsonResponse["data"][i]["id"] = i;
	//jsonResponse["data"][i]["s_type"] = 4;
	//jsonResponse["data"][i]["value"] = map(analogRead(15)*3.30/4096,1.98,3.2,0,100);
	//ESP_LOGI(HENSOR_TAG, "Battery-Voltage: %.2f", analogRead(15)*3.30/4096);

	i++;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 6;
	jsonResponse["data"][i]["name"] = "Battery";
	float readed_battery = analogRead(15)*3.30/4096;
	if(readed_battery > 3.10 ) readed_battery = 3.10;
	jsonResponse["data"][i]["l1"] = String((readed_battery - 2.00)*100/1.1) + "%";
	jsonResponse["data"][i]["l2"] = String(analogRead(15)*3.30/4096);
	ESP_LOGI(HENSOR_TAG, "Battery-Voltage: %.2f", analogRead(15)*3.30/4096);

	i++;
	jsonResponse["data"][i]["id"] = i;
	jsonResponse["data"][i]["s_type"] = 5;
	jsonResponse["data"][i]["ssid"] = Network::SSID;
	jsonResponse["data"][i]["ip"] = "";

	serializeJson(jsonResponse, jsonString);
}
