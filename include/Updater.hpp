#include <Arduino.h>
#ifndef INC_HTTP_CLIENT
#define INC_HTTP_CLIENT
    #include <HTTPClient.h>
    #include <HTTPUpdate.h>
    #include <WiFiClientSecure.h>
    #include <ArduinoJson.h>
#endif

#define URL_versions "https://raw.githubusercontent.com/SmartSolutionsLabs/envsens-firmware/updater/manifestURL.json"

class GithubFirmwareUpdater{
    private:
        JsonDocument getPayload;

        String URL_check_Version =
        "https://github.com/SmartSolutionsLabs/envsens-firmware/releases/download/";
        // v1.0.0/updateVersionURL.json";

        String URL_newFirmware_Bin =
        "https://github.com/SmartSolutionsLabs/envsens-firmware/releases/download/";
        //v1.0.0/firmware.bin";

        String firmwareVersion = "0.0.1";

        String newFirmwareVersion;

        TickType_t currentTime;

        TickType_t initTime;

        int num;

    public:

        GithubFirmwareUpdater();

        void firmwareUpdate();

        bool firmwareVersionCheck(void );

        void checkForUpdatesEveryTime(uint32_t milliseconds);

        String getNewFirmwareVersion();

        String getFirmwareVersion();
};
