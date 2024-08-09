#ifndef NETWORK_INC
#define NETWORK_INC

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <ElegantOTA.h>
//#include <Arduino_ESP32_OTA.h>
#include "root_ca.hpp"
#include <esp32fota.h>

static char const OTA_FILE_LOCATION[] = "https://raw.githubusercontent.com/arduino-libraries/Arduino_ESP32_OTA/main/examples/LOLIN_32_Blink/LOLIN_32_Blink.ino.ota";

#ifndef MAX_ATTEMPTS_QUANTITY
#define MAX_ATTEMPTS_QUANTITY 4
#endif

#define VERSION "0.0.1" // The current version of this program

class Network {
	private:
		const char* MyHostName;
		esp32FOTA esp32FOTA("esp32-fota-http", 1, false);
		const char* manifest_url = "https://raw.githubusercontent.com/SmartSolutionsLabs/envsens-firmware/main/manifestUpdate.json";
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
