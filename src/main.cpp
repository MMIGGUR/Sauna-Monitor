#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <time.h>

TFT_eSPI tft = TFT_eSPI();
Preferences preferences;
WiFiManager wifiManager;

char shellyIp[16] = "192.168.1.200";
char shellyUrl[80];

WiFiManagerParameter shellyIpParameter(
    "shelly_ip",
    "Shelly Plus Uni IP",
    "192.168.1.200",
    16
);

float temperature = 0.0;

unsigned long previousUpdate = 0;
const unsigned long updateInterval = 30000;

const int setupButtonPin = 0;

void showMessage(const String& message)
{
    tft.fillScreen(0x0000);
    tft.setTextColor(0xFFFF, 0x0000);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(message, 160, 120, 4);
}

void buildShellyUrl()
{
    snprintf(
        shellyUrl,
        sizeof(shellyUrl),
        "http://%s/rpc/Shelly.GetStatus",
        shellyIp
    );
}

bool savePortalSettings()
{
    strlcpy(
        shellyIp,
        shellyIpParameter.getValue(),
        sizeof(shellyIp)
    );

    IPAddress parsedIp;

    if (!parsedIp.fromString(shellyIp))
    {
        showMessage("Shelly IP viga");
        return false;
    }

    preferences.putString("shelly_ip", shellyIp);
    buildShellyUrl();

    return true;
}

void drawScreen()
{
    tft.fillScreen(0x0000);
    tft.setTextDatum(MC_DATUM);

    tft.setTextColor(0xFFFF, 0x0000);
    tft.drawString("SAUN", 160, 35, 4);

    if (temperature < 90.0)
    {
        tft.setTextColor(0x001F, 0x0000);
    }
    else
    {
        tft.setTextColor(0xFFFF, 0x0000);
    }

    tft.drawString(
        String(temperature, 1) + " C",
        160,
        110,
        7
    );

    struct tm timeinfo;

    if (getLocalTime(&timeinfo))
    {
        char clockBuffer[6];

        snprintf(
            clockBuffer,
            sizeof(clockBuffer),
            "%02d:%02d",
            timeinfo.tm_hour,
            timeinfo.tm_min
        );

        tft.setTextColor(0xFFFF, 0x0000);
        tft.drawString(clockBuffer, 160, 205, 4);
    }
}

void readShellyTemperature()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        showMessage("WiFi puudub");
        return;
    }

    HTTPClient http;
    http.begin(shellyUrl);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();

        JsonDocument doc;
        DeserializationError error =
            deserializeJson(doc, payload);

        if (error)
        {
            http.end();
            showMessage("JSON viga");
            return;
        }

        if (doc["temperature:100"]["tC"].isNull())
        {
            http.end();
            showMessage("Anduri viga");
            return;
        }

        temperature =
            doc["temperature:100"]["tC"].as<float>();

        drawScreen();

        Serial.print("Shelly temperatuur: ");
        Serial.print(temperature, 1);
        Serial.println(" C");
    }
    else
    {
        showMessage("Shelly viga");
    }

    http.end();
}

void openSetupPortal()
{
    showMessage("WiFi seadistus");

    bool connected =
        wifiManager.startConfigPortal(
            "SaunMonitor-Setup"
        );

    if (!connected)
    {
        showMessage("WiFi viga");
        delay(3000);
        ESP.restart();
    }

    if (!savePortalSettings())
    {
        delay(3000);
        return;
    }

    showMessage("Salvestatud");
    delay(1000);

    configTzTime(
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "pool.ntp.org",
        "time.google.com"
    );

    readShellyTemperature();
    previousUpdate = millis();
}

void setup()
{
    Serial.begin(115200);

    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);

    pinMode(setupButtonPin, INPUT_PULLUP);

    tft.init();
    tft.setRotation(2);

    showMessage("WiFi...");

    preferences.begin("saunmonitor", false);

    String savedShellyIp =
        preferences.getString(
            "shelly_ip",
            "192.168.1.200"
        );

    strlcpy(
        shellyIp,
        savedShellyIp.c_str(),
        sizeof(shellyIp)
    );

    shellyIpParameter.setValue(
        shellyIp,
        sizeof(shellyIp)
    );

    wifiManager.addParameter(&shellyIpParameter);

    bool connected =
        wifiManager.autoConnect(
            "SaunMonitor-Setup"
        );

    if (!connected)
    {
        showMessage("WiFi viga");
        delay(3000);
        ESP.restart();
    }

    if (!savePortalSettings())
    {
        return;
    }

    configTzTime(
        "EET-2EEST,M3.5.0/3,M10.5.0/4",
        "pool.ntp.org",
        "time.google.com"
    );

    readShellyTemperature();
    previousUpdate = millis();
}

void loop()
{
    if (digitalRead(setupButtonPin) == LOW)
    {
        delay(1500);

        if (digitalRead(setupButtonPin) == LOW)
        {
            openSetupPortal();

            while (digitalRead(setupButtonPin) == LOW)
            {
                delay(50);
            }
        }
    }

    if (millis() - previousUpdate >= updateInterval)
    {
        previousUpdate = millis();
        readShellyTemperature();
    }

    delay(50);
}