# ESP8266_Wifi_Repeater

A full functional WiFi repeater (correctly: a WiFi NAT router)

This is an implementation of a WiFi NAT router on the esp8266 and esp8285. It also includes support for a packet filtering firewall with ACLs, port mapping, traffic shaping, hooks for remote monitoring (or packet sniffing), an MQTT management interface, simple GPIO interaction, and power management. For a setup with multiple routers in a mesh to cover a larger area a new mode "Automesh"

Typical usage scenarios include:

Simple range extender for an existing WiFi network
Battery powered outdoor (mesh) networks
Setting up an additional WiFi network with different SSID/password for guests
Setting up a secure and restricted network for IoT devices
Translating WPA2 Enterprise networks to WPA-PSK
Monitor probe for WiFi traffic analysis
Network experiments with routes, ACLs and traffic shaping
Meshed IoT device with basic I/O capabilities and MQTT control
By default, the ESP acts as STA and as soft-AP and transparently forwards any IP traffic through it. As it uses NAT no routing entries are required neither on the network side nor on the connected stations.

Measurements show, that it can achieve about 5 Mbps in both directions, so even streaming is possible.

Screenshot

![image](https://github.com/rutwik2172000/ESP8266_Wifi_Repeater/assets/106751275/42aee93a-43c9-48b4-ae76-c559c23b2581)
