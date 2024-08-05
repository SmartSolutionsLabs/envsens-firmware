#ifndef NETWORK_INC
#define NETWORK_INC

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

#ifndef MAX_ATTEMPTS_QUANTITY
#define MAX_ATTEMPTS_QUANTITY 4
#endif

class Network {
	private:
		const char* MyHostName = "Galatea G-3";
	protected:
		static Network * network;
		Network();

		static uint32_t remainingAttempts;

	public:
		void operator=(const Network &) = delete;
		static Network * getInstance();
		Network(Network &other) = delete;

		AsyncWebServer server;

		static void onConnected(WiFiEvent_t event, WiFiEventInfo_t info);

		static void onAddressed(WiFiEvent_t event, WiFiEventInfo_t info);

		static void onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

		/**
		 * Call it when change the credentials.
		 */
		void connect();

		// credentials caché
		static String SSID;
		static String PASSWORD;
};

#endif
