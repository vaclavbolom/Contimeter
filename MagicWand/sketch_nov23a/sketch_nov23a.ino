#include "WiFi.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Nastavení Wi-Fi do režimu stanice (STA) a odpojení od případné sítě
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Inicializace skenování Wi-Fi...");
}

void loop() {
  Serial.println("Skenování sítí...");
  
  // Spuštění skenování (WiFi.scanNetworks vrátí počet nalezených sítí)
  int n = WiFi.scanNetworks();
  Serial.println("Skenování dokončeno.");

  if (n == 0) {
    Serial.println("Nebyly nalezeny žádné Wi-Fi sítě.");
  } else {
    Serial.print("Nalezeno sítí: ");
    Serial.println(n);
    
    for (int i = 0; i < n; ++i) {
      // Výpis SSID, síly signálu (RSSI) a typu šifrování
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm) [");
      Serial.print((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Otevřená" : "Zabezpečená");
      Serial.println("]");
      delay(10);
    }
  }
  
  Serial.println("");

  // Pauza 5 sekund před dalším skenováním
  delay(5000);
}