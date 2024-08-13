#include "Updater.hpp"

GithubFirmwareUpdater::GithubFirmwareUpdater(void){
    this->initTime = pdTICKS_TO_MS( xTaskGetTickCount() );
}

bool GithubFirmwareUpdater::checkForUpdatesEveryTime(uint32_t milliseconds) {
    this->currentTime = pdTICKS_TO_MS( xTaskGetTickCount() );

    if ((this->currentTime - this->initTime) >= milliseconds){

        this->initTime = this->currentTime;
        if (this->firmwareVersionCheck()) {

            this->firmwareUpdate();
        }
    }
}

void GithubFirmwareUpdater::firmwareUpdate(void) {
    WiFiClientSecure client;
    client.setInsecure();
    httpUpdate.setLedPin(9, LOW);
    t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

    switch(ret){
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;

    }
}

bool GithubFirmwareUpdater::firmwareVersionCheck(void) {
    String payload;
    int httpCode;
    String fwurl = "";
    fwurl += URL_fw_Version;
    fwurl += "?";
    fwurl += String(rand());
    Serial.println(fwurl);
    WiFiClientSecure * client = new WiFiClientSecure;

    if(client){
    //client->setCACert(rootCACertificate);
        client->setInsecure();
        // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
        HTTPClient https;

        if (https.begin( * client, fwurl)){ // HTTPS
            Serial.print("[HTTPS] GET...\n");
            // start connection and send HTTP header
            vTaskDelay(1000);
            httpCode = https.GET();
            vTaskDelay(1000);
            if (httpCode == HTTP_CODE_OK){ // if version received
                payload = https.getString(); // save received version
            }
            else{
                Serial.print("error in downloading version file:");
                Serial.println(httpCode);
            }
            https.end();
        }
        delete client;
    }

    if (httpCode == HTTP_CODE_OK){
        payload.trim();
        Serial.println(payload);
        if (payload.equals(this->firmwareVersion)) {
            Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
            this->newFirmwareVersion = payload;
            return false;
        }
        else
        {
            this->FirmwareVer = payload;
            Serial.println(payload);
            Serial.println("New firmware detected");
            return true;
        }
    }
    return true;
}