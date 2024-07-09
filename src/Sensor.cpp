#include "Sensor.hpp"

Sensor::Sensor(const char * name, int taskCore) : Thread(name, taskCore) {
}

bool Sensor::isConnected() const {
	return this->connectedStatus;
}
