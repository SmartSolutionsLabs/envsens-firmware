#include "Communicator.hpp"
#include "Hensor.hpp"

#include <ArduinoJson.h>
#include <WiFi.h>

static const char * HENSOR_TAG = "Hensor";

Communicator * Communicator::communicator = nullptr;

Communicator * Communicator::getInstance() {
	if (communicator == nullptr) {
		communicator = new Communicator("comm", 1);
	}

	return communicator;
}

Communicator::Communicator(const char * name, int taskCore) : Thread(name, taskCore) {
	WiFiClient wifiClient;

	this->httpClient = new HttpClient(wifiClient);
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
		default:
			ESP_LOGI(HENSOR_TAG, "Command non valid");
	}

	// Answer
	std::string answer;
	serializeJson(jsonResponse, answer);
	Ble::bleCallback->writeLargeText(Ble::resCharacteristic, answer);
}

inline void Communicator::sendOut() {
	static int err;
	err = 0;

	const char * hostname = this->endpoint.hostname.c_str();
	const char * post = this->endpoint.hostname.c_str();

	this->httpClient->post(hostname, post);
	if (err == 0) {
		err = this->httpClient->responseStatusCode();

		if (err == 200) {
			Serial.print("Got OK status\n");
		}
	}
}

void Communicator::addInstruction(String instruction) {
	this->instructionsQueue.enqueue(instruction);
}

void Communicator::run(void * data) {
	static TickType_t delay = 1 / portTICK_PERIOD_MS;
	static Hensor * hensor = Hensor::getInstance();

	while (1) {
		vTaskDelay(this->iterationDelay);

		// Process external instructions
		while (!this->instructionsQueue.isEmpty()) {
			vTaskDelay(delay);
			String instruction = this->instructionsQueue.dequeue();
			this->parseIncome(&instruction);
		}

		// If the endpoint is empty we can't send anything
		if (hensor->isProductionMode() && this->endpoint.hostname.length() && WiFi.status() == WL_CONNECTED) {
			this->sendOut();
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
	this->endpoint.hostname = newPost;
}

inline const String& Communicator::getEndpointPost() const {
	return this->endpoint.post;
}
