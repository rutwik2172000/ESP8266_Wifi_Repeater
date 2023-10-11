#if LWIP_FEATURES && !LWIP_IPV6

#define HAVE_NETDUMP 0

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <LwipDhcpServer.h>

#define NAPT 1000
#define NAPT_PORT 10

#if HAVE_NETDUMP

#include <NetDump.h>

void dump(int netif_idx, const char* data, size_t len, int out, int success) {
  (void)success;
  Serial.print(out ? F("out ") : F(" in "));
  Serial.printf("%d ", netif_idx);

  // optional filter example: if (netDump_is_ARP(data))
  {
    netDump(Serial, data, len);
    //netDumpHex(Serial, data, len);
  }
}
#endif

// MY FUNCTIONS
void testwifi() {
  Serial.printf("\nTesting connection with '%s'\n", WiFi.SSID().c_str());
  int count = 0;
  digitalWrite(2,LOW);
  while (count < 20) {
    WiFi.mode(WIFI_STA);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\nWiFi Connected! \nSTA: %s (dns: %s / %s)\n\n",
                    WiFi.localIP().toString().c_str(),
                    WiFi.dnsIP(0).toString().c_str(),
                    WiFi.dnsIP(1).toString().c_str());
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAPConfig(  // Set IP Address, Gateway and Subnet
        IPAddress(192, 168, 4, 1),
        IPAddress(192, 168, 4, 1),
        IPAddress(255, 255, 255, 0));
      // dhcpSoftAP.dhcps_set_dns(0, WiFi.dnsIP(0));
      // dhcpSoftAP.dhcps_set_dns(1, WiFi.dnsIP(1));
      WiFi.softAPDhcpServer().setDns(WiFi.dnsIP(0));
      WiFi.softAP(WiFi.softAPSSID(), WiFi.softAPPSK());
      return;
    }
    else {
      Serial.print(".");
      delay(1000);
      count++;
    }
  }
  Serial.printf("\nCan't connect to WiFi, connect to AP '%s' and configure...\n\n", WiFi.softAPSSID());
}

// SERVER
ESP8266WebServer server(80);
void handleRoot() {
  String content;
  content = "<!DOCTYPE html><html lang='en'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  content += "<head><title>ESP8266 Configuration Page</title></head>";
  content += "<body>";
  content += "<h1>Wireless Station Settings</h1>";
  content += "<form action='/applysta' method='post'>";
  content += "<label for='ssid-select'>SSID:</label><br>";
  content += "<select name='stassid' id='ssid-select'>";
  String ssid;
  int32_t rssi;
  uint8_t encryptionType;
  uint8_t *bssid;
  int32_t channel;
  bool hidden;
  int scanResult;
 
  Serial.println(F("Starting WiFi scan..."));
 
  scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);

  if (scanResult == 0) {
    Serial.println(F("No networks found"));
  }
  else if (scanResult > 0) {
    Serial.printf(PSTR("%d networks found:\n"), scanResult);
 
    // Print unsorted scan results
    for (int8_t i = 0; i < scanResult; i++) {
      WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);
      if (ssid == WiFi.SSID() && WiFi.status() == WL_CONNECTED) {
        content += "<option value='" + ssid + "' selected>" +  rssi + "dB | " + ssid + "</option>";
      }
      else {
        content += "<option value='" + ssid + "'>" + rssi + "dB | " + ssid + "</option>";
      }
    }
  }
  else {
    Serial.printf(PSTR("WiFi scan error %d"), scanResult);
  }
  content += "</select><br>";
  content += "<label for='stapsk'>Password:</label><br><input type='password' id='stapsk' name='stapsk' placeholder='********' minlength='8' maxlength='63'><br>";
  content += "<input type='radio' id='perm' name='stasetting' value='true'>";
  content += "<label for='perm'>Save Permanently</label>";
  content += "<input checked type='radio' id='temp' name='stasetting' value='false'>";
  content += "<label for='temp'>Save Until Reboot</label><br><br>";
  content += "<input type='submit' value='Apply'><br>";
  content += "</form>";
  content += "<h1>Access Point Settings</h1>";
  content += "<form action='/applyap' method='post'>";
  content += "<label for='apssid'>SSID:</label><br><input type='text' id='apssid' name='apssid' minlength='2' maxlength='32' value='" + WiFi.softAPSSID() + "'><br>";
  content += "<label for='stapsk'>Password:</label><br><input type='text' id='appsk' name='appsk' minlength='8' maxlength='63' value='" + WiFi.softAPPSK() + "'><br>";
  content += "<input type='radio' id='perm' name='apsetting' value='true'>";
  content += "<label for='perm'>Save Permanently</label>";
  content += "<input checked type='radio' id='temp' name='apsetting' value='false'>";
  content += "<label for='temp'>Save Until Reboot</label><br><br>";
  content += "<input type='submit' value='Apply'><br>";
  content += "</form>";
  content += "<h1>Miscellaneous</h1><br>";
  content += "<form><input type='submit' value='Reboot' formaction='/reboot'></form>";
  content += "</body>";
  content += "</html>";
  server.send(200, "text/html", content);
}
String content;
void serverconfig() {
  server.begin();
  server.on("/", handleRoot);
  server.onNotFound([]() {
    server.send(404, "text/plain", "How the heck did you get here? I'd go back if I were you.");
  });

  server.on("/applysta", []() {
    String stassid = server.arg("stassid");
    String stapsk = server.arg("stapsk");
    server.send(200, "text/plain", "Settings Recieved. You can go back and wait for reconnection.");
    delay(5000);
    (server.arg("stasetting") == "true") ? WiFi.persistent(true) : WiFi.persistent(false);
    Serial.printf("\n\nAttempting to connect to '%s' using password '%s' \n", stassid.c_str(), stapsk.c_str());
    WiFi.begin(stassid, stapsk);
    testwifi();
  });

  server.on("/applyap", []() {
    String apssid = server.arg("apssid");
    String appsk = server.arg("appsk");
    server.send(200, "text/plain", "Settings Recieved. You can go back and wait for reconnection.");
    delay(5000);
    (server.arg("apsetting") == "true") ? WiFi.persistent(true) : WiFi.persistent(false);
    Serial.printf("Creating AP '%s' with password '%s' \n", apssid.c_str(), appsk.c_str());
    WiFi.softAP(apssid, appsk);
  });

  server.on("/reboot", []() {
    server.send(200, "text/plain", "Rebooting now...");
    delay(2000);
    ESP.reset();
  });
}

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  delay(1000);
  Serial.printf("\n\nNAPT Range extender\n");
  Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());

#if HAVE_NETDUMP
  phy_capture = dump;
#endif

  WiFi.setPhyMode(WIFI_PHY_MODE_11N); // Set radio type to N
  WiFi.mode(WIFI_AP_STA);
  WiFi.persistent(false);
 
  if (WiFi.SSID() != NULL) {
    WiFi.begin(); // Use stored credentials to connect to network
    testwifi();
  }
  else {
    WiFi.softAPConfig(  // Set IP Address, Gateway and Subnet
      IPAddress(192, 168, 4, 1),
      IPAddress(192, 168, 4, 1),
      IPAddress(255, 255, 255, 0));
    WiFi.softAP(WiFi.softAPSSID(), WiFi.softAPPSK()); // Create AP with stored credentials
  }

  Serial.printf("Heap before: %d\n", ESP.getFreeHeap());
  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT, (int)ret, (int)ERR_OK);
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) {
      Serial.printf("\nWiFi Network '%s' with Password '%s' and IP '%s' is now setup\n", WiFi.softAPSSID().c_str(), WiFi.softAPPSK().c_str(), WiFi.softAPIP().toString().c_str());
    }
  }
  Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
  if (ret != ERR_OK) {
    Serial.printf("NAPT initialization failed\n");
  }

  serverconfig();
}

#else

void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nNAPT not supported in this configuration\n");
}

#endif

void loop() {
  server.handleClient();
}
