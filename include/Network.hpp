#ifndef NETWORK_INC
#define NETWORK_INC

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

class Network {
	private:
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
