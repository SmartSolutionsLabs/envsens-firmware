#include "CO2sensor.hpp"

CO2sensor::CO2sensor(const char * name, int taskCore) : Sensor(name, taskCore) {
}

void CO2sensor::connect(void * data) {
	this->sensor = new SensirionI2CScd4x();

	this->sensor->begin((TwoWire&)data);
}

void CO2sensor::run(void* data) {
	TickType_t xDelayIteration = 1000 / portTICK_PERIOD_MS; // Iteration speed

	uint16_t error;
	uint16_t co2 = 0;
	float temperature = 0.0f;
	float humidity = 0.0f;
	bool isDataReady = false;

	while (1) {
		vTaskDelay(xDelayIteration);

		error = this->sensor->getDataReadyFlag(isDataReady);

		if (error) {
			Serial.print("Error trying to getDataReadyFlag()\n");
		}

		if (!isDataReady) {
			continue;
		}

		error = this->sensor->readMeasurement(co2, temperature, humidity);
		if (error) {
			Serial.print("Error trying to readMeasurement()\n");
		}
		else if (co2 == 0) {
			Serial.print("Invalid CO2 sample detected, skipping.\n");
			continue;
		}
		else {
			Serial.print("Co2:");
			Serial.println(co2);
		}
	}
}
