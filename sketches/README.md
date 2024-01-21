# Simple Workshop Examples

Example code for the Simple in-person workshops.

## Running in the Arduino IDE

Open an example from the examples folder in the Arduino IDE and press the Upload button.

## Running using `arduino-cli`

Make sure you have [arduino-cli](https://github.com/arduino/arduino-cli) installed. Then compile an example using

```bash
make build EXAMPLE=name-of-example # e.g. EXAMPLE=simple-fm
```

To upload it (you can also directly just upload it):

```bash
make upload EXAMPLE=name-of-example
```
