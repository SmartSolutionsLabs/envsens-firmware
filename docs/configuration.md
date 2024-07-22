# Device Configuration Guide

## Using Bluetooth Configuration Mode

The device is configured using Bluetooth configuration mode. All commands are structured in JSON format.

### Command Structure

All messages must include a mandatory key "cmd" followed by an integer instruction. For example:

```json
{"cmd": 1}
```

#### Configuring Network Credentials (cmd: 7)

To configure network credentials, use command 7. Include the following string attributes:

	"ssid": WiFi access point name.
	"pass": Password for the WiFi network.

Example:

```json
{"cmd": 7, "ssid": "YourWiFiNetwork", "pass": "YourWiFiPassword"}
```

#### Setting System Clock (cmd: 9)

To set the system clock, use command 9. Include the following string attribute:

	"time": Date and time formatted as YYYYMMDDhhmmss (e.g., 20240714203040 for July 14, 2024, 20:30:40).

Example:

```json
{"cmd": 9, "time": "20240714203040"}
```

#### Configuring Device Name (cmd: 1000)

To configure the device name, use command 1000. Include the following string attribute:

	"name": Name of the device.

Example:

```json
{"cmd": 1000, "name": "YourDeviceName"}
```

#### Configuring Endpoints (cmd: 1001)

To configure endpoints, use command 1001. Include the following string attributes:

	"host": Domain where the device will send HTTP requests.
	"post": Complete path of the endpoint starting with slash /.

Example:

```json
{"cmd": 1001, "host": "example.com", "post": "/api/data"}
```

#### Setting Data Transmission Interval (cmd: 1002)

To configure the data transmission interval, use command 1002. Include the following numeric attribute:

	"lap": Interval in minutes (1 to 59) when the device will send data.

Example:

```json
{"cmd": 1002, "lap": 15}
```

#### Configuring Sensor Calibration Constants (cmd: 1003)

To configure sensor calibration constants, use command 1003. These constants are generally values ​​close to 1. Include any of the following numeric attributes:

	"co2": Integer value for CO2 calibration.
	"nh3": Decimal value for NH3 calibration.
	"t": Decimal value for temperature calibration.
	"humidity": Integer value for humidity calibration.
	"pressure": Integer value for pressure calibration.

Example:

```json
{"cmd": 1003, "co2": 1, "nh3": 1.23, "t": 1.5, "humidity": 1, "pressure": 1}
```

#### Retrieving Current Device Values (cmd: 2000)

To retrieve all current device values, use command 2000.

Example:

```json
{"cmd": 2000}
```

### Notes

	Ensure Bluetooth connectivity and correct JSON formatting for successful configuration.
	Refer to specific device documentation for advanced configurations and troubleshooting tips.
