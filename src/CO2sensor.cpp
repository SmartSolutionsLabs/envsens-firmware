#include "CO2sensor.hpp"

CO2sensor::CO2sensor(const char * name, int taskCore) : Sensor(name, taskCore) {
}

void CO2sensor::connect(void * data) {
	this->sensor = new SCD4x();

	this->connectedStatus = this->sensor->begin(static_cast<TwoWire*>(data));
}

void CO2sensor::run(void* data) {
	if( !this->connectedStatus ) {
		this->stop();
	}

	this->iterationDelay = 6000 / portTICK_PERIOD_MS;

	while (1) {
		vTaskDelay(this->iterationDelay);

		if (!this->sensor->readMeasurement()) {
			Serial.print("Failed to perform reading\n");
			continue;
		}

		Serial.print("CO2(ppm): ");
		Serial.println(this->sensor->getCO2());

		Serial.print("Temperature(C): ");
		Serial.println(this->sensor->getTemperature(), 1);
	}

	this->stop();
}
