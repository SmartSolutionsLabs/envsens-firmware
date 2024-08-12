#include "NH3sensor.hpp"
#include "Hensor.hpp"

static const char * HENSOR_TAG = "Hensor";

NH3sensor::NH3sensor(const char * name, int taskCore) : Sensor(name, taskCore) {
	this->channelData = 0;
}

void NH3sensor::connect(void * data) {
	this->sensor = new Adafruit_ADS1115();

	this->connectedStatus = this->sensor->begin(ADS1X15_ADDRESS, static_cast<TwoWire*>(data));
}

void NH3sensor::run(void* data) {
	if( !this->connectedStatus ) {
		this->stop();
	}

	this->iterationDelay = 3000 / portTICK_PERIOD_MS;

	Hensor * hensor = Hensor::getInstance();

	int16_t iterationsMeassure = 3;
	while (1) {
		vTaskDelay(this->iterationDelay);
		if(--iterationsMeassure >= 0){
			channelData = this->sensor->readADC_SingleEnded(0);
			// TODO: Here must validate if we get correct data

			voltage += this->sensor->computeVolts(this->channelData);
			continue;
		}
		iterationsMeassure = 3;
		voltage = voltage / 3;
		ESP_LOGI(HENSOR_TAG, "Channel data: %d", this->channelData);
		ESP_LOGI(HENSOR_TAG, "Voltage: %.2f", this->voltage);
		ESP_LOGI(HENSOR_TAG, "NH3-PPM: %d", hensor->FunctionNH3Calibrated(this->voltage));

		hensor->holdNH3Value(hensor->FunctionNH3Calibrated(this->voltage));
		voltage = 0;
	}

	this->stop();
}
