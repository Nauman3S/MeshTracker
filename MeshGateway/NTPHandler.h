
String getTimeStamp();
void setupNTP();
void loopNTP();



#include "time.h"


const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -11 * 60 * 60; //UTC-11=US Timezone
//offset for Niue, American Samoa, Swains Island,
const int daylightOffset_sec = 3600;

void setupNTP()
{
    //Serial.begin(115200);

    // Connect to Wi-Fi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");

    // Init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println(getTimeStamp());

    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

// void loopNTP(){
//   //delay(1000);
//   getTimeStamp();
// }

String getTimeStamp()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        ESP.restart();
        return String("N/A");
    }
    String mils = String(millis());
    mils = mils.substring(0, 3);
    String timeStamp = String(timeinfo.tm_year + 1900) + String("-") + String(timeinfo.tm_mon + 1) + String("-") + String(timeinfo.tm_mday) + String("T") +
                       String(timeinfo.tm_hour) + String(":") + String(timeinfo.tm_min) + String(":") + String(timeinfo.tm_sec) + String(".") + String(mils);
    //Serial.println(timeStamp);
    //Serial.println(ts);
    return timeStamp;
}