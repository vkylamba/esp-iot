
### LilyGo T-Call SIM800 board:
https://github.com/Xinyuan-LilyGO/LilyGo-T-Call-SIM800

### AzEnvy board
https://www.az-delivery.de/en/products/az-envy

- Connecting to Thingsboard and sending telemetry battery/rssi/bootcount via MQTT
- Sleeping ESP32 & SIM800 (~1mA consumption)


### Upload to board:
platformio run --upload-port /dev/tty.usbserial-AO009915 --target upload
