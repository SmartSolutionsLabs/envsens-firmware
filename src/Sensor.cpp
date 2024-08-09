#include "Sensor.hpp"

Sensor::Sensor(const char * name, int taskCore) : Thread(name, taskCore) {
}

bool Sensor::isConnected() const {
	return this->connectedStatus;
}

void Sensor::testReset() const {
	if (!this->connectedStatus) {
		Serial.print(this->name);
		Serial.print(" sensor unable to connect\n");
		esp_restart();
	}
}
