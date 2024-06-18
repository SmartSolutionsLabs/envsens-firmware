#ifndef INC_COMMUNICATOR
#define INC_COMMUNICATOR

/**
 * Process every income and outcome.
 * It's a singleton transceiver.
 */
class Communicator : public Thread {
	protected:
		static Communicator * motor;
		Communicator();
		Communicator(const char * name);

	public:
		// For singleton
		static Communicator * getInstance();
		Communicator(Communicator &other) = delete;
		void operator=(const Communicator &) = delete;

		/**
		 * All external instrunctions are processed here.
		 */
		void parseIncome(void * data);

		/**
		 * Sends data to exterior.
		 */
		void sendOut();
};

#endif
