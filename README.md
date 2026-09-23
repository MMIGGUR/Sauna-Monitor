# Sauna Monitor

ESP32-2432S028 põhine sauna temperatuurimonitor.

## Riistvara

- ESP32-2432S028
- 2.8" ILI9341 TFT ekraan
- Shelly Plus Uni
- DS18B20

## Funktsioonid

- Sauna temperatuuri kuvamine
- NTP kell
- WiFiManager seadistusportaal
- Shelly IP salvestamine ESP32 mällu
- BOOT nupp avab seadistusportaali
- Automaatne andmete värskendamine

## Temperatuuri värvid

- Alla 90°C = punane
- 90°C või rohkem = valge

## WiFi seadistus

BOOT nuppu ~1,5 sekundit all hoides avaneb seadistusportaal.

AP nimi:

SaunMonitor-Setup

Portaalis saab seadistada:

- WiFi võrgu
- WiFi parooli
- Shelly Plus Uni IP aadressi

## Arenduskeskkond

- PlatformIO
- ESP32 Arduino Framework

## Ekraani konfiguratsioon

- Driver: ILI9341_DRIVER
- Rotation: 2
- TFT_RGB_ORDER=TFT_BGR