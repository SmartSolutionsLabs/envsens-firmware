#ifndef NETWORK_INC
#define NETWORK_INC

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <ElegantOTA.h>
#include <Arduino_ESP32_OTA.h>
#include "root_ca.hpp"

static char const OTA_FILE_LOCATION[] = "https://raw.githubusercontent.com/arduino-libraries/Arduino_ESP32_OTA/main/examples/LOLIN_32_Blink/LOLIN_32_Blink.ino.ota";

#ifndef MAX_ATTEMPTS_QUANTITY
#define MAX_ATTEMPTS_QUANTITY 4
#endif

#define JSON_URL   "https://example.com/myimages/Further-OTA-Examples.json" // this is where you'll post your JSON filter file
#define VERSION    "1.0.0" // The current version of this program

class Network {
	private:
		const char* MyHostName;

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

		void setNetworkHostname(String hostname);

		void checkNewFirmware();
		/**
		 * Call it when change the credentials.
		 */
		void connect();

		// credentials caché
		static String SSID;
		static String PASSWORD;
		
		//String FirmwareVer = { "2.2" };
		//String URL_fw_Version = "https://raw.githubusercontent.com/programmer131/ESP8266_ESP32_SelfUpdate/master/esp32_ota/bin_version.txt";
		//String URL_fw_Bin = "https://raw.githubusercontent.com/programmer131/ESP8266_ESP32_SelfUpdate/master/esp32_ota/fw.bin";

};

#endif
