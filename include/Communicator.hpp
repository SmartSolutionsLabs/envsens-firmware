#ifndef INC_COMMUNICATOR
#define INC_COMMUNICATOR

#include "Thread.hpp"

/**
 * Process every income and outcome.
 * It's a singleton transceiver.
 */
class Communicator : public Thread {
	protected:
		static Communicator * communicator;
		Communicator();
		Communicator(const char * name);
		Communicator(const char * name, int taskCore);

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
		inline void sendOut();

		/**
		 * Checks if there are new logged data to send them out.
		 */
		void run(void * data) override;
};

#endif
