# UV reader

Read UV-A and UV-B radiation each minute and save the values with timestamp on an micro SD card.

## Pins

| Pin # | Name      | Pin name | Description                   |
| ----- | --------- | -------- | ----------------------------- |
| 1     | RST       | -        | Reset the chip                |
| 2     | ADC       | A0       | Read the UV sensor            |
| 3     | RESET     | -        | Enable, must be HIGH          |
| 4     | Deepsleep | 16       | Deepsleep, connect to RST pin |
| 5     | CSK       | 14       | SPI pin                       |
| 6     | MISO      | 12       | SPI pin                       |
| 7     | MOSI      | 13       | SPI pin                       |
| 16    | CS        | 15       | SPI pin                       |
| 17    | LED       | 2        | LED, turn on in LOW           |
| 18    | BOOT      | 0        | To flash the firmware         |
| 19    | SDA       | 4        | I2C pin                       |
| 20    | SCL       | 5        | I2C pin                       |
| 21    | RX        | -        | UART pin                      |
| 22    | TX        | -        | UART pin                      |
  
| Label | GPIO   | Input         | Output                | Notes                                                           |
| ----- | ------ | ------------- | --------------------- | --------------------------------------------------------------- |
| D0    | GPIO16 | no interrupt  | no PWM or I2C support | HIGH at boot used to wake up from deep sleep                    |
| D1    | GPIO5  | OK            | OK                    | often used as SCL (I2C)                                         |
| D2    | GPIO4  | OK            | OK                    | often used as SDA (I2C)                                         |
| D3    | GPIO0  | pulled up     | OK                    | connected to FLASH button, boot fails if pulled LOW             |
| D4    | GPIO2  | pulled up     | OK                    | HIGH at bootconnected to on-board LED, boot fails if pulled LOW |
| D5    | GPIO14 | OK            | OK                    | SPI (SCLK)                                                      |
| D6    | GPIO12 | OK            | OK                    | SPI (MISO)                                                      |
| D7    | GPIO13 | OK            | OK                    | SPI (MOSI)                                                      |
| D8    | GPIO15 | pulled to GND | OK                    | SPI (CS), Boot fails if pulled HIGH                             |
| RX    | GPIO3  | OK            | RX pin                | HIGH at boot                                                    |
| TX    | GPIO1  | TX pin        | OK                    | HIGH at boot, debug output at boot, boot fails if pulled LOW    |
| A0    | ADC0   | Analog        | Input                 | X                                                               |

## Calibration

Calculus to obtain the reference voltaje for genering UV intensity:

- `Vref` is the reference value to obtain:
- `Vout`: is read with multimeter.
- `Nread`: is read with ADC.

```bash
Vref = Vout x 1023 / Nread
Vref = 3.684
```

In the main code, put the next expression:  

```bash
Vout = Vref x Nread / 1023
Vout = 3.684 x Nread / 1023
```
