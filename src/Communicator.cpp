#include "Communicator.hpp"
#include "Hensor.hpp"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

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
    		jsonResponse["tipo_modelo"] = 1;
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
			String dateTime(jsonRequest["time"].as<String>());

			hensor->setTime(dateTime);

			break;
		}
		case 1000: {
			String name = jsonRequest["name"];
			String serialNumber = jsonRequest["serialNumber"];
			String bluetoothName = jsonRequest["bluetoothName"];
			hensor->setDeviceName(name);
			hensor->setDeviceSerialNumber(serialNumber);
			hensor->setBluetoothName(bluetoothName);
			break;
		}
		case 1001: {
			String hostname = jsonRequest["host"];
			String post = jsonRequest["post"];

			hensor->setEndpointHostname(hostname);
			hensor->setEndpointPost(post);

			break;
		}
		case 1002: {
			uint32_t interval = jsonRequest["lap-wifi"];
			uint32_t localinterval = jsonRequest["lap-ble"];
			hensor->setNetworkInterval(interval);
			hensor->setLocalInterval(localinterval);
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
			jsonResponse["name"] = hensor->getDeviceName();
			jsonResponse["serialNumber"] = hensor->getDeviceSerialNumber();
			jsonResponse["interval"] = this->networkInterval;
			jsonResponse["time"] = now.timestamp(DateTime::TIMESTAMP_FULL);
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
	serializeJson(jsonResponse, answer);
	Ble::bleCallback->writeLargeText(Ble::resCharacteristic, answer);
}

void Communicator::sendOut() {
	WiFiClientSecure httpClient;
	httpClient.setInsecure();

	if (!httpClient.connect(this->endpoint.hostname.c_str(), 443)) {
		Serial.print("Failed connection to ");
		Serial.println(this->endpoint.hostname);
		return;
	}
	else {
		Datagas currentDatagas = Hensor::getInstance()->getCurrentDatagas();
		DateTime dateTime(currentDatagas.unixtime);
		JsonDocument jsonResponse;
		jsonResponse["equipo"] = Hensor::getInstance()->getDeviceName();
		jsonResponse["fechahora"] = dateTime.timestamp(DateTime::TIMESTAMP_DATE) + " " + dateTime.timestamp(DateTime::TIMESTAMP_TIME);
		jsonResponse["co2"] = currentDatagas.co2;
		jsonResponse["nh3"] = currentDatagas.nh3;
		jsonResponse["temp"] = currentDatagas.temperature;
		jsonResponse["hr"] = currentDatagas.humidity;
		jsonResponse["pr"] = currentDatagas.pressure;
		float readed_battery = analogRead(15)*3.30/4096;
		if(readed_battery > 3.10 ) readed_battery = 3.10;
		float percentage = (readed_battery - 2.00)*100/1.1;
		jsonResponse["pila"] = percentage ;
		String body;
		serializeJson(jsonResponse, body);

		// Make a HTTP request:
		httpClient.println(String("POST ") + this->endpoint.post.c_str() + " HTTP/1.1");
		httpClient.println(String("Host: ") + this->endpoint.hostname.c_str());
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

		httpClient.stop();
	}
}

void Communicator::addInstruction(String instruction) {
	this->instructionsQueue.enqueue(instruction);
}

void Communicator::run(void * data) {
	static int checkedMinute = 0;
	static int currentMinute;
	static TickType_t delay = 1 / portTICK_PERIOD_MS;
	static Hensor * hensor = Hensor::getInstance();
	static int sendingInterval = hensor->isProductionMode() ? this->networkInterval : this->localInterval;

	this->iterationDelay = 10 / portTICK_PERIOD_MS;

	while (1) {
		vTaskDelay(this->iterationDelay);

		// Process external instructions
		while (!this->instructionsQueue.isEmpty()) {
			vTaskDelay(delay);
			String instruction = this->instructionsQueue.dequeue();
			this->parseIncome(&instruction);
		}

		// Check what time is it because we must send data no matter what mode
		DateTime now = hensor->getRtcNow();

		// To do it with seconds for BLE or minutes for WiFi
		if (hensor->isProductionMode()) {
			currentMinute = now.minute();
		}
		else {
			currentMinute = now.second();
		}

		// Do nothing if we did it before
		if (checkedMinute == currentMinute) {
			continue;
		}

		if (currentMinute % sendingInterval != 0) {
			continue;
		}

		// At passing must change it
		checkedMinute = currentMinute;

		if( !hensor->isProductionMode() ) {
			std::string jsonString;
			hensor->assemblySensorsStatus(jsonString);
			Ble::bleCallback->writeLargeText(Ble::statusCharacteristic, jsonString);
		}
		// If the endpoint is empty we can't send anything
		else if (hensor->isProductionMode() && !hensor->isSendingOut() && this->endpoint.hostname.length() && WiFi.status() == WL_CONNECTED) {
			hensor->setSendingOut(true);
			this->sendOut();
			hensor->setSendingOut(false);
		}
	}
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
