#ifndef INC_COMMUNICATOR
#define INC_COMMUNICATOR

#include "Datalogger.hpp"
#include "Thread.hpp"
#include "Ble.hpp"

#include <ArduinoQueue.h>
#include <HTTPUpdate.h>

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

		ArduinoQueue<String> instructionsQueue;

	private:
		/**
		 * Where is the endpoint.
		 */
		struct {
			String hostname;
			String post;
		} endpoint;

		/**
		 * Lap to send data out.
		 */
		uint32_t networkInterval;

		uint32_t localInterval;

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
		bool sendOut(String& body, String& hostname, String& path, String& response);

		/**
		 * Append an instruction to queue for processing in the thread.
		 */
		void addInstruction(String);

		/**
		 * Checks if there are new logged data to send them out.
		 */
		void run(void * data) override;

		void setEndpointHostname(String hostname);
		inline const String& getEndpointHostname() const;
		void setEndpointPost(String post);
		inline const String& getEndpointPost() const;

		void setNetworkInterval(uint32_t minutes);
		void setLocalInterval(uint32_t time);

		void updateFirmware() const;
};

#endif
