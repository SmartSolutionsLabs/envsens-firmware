#include "CO2sensor.hpp"
#include "Hensor.hpp"

static const char * HENSOR_TAG = "Hensor";

CO2sensor::CO2sensor(const char * name, int taskCore) : Sensor(name, taskCore) {
}

void CO2sensor::connect(void * data) {
	this->sensor = new SCD4x();

	vTaskDelay(50 / portTICK_PERIOD_MS); // Not usable but time is documented in hardware manual
	this->connectedStatus = this->sensor->begin(static_cast<TwoWire*>(data));
}

void CO2sensor::run(void* data) {
	if( !this->connectedStatus ) {
		this->stop();
	}

	this->iterationDelay = 10000 / portTICK_PERIOD_MS;

	Hensor * hensor = Hensor::getInstance();

	while (1) {
		vTaskDelay(this->iterationDelay);

		if (!this->sensor->readMeasurement()) {
			if (hensor->isProductionMode()) {
				this->testReset();
			}
			Serial.print("CO2 failed to perform reading\n");
			continue;
		}

		ESP_LOGI(HENSOR_TAG, "Temperature CO2 (C): %.2f", this->sensor->getTemperature());
		ESP_LOGI(HENSOR_TAG, "CO2(ppm): %d", hensor->FunctionCO2Calibrated(this->sensor->getCO2()));

		hensor->holdCO2Value(hensor->FunctionCO2Calibrated(this->sensor->getCO2()));
	}

	this->stop();
}
