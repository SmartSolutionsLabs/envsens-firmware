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

	switch(cmd) {
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

			hensor->setDeviceName(name);

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
			uint32_t interval = jsonRequest["lap"];

			hensor->setNetworkInterval(interval);

			break;
		}
		case 2000: {
			String wifiSsid, wifiPass;
			hensor->getWifiCredentials(wifiSsid, wifiPass);
			DateTime now = hensor->getRtcNow();
			jsonResponse["name"] = hensor->getDeviceName();
			jsonResponse["interval"] = this->networkInterval;
			jsonResponse["time"] = now.timestamp(DateTime::TIMESTAMP_FULL);
			jsonResponse["wifi"]["ssid"] = wifiSsid;
			jsonResponse["wifi"]["pass"] = wifiPass;
			jsonResponse["endpoint"]["host"] = this->endpoint.hostname;
			jsonResponse["endpoint"]["post"] = this->endpoint.post;

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

inline void Communicator::sendOut() {
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
		jsonResponse["Equipo"] = Hensor::getInstance()->getDeviceName();
		jsonResponse["FechaHora"] = dateTime.timestamp(DateTime::TIMESTAMP_DATE) + " " + dateTime.timestamp(DateTime::TIMESTAMP_TIME);
		jsonResponse["CO2"] = currentDatagas.co2;
		jsonResponse["NH3"] = currentDatagas.nh3;
		jsonResponse["Temp"] = currentDatagas.temperature;
		jsonResponse["HR"] = currentDatagas.humidity;
		jsonResponse["PR"] = currentDatagas.pressure;
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

	this->iterationDelay = 10 / portTICK_PERIOD_MS;

	while (1) {
		vTaskDelay(this->iterationDelay);

		// Process external instructions
		while (!this->instructionsQueue.isEmpty()) {
			vTaskDelay(delay);
			String instruction = this->instructionsQueue.dequeue();
			this->parseIncome(&instruction);
		}

		// If the endpoint is empty we can't send anything
		if (hensor->isProductionMode() && !hensor->isSendingOut() && this->endpoint.hostname.length() && WiFi.status() == WL_CONNECTED) {
			DateTime now = hensor->getRtcNow();
			currentMinute = now.minute();

			if (checkedMinute != currentMinute && currentMinute % this->networkInterval == 0) {
				hensor->setSendingOut(true);
				checkedMinute = currentMinute;
				this->sendOut();
				hensor->setSendingOut(false);
			}
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
