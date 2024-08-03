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

uint32_t  NH3sensor::getPPM(float voltage){
	this->R0 = 98400;
	this->maxVoltage = 5.193;
	this->RS = (R0*voltage)/(maxVoltage - voltage);
	this->nh3_read_value = (voltage * this->R0) / (maxVoltage - voltage);
	this->SR = 5;
	this->nh3_in_ppm = 0;
	
	this->nh3_in_ppm = pow(10, -1.8 * log(nh3_read_value/R0)/log(10) - 0.163);
	return this->nh3_in_ppm;
}

void NH3sensor::run(void* data) {
	if( !this->connectedStatus ) {
		this->stop();
	}

	Hensor * hensor = Hensor::getInstance();

	int16_t iterationsMeassure = 3;
	while (1) {
		vTaskDelay(this->iterationDelay);
		if(--iterationsMeassure >= 0){
			channelData = this->sensor->readADC_SingleEnded(0);
			voltage += this->sensor->computeVolts(this->channelData);
			continue;
		} 
		iterationsMeassure = 3;
		voltage = voltage / 3;
		ESP_LOGI(HENSOR_TAG, "Channel data: %d", this->channelData);
		ESP_LOGI(HENSOR_TAG, "Voltage: %.2f", this->voltage);
		ESP_LOGI(HENSOR_TAG, "NH3-PPM: %d", this->getPPM(this->voltage));

		hensor->holdNH3Value(this->getPPM(this->voltage) * hensor->getNH3Multiplier());
		voltage = 0;
	}

	this->stop();
}
