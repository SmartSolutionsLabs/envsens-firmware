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
	JsonDocument jsonAnswer;
	JsonDocument instruction;
	deserializeJson(instruction, *message);

	jsonAnswer["cmd"] = instruction["cmd"];
	unsigned int cmd = instruction["cmd"];

	switch(cmd) {
		case 7: {
			String ssid = instruction["ssid"];
			String pass = instruction["pass"];

			hensor->setWifiCredentials(ssid, pass);

			break;
		}
		case 1000: {
			String name = instruction["name"];

			hensor->setDeviceName(name);

			break;
		}
		default:
			ESP_LOGI(HENSOR_TAG, "Command non valid");
	}

	// Answer
	std::string answer;
	serializeJson(jsonAnswer, answer);
	Ble::bleCallback->writeLargeText(Ble::resCharacteristic, answer);
}

inline void Communicator::sendOut() {
	static int err;
	err = 0;

	const char * hostname = const_cast<const String&>(this->endpoint.hostname).c_str();
	const char * post = const_cast<const String&>(this->endpoint.hostname).c_str();

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

	while (1) {
		vTaskDelay(this->iterationDelay);

		// Process external instructions
		while (!this->instructionsQueue.isEmpty()) {
			vTaskDelay(delay);
			String instruction = this->instructionsQueue.dequeue();
			this->parseIncome(&instruction);
		}

		// If the endpoint is empty we can't send anything
		if (const_cast<const String&>(this->endpoint.hostname).isEmpty()) {
			continue;
		}

		this->sendOut();
	}
}

void Communicator::setEndpointHostname(String newHostname) {
	String& hostname = const_cast<String&>(this->endpoint.hostname);
	hostname = newHostname;
}

inline const String& Communicator::getEndpointHostname() const {
	return const_cast<const String&>(this->endpoint.hostname);
}

void Communicator::setEndpointPost(String newPost) {
	String& post = const_cast<String&>(this->endpoint.hostname);
	post = newPost;
}

inline const String& Communicator::getEndpointPost() const {
	return const_cast<const String&>(this->endpoint.post);
}
