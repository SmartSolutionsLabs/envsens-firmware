#ifndef INC_NH3SENSOR
#define INC_NH3SENSOR

#include "Sensor.hpp"

class NH3sensor : public Sensor {
	public:
		NH3sensor(const char * name, int taskCore = 1);

		void connect(void * data) override;

		void run(void* data) override;
};

#endif
