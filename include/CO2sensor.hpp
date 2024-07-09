#ifndef INC_CO2SENSOR
#define INC_CO2SENSOR

#include "Sensor.hpp"

class CO2sensor : public Sensor {
	public:
		CO2sensor(const char * name, int taskCore = 1);

		void connect(void * data);

		void run(void* data);
};

#endif
