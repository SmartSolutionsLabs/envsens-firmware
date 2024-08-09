#include "Network.hpp"

String Network::SSID;
String Network::PASSWORD;
uint32_t Network::remainingAttempts;

Network * Network::network = nullptr;

Network * Network::getInstance() {
	if(network == nullptr) {
		network = new Network();
	}

	return network;
}

Network::Network() : server(80) {
	WiFi.mode(WIFI_STA);
	WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  	WiFi.setHostname("EnvSens");

	server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
		request->send(200, "text/plain", "This is the ESP HTTP server.");
	});

	// Start AsyncElegantOTA
	/*ElegantOTA.begin(&server);
	ElegantOTA.setAutoReboot(true);
	ElegantOTA.onStart([]() {
		Serial.print("OTA update started!\n");
	});
	ElegantOTA.onProgress([](size_t current, size_t final) {
		Serial.printf("OTA progress: %u%%\n", (current * 100) / final);
	});
	ElegantOTA.onEnd([](bool success) {
		if(success) {
			Serial.println("OTA update completed successfully.");
		}
		else {
			Serial.println("OTA update failed.");
			// Add failure handling here.
		}
	});
		*/
	server.begin();

	// Delete old configuration
	WiFi.disconnect(true);

	WiFi.onEvent(onConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
	WiFi.onEvent(onAddressed, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
	WiFi.onEvent(onDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

void Network::onConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	Serial.print("WiFi.connected\n");
}

void Network::onAddressed(WiFiEvent_t event, WiFiEventInfo_t info) {
	digitalWrite(NETWORK_STATUS_LED_PIN, HIGH);
	Serial.print("WiFi.addressed: ");
	Serial.println(WiFi.localIP());
}

void Network::onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	digitalWrite(NETWORK_STATUS_LED_PIN, LOW);
	Serial.print("WiFi.disconnected\n");

	if(Network::SSID == "" || Network::PASSWORD == "") {
		Serial.print("No net credentials.");
		return;
	}

	// Reattempt the connection
	if(WiFi.status() != WL_CONNECTED){
		if(Network::remainingAttempts > 0) {
			--Network::remainingAttempts;
			WiFi.reconnect();
			//WiFi.begin(Network::SSID, Network::PASSWORD);
		}
	}
}

void Network::setNetworkHostname(String newhostname){
	WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE); 
  	WiFi.setHostname(newhostname.c_str());
}

void Network::connect() {
	if(Network::SSID == "" || Network::PASSWORD == "") {
		Serial.print("No net credentials.");
		return;
	}

	// Reset the counter
	Network::remainingAttempts = MAX_ATTEMPTS_QUANTITY;

	// Attempt the connection
	WiFi.begin(Network::SSID, Network::PASSWORD);
}

void Network::checkNewFirmware(){
	Arduino_ESP32_OTA ota;
	Arduino_ESP32_OTA::Error ota_err = Arduino_ESP32_OTA::Error::None;

	/* Configure custom Root CA */
	ota.setCACert(root_ca);

	Serial.println("Initializing OTA storage");
	if ((ota_err = ota.begin()) != Arduino_ESP32_OTA::Error::None)
	{
		Serial.print  ("Arduino_ESP_OTA::begin() failed with error code ");
		Serial.println((int)ota_err);
		return;
	}


	Serial.println("Starting download to flash ...");
	int const ota_download = ota.download(OTA_FILE_LOCATION);
	if (ota_download <= 0)
	{
		Serial.print  ("Arduino_ESP_OTA::download failed with error code ");
		Serial.println(ota_download);
		return;
	}
	Serial.print  (ota_download);
	Serial.println(" bytes stored.");


	Serial.println("Verify update integrity and apply ...");
	if ((ota_err = ota.update()) != Arduino_ESP32_OTA::Error::None)
	{
		Serial.print  ("ota.update() failed with error code ");
		Serial.println((int)ota_err);
		return;
	}

	Serial.println("Performing a reset after which the bootloader will start the new firmware.");
	#if defined(ARDUINO_NANO_ESP32)
	Serial.println("Hint: Arduino NANO ESP32 will blink Red Green and Blue.");
	#else
	Serial.println("Hint: LOLIN32 will blink Blue.");
	#endif
	delay(1000); /* Make sure the serial message gets out before the reset. */
	ota.reset();
}