#include "Sensor.hpp"

Sensor::Sensor(const char * name, int taskCore) : Thread(name, taskCore) {
}

bool Sensor::isConnected() const {
	return this->connectedStatus;
}

void Sensor::testReset() {
	if (--this->remainingAttempts == 0) {
		//esp_restart();
	}
}

void Sensor::resetRemaining() {
	this->remainingAttempts = 10;
}
