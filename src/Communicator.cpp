#include "Communicator.hpp"
#include "Hensor.hpp"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "Network.hpp"

static const char * HENSOR_TAG = "Hensor";

Communicator * Communicator::communicator = nullptr;

Communicator * Communicator::getInstance() {
	if (communicator == nullptr) {
		communicator = new Communicator("comm", 1);
	}

	return communicator;
}

Communicator::Communicator(const char * name, int taskCore) : Thread(name, taskCore) {
}

void Communicator::parseIncome(void * data) {
	static Hensor * hensor = Hensor::getInstance();

	String * message = static_cast<String*>(data);
	JsonDocument jsonRequest;
	JsonDocument jsonResponse;
	deserializeJson(jsonRequest, *message);

	jsonResponse["cmd"] = jsonRequest["cmd"];
	unsigned int cmd = jsonRequest["cmd"];
	jsonResponse["success"] = true;
	switch(cmd) {
		case 0: {
			jsonResponse["nro_serie"] = hensor->getDeviceSerialNumber();
			jsonResponse["tipo_modelo"] = hensor->getType();
			Serial.println("case 0 , asked");
			break;
		}
		case 1: {
			jsonResponse["version"] = 1.0f;
			jsonResponse["relays"] = 1;
			break;
		}
		case 2: {
			jsonResponse["cal"] = true;  //envia estado de calibracion del relay
			jsonResponse["id"] = 1;
			jsonResponse["name"] = "test";
			jsonResponse["vol"] = 100;  // litros
			jsonResponse["h_dia"] = 6;
			JsonArray scheduleArray = jsonResponse["horarios"].to<JsonArray>();
			int schedule[7]= {1,0,0,0,0,0,1};
			for(int j=0; j<7;j++){
				JsonArray hoursArray = scheduleArray.add<JsonArray>();
				copyArray(schedule, hoursArray);
			}
			break;
		}
		case 6: {
			break;
		}
		case 7: {
			String ssid = jsonRequest["ssid"];
			String pass = jsonRequest["pass"];

			hensor->setWifiCredentials(ssid, pass);

			break;
		}
		case 9: {
			const char* dateTime = jsonRequest["time"];

			if (dateTime) {
				hensor->setTime(String(dateTime));
			}

			break;
		}
		case 10:{
			hensor->setProductionMode( !hensor->isProductionMode() ); // toggle
			esp_restart(); // Everything is ended
			break;
		}
		case 1000: {
			if (jsonRequest.containsKey("type")) {
				hensor->setType(jsonRequest["type"].as<uint8_t>());
			}

			const char* name = jsonRequest["name"];
			if (name) {
				hensor->setDeviceName(String(name));
				hensor->setNetworkHostname(String(name));
			}

			const char* serialNumber = jsonRequest["serialNumber"];
			if (serialNumber) {
				hensor->setDeviceSerialNumber(String(serialNumber));
			}

			const char* bluetoothName = jsonRequest["bluetoothName"];
			if (bluetoothName) {
				hensor->setBluetoothName(String(bluetoothName));
			}

			break;
		}
		case 1001: {
			const char* hostname = jsonRequest["host"];
			const char* post = jsonRequest["post"];

			if (hostname) {
				hensor->setEndpointHostname(String(hostname));
			}
			if (post) {
				hensor->setEndpointPost(String(post));
			}

			break;
		}
		case 1002: {
			if (jsonRequest.containsKey("lap-wifi")) {
				hensor->setNetworkInterval(jsonRequest["lap-wifi"].as<uint32_t>());
			}
			if (jsonRequest.containsKey("lap-ble")) {
				hensor->setLocalInterval(jsonRequest["lap-ble"].as<uint32_t>());
			}

			break;
		}
		case 1003: {
			float_t CO2_a = jsonRequest["CO2_a"];
			float_t CO2_b = jsonRequest["CO2_b"];
			float_t NH3_m = jsonRequest["NH3_m"];
			float_t NH3_n = jsonRequest["NH3_n"];
			uint32_t NH3_R0 = jsonRequest["NH3_R0"];
			float_t NH3_maxV= jsonRequest["NH3_maxV"];
			float_t T_a = jsonRequest["T_a"];
			float_t T_b = jsonRequest["T_b"];
			float_t H_a = jsonRequest["H_a"];
			float_t H_b = jsonRequest["H_b"];
			float_t P_a = jsonRequest["P_a"];
			float_t P_b = jsonRequest["P_b"];

			if (CO2_a) {
				hensor->setCO2Calibration(CO2_a, CO2_b, true);
			}
			if (NH3_m) {
				hensor->setNH3Calibration(NH3_m, NH3_n, NH3_R0, NH3_maxV, true);
			}
			if (T_a) {
				hensor->setTemperatureCalibration(T_a, T_b, true);
			}
			if (H_a) {
				hensor->setHumidityCalibration(H_a, H_b, true);
			}
			if (P_a) {
				hensor->setPressureCalibration(P_a, P_b, true);
			}

			break;
		}
		case 2000: {
			String wifiSsid, wifiPass;
			hensor->getWifiCredentials(wifiSsid, wifiPass);
			DateTime now = hensor->getRtcNow();
			jsonResponse["device"]["type"] = hensor->getType();
			jsonResponse["device"]["name"] = hensor->getDeviceName();
			jsonResponse["device"]["serialNumber"] = hensor->getDeviceSerialNumber();
			jsonResponse["device"]["bluetoothName"] = hensor->getBluetoothName();
			jsonResponse["device"]["time"] = now.timestamp(DateTime::TIMESTAMP_FULL);
			jsonResponse["interval"] = this->networkInterval;
			jsonResponse["wifi"]["ssid"] = wifiSsid;
			jsonResponse["wifi"]["pass"] = wifiPass;
			jsonResponse["endpoint"]["host"] = this->endpoint.hostname;
			jsonResponse["endpoint"]["post"] = this->endpoint.post;

			jsonResponse["CO2_a"] = hensor->getCO2Calibration(0);
			jsonResponse["CO2_b"] = hensor->getCO2Calibration(1);
			jsonResponse["NH3_m"] =    hensor->getNH3Calibration(0);
			jsonResponse["NH3_n"] =    hensor->getNH3Calibration(1);
			jsonResponse["NH3_R0"] =   hensor->getNH3Calibration(2);
			jsonResponse["NH3_maxV"] = hensor->getNH3Calibration(3);
			jsonResponse["T_a"] = hensor->getTemperatureCalibration(0);
			jsonResponse["T_b"] = hensor->getTemperatureCalibration(1);
			jsonResponse["H_a"] = hensor->getHumidityCalibration(0);
			jsonResponse["H_b"] = hensor->getHumidityCalibration(1);
			jsonResponse["P_a"] = hensor->getPressureCalibration(0);
			jsonResponse["P_b"] = hensor->getPressureCalibration(1);
			break;
		}
		default:
			ESP_LOGI(HENSOR_TAG, "Command non valid");
	}

	// Answer
	std::string answer;
	serializeJsonPretty(jsonResponse, answer);
	Ble::bleCallback->writeLargeText(Ble::resCharacteristic, answer);
}

bool Communicator::sendOut(String& body, String& hostname, String& path) {
	if (WiFi.status() != WL_CONNECTED) {
		return false;
	}

	WiFiClientSecure httpClient;
	httpClient.setInsecure();
	if (!httpClient.connect(hostname.c_str(), 443)) {
		Serial.print("Failed connection to ");
		Serial.println(hostname);

		httpClient.stop();
		return false;
	}
	else {
		// Make a HTTP request:
		httpClient.println(String("POST ") + path + " HTTP/1.1");
		httpClient.println(String("Host: ") + hostname);
		httpClient.println("Connection: close");
		httpClient.println("Content-Type: application/json");
		httpClient.print("Content-Length: ");
		httpClient.println(body.length());
		httpClient.println();
		httpClient.println(body);

		while (httpClient.connected()) {
			String line = httpClient.readStringUntil('\n');
			if (line == "\r") {
				break;
			}
		}
		// if there are incoming bytes available
		// from the server, read them and print them:
		while (httpClient.available()) {
			char c = httpClient.read();
			Serial.write(c);
		}
		Serial.write('\n');

		httpClient.stop();

		return true;
	}
}

void Communicator::addInstruction(String instruction) {
	this->instructionsQueue.enqueue(instruction);
}

void Communicator::run(void * data) {
	static TickType_t delay = 1 / portTICK_PERIOD_MS;
	static Hensor * hensor = Hensor::getInstance();
	static int sendingInterval = hensor->isProductionMode() ? this->networkInterval : this->localInterval;

	// A little delay
	vTaskDelay(1000 / portTICK_PERIOD_MS);

	this->iterationDelay = 10 / portTICK_PERIOD_MS;

	while (1) {
		vTaskDelay(this->iterationDelay);

		// Process external instructions
		while (!this->instructionsQueue.isEmpty()) {
			vTaskDelay(delay);
			String instruction = this->instructionsQueue.dequeue();
			this->parseIncome(&instruction);
		}

		if (hensor->hasSentOnTime(sendingInterval)) {
			continue;
		}

		if( !hensor->isProductionMode() ) {
			std::string jsonString;
			hensor->assemblySensorsStatus(jsonString);
			Ble::bleCallback->writeLargeText(Ble::statusCharacteristic, jsonString);
		}
		// Here is for production mode
		// if the endpoint is empty we can't send anything
		// if there is WiFi connection
		else if (!hensor->isSendingOut() && this->endpoint.hostname.length()) {
			hensor->setSendingOut(true);

			// Catch here so more accurate
			Datagas currentDatagas = Hensor::getInstance()->getCurrentDatagas();
			if (hensor->getType() == NODE_MASTER_TYPE && currentDatagas.co2 == 0) {
				ESP_LOGI(HENSOR_TAG, "CO2 value is probably reading 0");
				hensor->setSendingOut(false);
				continue;
			}

			String serialization;
			hensor->serializeDatagas(serialization, currentDatagas);

			if (!this->sendOut(serialization, this->endpoint.hostname, this->endpoint.post)) {
				// Because it was not sent
				Datalogger::getInstance()->saveLocalStorageRow(currentDatagas);

				hensor->setSendingOut(false);
				continue;
			}

			// Try while there is connection
			uint8_t lastIndex = Datalogger::getInstance()->getLastLocalStorageIndex();
			while (lastIndex) {
				currentDatagas = Datalogger::getInstance()->readLocalStorageRow(lastIndex);
				ESP_LOGI(HENSOR_TAG, "Index selected to send: %d; unixtime: %d", lastIndex, currentDatagas.unixtime);

				hensor->serializeDatagas(serialization, currentDatagas); // serializing again because is another datagas
				if (!this->sendOut(serialization, this->endpoint.hostname, this->endpoint.post)) {
					// Don't try because we didn't reach server
					break;
				}

				Datalogger::getInstance()->cleanLocalStorageRow(lastIndex);

				// Decrement for the next row
				--lastIndex;
			}

			hensor->setSendingOut(false);
		}
	}

	// Reset because there is a bug
	Serial.print("Task Communicator ended!\n");
	esp_restart();
}

void Communicator::setEndpointHostname(String newHostname) {
	this->endpoint.hostname = newHostname;
}

inline const String& Communicator::getEndpointHostname() const {
	return this->endpoint.hostname;
}

void Communicator::setEndpointPost(String newPost) {
	this->endpoint.post = newPost;
}

inline const String& Communicator::getEndpointPost() const {
	return this->endpoint.post;
}

void Communicator::setNetworkInterval(uint32_t minutes) {
	this->networkInterval = minutes;
}

void Communicator::setLocalInterval(uint32_t time) {
	this->localInterval = time;
}
