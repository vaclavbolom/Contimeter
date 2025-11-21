![ESP32-C3 OLED kit a měření teploty, vlhkosti a koncentrace CO2](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/10.jpg)

# ESP32-C3_OLED_kit

Měření teploty, vlhkosti tlaku nebo CO2 za pár korun do domácnosti nebo kanceláře. V této jedné malé krabičce je vše precizně připraveno pro osazení displeje, čidla i řídící desky. </br>
Celou měřící stanici obsluhuje [LaskaKit ESP32-C3-LPKit](https://www.laskakit.cz/laskkit-esp-12-board/?variantId=10482). Na této malé desce je čip ESP32-C3 podporující Wi-Fi a Bluetooth připojení, možnost připojení baterie, I2C čidla díky [našemu uŠup konektoru](https://blog.laskakit.cz/predstavujeme-univerzalni-konektor-pro-propojeni-modulu-a-cidel-%ce%bcsup/). Navíc, deska samotná má možnost osadit dvojici hřebínků na které jsou vyvedeny další GPIO piny a sběrnice ve stejném pinoutu jako je má Wemos D1 Mini. </br>
Není tak problém použít jeden z [mnoha rozšiřujících desek - shieldů](https://www.laskakit.cz/esp-a-wemos/). 

Zpět ale k tomuto kitu - [LaskaKit ESP32-C3 OLED](https://www.laskakit.cz/laskkit-esp-12-board/?variantId=10482) s použitým čidlem [LaskaKit SCD41 pro měření teploty, vlhkosti a koncentrace CO2](https://www.laskakit.cz/laskakit-scd41-senzor-co2--teploty-a-vlhkosti-vzduchu/). </br>
Osadit ale můžeš i jiná čidla. Pokud chceš měřit tlak místo koncentrace CO2 a teplotu a vlhkost, pak použij náš modul [LaskaKit BME280 Senzor tlaku, teploty a vlhkosti vzduchu](https://www.laskakit.cz/arduino-senzor-tlaku--teploty-a-vlhkosti-bme280/), pokud ti stačí jenom teplota a vlhkost, pak můžeš použít levnější modul [LaskaKit SHT40 Senzor teploty a vlhkosti vzduchu](https://www.laskakit.cz/laskakit-sht40-senzor-teploty-a-vlhkosti-vzduchu/).

V tomto článku se dozvíš, jak to všechno jednoduše poskládat a ukážeme ti jak to celé rozfungovat během pár minut. 

![ESP32-C3 OLED kit a měření teploty, vlhkosti a koncentrace CO2](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/1.1.jpg)

## Seznam dílů
Základem jsou je LaskaKit ESP32-C3 OLED kit, který obsahuje:</br>
1x  [LaskaKit ESP32-C3-LPKit](https://www.laskakit.cz/laskkit-esp-12-board/?variantId=10482) </br>
1x  [LaskaKit SCD41 Senzor CO2, teploty a vlhkosti vzduchu](https://www.laskakit.cz/laskakit-scd41-senzor-co2--teploty-a-vlhkosti-vzduchu/) nebo [LaskaKit BME280 Senzor tlaku, teploty a vlhkosti vzduchu](https://www.laskakit.cz/arduino-senzor-tlaku--teploty-a-vlhkosti-bme280/) nebo [LaskaKit SHT40 Senzor teploty a vlhkosti vzduchu](https://www.laskakit.cz/laskakit-sht40-senzor-teploty-a-vlhkosti-vzduchu/)</br>
1x  [LaskaKit OLED displej 128x64 1.3" I²C](https://www.laskakit.cz/laskakit-oled-displej-128x64-1-3--i2c/?variantId=11903) </br>
1x  [μŠup, STEMMA QT, Qwiic JST-SH 4-pin kabel - 5cm](https://www.laskakit.cz/--sup--stemma-qt--qwiic-jst-sh-4-pin-kabel-5cm/) </br>
1x  [μŠup, STEMMA QT, Qwiic JST-SH 4-pin kabel - 10cm](https://www.laskakit.cz/--sup--stemma-qt--qwiic-jst-sh-4-pin-kabel-10cm/) </br>
10x [Šroub do plastu s půlkulatou hlavou a kříž. drážkou 2,2x5](https://www.laskakit.cz/sroub-do-termoplastu-s-cockovou-hlavou-a-krizovou-drazkou-2-2x5-bn-82428-ocel/)</br>
6x [Šroub do plastu s půlkulatou hlavou a kříž. drážkou 2,2x8](https://www.laskakit.cz/sroub-do-plastu-s-pulkulatou-hlavou-a-kriz--drazkou-2-2x8-bn-82428-zb/)</br>
1x Krabička pro čidlo SCD41 (CO2, teplota, vlhkost) nebo BME280 (tlak, teplota, vlhkost) / SHT41 (teplota a vlhkost).</br>

## Programování
Než složíme celé zařízení dohromady, naprogramujeme řídící desku. Program, [který najdeš tady](https://github.com/LaskaKit/ESP32-C3_OLED_kit/tree/main/SW/SH1106_SCD41_ESP32-C3-LPKit), dělá to, že změří CO2, teplotu, vlhkost (nebo tlak, teplotu, vlhkost) a zobrazí ji na displeji a poté se zařízení uspí. 
Zasuň USB-C konektor do ESP32-C3-LPkit a poté do počítače, kde sis už nainstaloval [Arduino IDE](https://www.arduino.cc/en/software).

Nainstalujeme knihovnu podporující čipy ESP32 v Arduino IDE. 
Z https://github.com/espressif/arduino-esp32, vlož tento odkaz https://espressif.github.io/arduino-esp32/package_esp32_index.json

![ESP32-C3 OLED kit a přidání knihovny](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/ArduinoIDE_1.jpg)

![ESP32-C3 OLED kit a přidání knihovny](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/ArduinoIDE_2.jpg)

![ESP32-C3 OLED kit a přidání knihovny](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/ArduinoIDE_3.jpg)

Instalace podpory OLED 

![ESP32-C3 OLED kit a podpora OLED](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/ArduinoIDE_4.jpg)

Instalace knihovny SCD41

![ESP32-C3 OLED kit a podpora SCD41](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/ArduinoIDE_5.jpg)

a/nebo BME280

![ESP32-C3 OLED kit a podpora BME280](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/ArduinoIDE_5.1.jpg)

a/nebo SHT41

![ESP32-C3 OLED kit a podpora SHT41](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/ArduinoIDE_5.2.jpg)

Vybereme správnou desku

![ESP32-C3 OLED kit a výběr desky](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/ArduinoIDE_6.jpg)

A Port (ten se objeví po zasunutí USB-C kabelu do počítače)

![ESP32-C3 OLED kit a výběr portu](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/ArduinoIDE_7.jpg)

Zkopírujeme kód z https://github.com/LaskaKit/ESP32-C3_OLED_kit/tree/main/SW a naprogramuješ

![ESP32-C3 OLED kit a nahrání programu do desky](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/ArduinoIDE_8.jpg)

A je hotovo :-) 

## ESP32-C3 OLED kit a přišroubování držáku čidla 
![ESP32-C3 OLED kit a přišroubování držáku čidla](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/2.1.jpg)

Nejprve přišroubuj držák čidla do krabičky. Použij šrouby 2.2x8mm. Tento držák je buď určen pro čidlo [BME280 (tlak, teplota, vlhkost)](https://www.laskakit.cz/arduino-senzor-tlaku--teploty-a-vlhkosti-bme280/) nebo [SCD41 (CO2, teplota, vlhkost)](https://www.laskakit.cz/laskakit-scd41-senzor-co2--teploty-a-vlhkosti-vzduchu/)

## ESP32-C3 OLED kit a propojení uŠup kabelů s jednotlivými moduly
![ESP32-C3 OLED kit a propojení uŠup kabelů s jednotlivými moduly](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/3.1.jpg)

Nyní propojíme jednotlivé moduly. Mezi ESP32-C3-LPkit a SCD41 doporučuji použít 5cm uŠup kabel, a mezi OLED a SCD41 pak použít 10cm uŠup kabel.

## ESP32-C3 OLED kit a připevnění modulů
![ESP32-C3 OLED kit a připevnění modulů](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/5.1.jpg)

Přišroubuj OLED a čidlo ke krabičce. Využij šrouby o velikosti 2.2x5mm. 

## ESP32-C3 OLED kit a připevnění ESP32-C3-LPkit
![ESP32-C3 OLED kit a připevnění ESP32-C3-LPkit](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/6.1.jpg)

Hlavní řídící desku jenom zasuneš do předem vytištěných drážek. Zasunutí je jednoduché a díky správné pozici desky vůči krabičce, USB-C konektor bude směřovat do do díry v zadní straně krabičky. Takto se bude deska programovat a napájet. 

## ESP32-C3 OLED kit a kompletace
![ESP32-C3 OLED kit a smontování](https://github.com/LaskaKit/ESP32-C3_OLED_kit/blob/main/img/9.1.jpg)

Nakonec sešroubuješ obě části krabičky k sobě. A to je všechno :-) 
