# nrf node code

This is the code running on anything connected to an nRF24L01+ module in my house.

## Gateway

Arduino code of a gateway talking to the nodes. The gateway is responsible for listening to data from nodes.
It can also send data to other nodes.

It uses serial to communicate what it hears from nodes, as well as takes commands to send to external nodes.

### Reading nodes data

Each package from a node is sent to the serial in the following format:

    Data 100, 1, 255, 8, 27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

Each line is prepended with `Data`, followed with 32 byte values. The first three bytes are: Sensor ID, Sensor Type, and voltage.
Voltage is a value between 0-254. 255 is used to signify no battery.

### Sending data to nodes

The gateway listens for 37 bytes on the serial connection. Those 37 bytes are composed of 5 address bytes, and 32 message bytes.
Currently there is no way to reset the connection, but should be added.

## tempSensor

This is the arduino code for a temperature sensor. Update SENSOR_ID when making new sensors.

# receiver

Generic code for a sensor that can receive data from a gateway.
