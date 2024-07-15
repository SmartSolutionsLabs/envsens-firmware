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

void Communicator::run(void * data) {
	while (1) {
		vTaskDelay(this->iterationDelay);

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
