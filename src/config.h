#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <type_traits>

template<typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type
{
	return static_cast<typename std::underlying_type<E>::type>(e);
}

struct Config_wlan
{
	const char *ssid;
	const char *password;
};

struct Config_mqtt
{
	const char *mqtt_server;
	int mqtt_port;
	const char *mqtt_user;
	const char *mqtt_password;
};

struct Config
{
	Config_wlan wlan;
	Config_mqtt mqtt;
	const char *hostname;
	const char *password;
	uint8_t timezone_h;
};

extern const Config CONFIG;

#define PIN_LED     15    // board pin 8
#define PIN_PUMP    4    // board pin 2

#endif //CONFIG_H
