# HomiePWM
Three Channel MQTT PWM Controller based on Homie

Two HomieNode classes are provided.  These can also be used on other Homie
projects.

PWMNode.cpp

HomieNode to control a single channel PWM output.

Loosely based on PWM example from Roger Marvin.

'sequence' is advertised and when a new JSON dataset is posted
to <Homie base topic>/<deviceID>/<NodeID>/sequence/set it is parsed
for a sequence of t-v (time-value) pairs.

Each 't' represents the time in milliseconds since the previous event.
The corresponding 'v' represents an ouput level from 0 to 100 that is
reached at the end of this time segment.

Several t-v pairs can be run in sequence, which continuously repeats
from the start.

An example sequence is shown below:

    {"seq":[
        {"t":"5000","v":"0"},
        {"t":"1000","v":"0"},
        {"t":"5000","v":"100"},
        {"t":"1000","v":"100"}
    ]}

This will cause the corresponding output to be off for 5 seconds, then fade to maximum taking 1 second.  After staying at maximum for 5 seconds it will then fade to off taking 1 second.  The whole sequence will take 12 seconds.

sync() is optionally called from a Broadcast Handler.  This is used to
apply a time offset to align the local system clock to a reported time
being broadcast.  A periodic broadcast (e.g. 10 seconds) will keep all
controllers synchronised, so distributed light fades and sequences can be
achieved.

ResetNode.cpp

Monitors the 'reset' property, and when 'true' is published via
<Homie base topic>/<deviceID>/RESET/reset the do_reset flag is set.

isDoReset() is called from the main application and an appropriate
action is taken.

In the case of the HomiePWM application the SPIFFS config file is deleted
forcing a reconfigure on next boot.

CONFIGURATION

A number of #define statements are provided at the top of the main.cpp file.

CHANNEL1, CHANNEL2, and CHANNEL3 cause each node to be included in the build.

CHANNEL1 and CHANNEL2 are discrete outputs using GPIO-0 and GPIO-2 respectively.  CHANNEL3 repurposes the serial RXD line which is found on GPIO-3.

If LOCAL_CONFIG is not defined the application will depend on the local access point being enabled to collect WiFi and MQTT settings.  Once configured the settings are stored in SPIFFS file system and subsequent reboots bypass the configuration state.  By writing to the reset property the SPIFFS file is deleted forcing the configuration state at the following reboot.  It is not practical to use the ResetTrigger by GPIO as all GPIO pins are active outputs.

By setting LOCAL_CONFIG and providing parameters to WIFI_SSID, WIFI_PASSWORD, MQTT_HOST and MQTT_PORT the application will replace the SPIFFS configuration file with one hard-coded with the settings provided.

OTA Programming

Due to the size of the compiled application exceeding 50% of the available flash storage there is insufficient storage to accommodate a new version for OTA reconfiguration.  If this is required the flash memory on a standard ESP-01 can be replaced to provide 4MB.  A Windbond W25Q32BVSIG-25Q32 or equivalent will work as required.
