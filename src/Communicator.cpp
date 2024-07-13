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
}

void Communicator::run(void * data) {
	while (1) {
		vTaskDelay(this->iterationDelay);

		this->sendOut();
	}
}
