#include "config.h"

// Note: G++ seems to require that these are in the proper order (same as declared).
const Config CONFIG = {
	.wlan = {
		.ssid = "",
		.password = "",
	},
	.mqtt = {
		.mqtt_server = "",
		.mqtt_port = 8883,
		.mqtt_user = "",
		.mqtt_password = ""
	},
	.hostname = "almond.com",
	.password = "froyo",
	.timezone_h = 3,
};
