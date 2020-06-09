<div align="center">

# Secure MQTT Connection

</div>

<div align="center">

    secure mqtt esp8266 hardware control code for any system

  [![Almond](../public/img/readme.svg)](https://almond.com)

  #### Simple but complicated

</div>

## Disclaimer
The following documentation in no way guarantees a secure system.
*See [LICENSE](/LICENSE).*

## Features
- MQTT Publish/Subscribe Messaging secured using TLS 1.2
- Secure MQTT Server setup
- Secure ESP8266 MQTT Client
- AVR SPI Slave

## Design 

### Secure MQTT Broker
The MQTT Broker named Mosquitto is installed on the digital ocean server. However, the MQTT Broker could be run on any other computer on the same local area network (LAN). You could even run it on a Raspberry PI if performance isn’t a strict requirement.
<br>[MQTT Broker secure setup - detail](/mqtt-server-setup#mqtt-broker-secure-setup)

### Secure ESP8266 MQTT Client
Again, there are many tutorials on how to program the ESP8266 as an MQTT Client. But it's hard to find a single source clearly describing how to program the ESP8266 as a **secure** MQTT Client. It is important to know security as a software engineer these days.
<br>[Secure ESP8266 MQTT Client - detail](#main)

### AVR SPI Slave
The original plan was to connect peripherals directly to the ESP8266 data pins but I ran out of pins very quickly. So I used an ATmega32 and connected the two chips via SPI with the ESP8266 as the master and the ATmega as the slave. In additions to giving me a lot more data pins this scheme also gave me 5v tolerance.
<br>[AVR SPI Slave - detail](/spi-slave-avr#avr-spi-slave)

### ESP 8266 Datasheet
[Datasheet Download](https://circuits4you.com/wp-content/uploads/2018/12/esp32-wroom-32_datasheet_en.pdf)

### ESP 8266 Pinout
<br />
<img width="1440" alt="almond-screenshot" src="../public/img/ESP8266_12X.jpg">
<br />

## FAQ
See the almond [wiki](https://github.com/mashafrancis/almond-hw/wiki)
