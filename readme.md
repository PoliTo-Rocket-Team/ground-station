# Ground Station

QtQuick Ddesktop app of the ground station + Arduino code of the LoRa 

## Messages

| code | name | load type | load description | 
| :--: | ---- | :--: | ----------- |
| C | COM Check | - | - |
| O | Ok | - | - |
| E | Error | 1 byte | which error occured (see [error codes](#errors-codes)) |
| D | Data | 9 float | barometer, lat, lng, linear acceleration, angular acceleration |
| P | Parachute opened | 1 byte | 1 for drogue, 2 for main |
| F | Frequency change | 1 byte | 0 to 81 (see LoRa library) |

All of the messages are simply forwarded by the GS LoRa, but some extra logic is implemented for `[C]` and `[F]`.

### Communication check 

When the rocket board startups or has changed the frequency, it trasmits repeatedly a [C] message. Upon receiving it, the GS LoRa forwards it to the app backend and sends it back to the rocket, ignoring any further `[C]` message received.

### Frequency change

After the GS LoRa forwards `[F]` with the new frequency:

 - If no `[C]` is received within 5 seconds, the GS LoRa sends `[E4]` to the app backend and switches to the old frequency;
 - Otherwise, the `[C]` is sent back to the rocket and forwarded to the backend (ignoring any further `[C]` from the rocket). 

### Errors codes

| number | description |
| :----: | ----------- |
| 1 | IMU not working |
| 2 | Barometer not working |
| 3 | GPS not working |
| 4 | Could not change frequency |
