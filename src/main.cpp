#include <Homie.hpp>
#include <Homie/Config.hpp>

#define CHANNEL1
#define CHANNEL2
#define CHANNEL3

// no quotes around string values
// #define LOCAL_CONFIG
#define WIFI_SSID ssid
#define WIFI_PASSWORD password
#define MQTT_HOST MQTT Host
#define MQTT_PORT 1883
#define BASE_TOPIC homie/

#include <PWMNode.h>
#include <ResetNode.h>
#include <LoggerNode.h>

#define FW_NAME "HomiePWM"
#define FW_MAJOR "0"
#define COMMIT_COUNTER "2"
#define BUILD_NUMBER "1"

#define FW_VERSION FW_MAJOR "." COMMIT_COUNTER "." BUILD_NUMBER

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */

LoggerNode LN;

ResetNode rst_node("RESET");

// Up to three LED strips on GPIO 0, 2 and 3
#ifdef CHANNEL1
PWMNode leds1("LEDS-1", 0);
#endif

#ifdef CHANNEL2
PWMNode leds2("LEDS-2", 2);
#endif

#ifdef CHANNEL3
PWMNode leds3("LEDS-3", 3);
#endif

void wipeConfig() {
  if (SPIFFS.begin()) {
    SPIFFS.remove("/homie/config.json");
    SPIFFS.end();
  }
}

#ifdef LOCAL_CONFIG
void initConfig() {
  if (SPIFFS.begin()) {
    SPIFFS.remove("/homie/config.json");

    File configFile = SPIFFS.open("/homie/config.json", "w");
    configFile.println("{");
    configFile.println("    \"wifi\": {");
    configFile.println("        \"ssid\": \"WIFI_SSID\",");
    configFile.println("        \"password\": \"WIFI_PASSWORD\"");
    configFile.println("    },");
    configFile.println("    \"mqtt\": {");
    configFile.println("        \"host\": \"MQTT_HOST\",");
    configFile.println("        \"port\": MQTT_PORT,");
    configFile.println("        \"base_topic\": \"BASE_TOPIC\",");
    configFile.println("        \"auth\": false");
    configFile.println("    },");
    configFile.println("    \"name\": \"PWM\",");
    configFile.println("    \"ota\": {");
    configFile.println("        \"enabled\": true");
    configFile.println("    }");
    configFile.println("}");

    configFile.close();

    SPIFFS.end();
  }
}
#endif

bool broadcastHandler(const String &level, const String &value) {
  unsigned long now = millis();

  LN.logf("HomiePWM", LoggerNode::INFO, "Broadcast [%s]:[%s]\n", level.c_str(), value.c_str());

  if (level.compareTo("sync") == 0) {
    uint64_t delta_time = strtoull(value.c_str(), NULL, 10) - now;

#ifdef CHANNEL1
    leds1.sync(delta_time);
#endif

#ifdef CHANNEL2
    leds2.sync(delta_time);
#endif

#ifdef CHANNEL3
    leds3.sync(delta_time);
#endif
  }

  return true;
}

void setup() {
  Homie_setFirmware(FW_NAME, FW_VERSION);

#ifdef CHANNEL3
  // make RXD a GPIO
  pinMode(3,FUNCTION_3);
#else
  Serial.begin(74880);
  Serial.println("Start");
  Serial.flush();
#endif

#ifdef LOCAL_CONFIG
  initConfig();
#endif

  // Homie.disableLedFeedback();
  Homie.disableResetTrigger();

#ifndef CHANNEL3
  Homie.setLoggingPrinter(&Serial);
#endif

  Homie.disableLogging();
  Homie.setBroadcastHandler(broadcastHandler);
  Homie.setup();
}


void loop() {
    Homie.loop();

    if (rst_node.isDoReset()) {
      wipeConfig();
      Homie.setIdle(true);
      Homie.reset();
    }
}
