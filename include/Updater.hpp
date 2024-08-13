#include <Arduino.h>
#ifndef INC_WIFI
#define INC_WIFI
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include <HTTPClient.h>
    #include <HTTPUpdate.h>
    #include <WiFiClientSecure.h>
#endif


#define URL_fw_Version "https://github.com/SmartSolutionsLabs/envsens-firmware/releases/download/v1.0.0/manifestURL.json"
#define URL_fw_Bin "https://github.com/SmartSolutionsLabs/envsens-firmware/releases/download/v1.0.0/firmware.bin"

class GithubFirmwareUpdater{
    private:
        String firmwareVersion = "0.0.1";

        String newFirmwareVersion = "";

        TickType_t currentTime;

        TickType_t initTime;

        int num;

    public:

        GithubFirmwareUpdater();

        void firmwareUpdate();

        bool firmwareVersionCheck(void );

        bool checkForUpdatesEveryTime(uint32_t milliseconds);

        String getNewFirmwareVersion();

        String getFirmwareVersion();
};
