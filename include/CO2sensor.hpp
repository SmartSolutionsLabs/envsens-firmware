#ifndef INC_CO2SENSOR
#define INC_CO2SENSOR

#include "Sensor.hpp"

#include <SparkFun_SCD4x_Arduino_Library.h>

class CO2sensor : public Sensor {
	protected:
		SCD4x * sensor = nullptr;

	public:
		CO2sensor(const char * name, int taskCore = 1);

		void connect(void * data) override;

		void run(void* data) override;
};

#endif
