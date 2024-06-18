#ifndef INC_SENSOR
#define INC_SENSOR

/**
 * Controlling hardware for input of gas.
 */
class Sensor : public Thread {
	public:
		Sensor();

		/**
		 * Here starts.
		 */
		virtual void connect(void * data) = 0;

		void run(void* data);
};

#endif
