#include "Communicator.hpp"

#include <WiFi.h>

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
}

inline void Communicator::sendOut() {
	static int err;
	err = 0;

	this->httpClient->post(this->endpoint.hostname.c_str(), this->endpoint.post.c_str());
	if (err == 0) {
		err = this->httpClient->responseStatusCode();

		if (err == 200) {
			Serial.print("Got OK status\n");
		}
	}
}

void Communicator::run(void * data) {
	while (1) {
		vTaskDelay(this->iterationDelay);

		// If the endpoint is empty we can't send anything
		if (this->endpoint.hostname.length() == 0) {
			continue;
		}

		this->sendOut();
	}
}

void Communicator::setEndpointHostname(String hostname) {
	this->endpoint.hostname = hostname;
}

String Communicator::getEndpointHostname() const {
	return this->endpoint.hostname;
}

void Communicator::setEndpointPost(String post) {
	this->endpoint.post = post;
}

String Communicator::getEndpointPost() const {
	return this->endpoint.post;
}
