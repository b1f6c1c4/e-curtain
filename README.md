# e-curtain

> Minimalistic IoT on Raspberry Pi

## Features

- **Everything is DIY**
- Automatic bedroom temperature control for optimal sleep quality
- Automatic room ventilation for optimal air quality
- Automatic weather information retrival
- Automatic room temperature measurement
- Automatic curtain control
- Automatic window control
- Automatic AC cooling/heating/ventilating/off mode selection
- Automatic duct gate (cut-off) control
- Automatic fan control (on/off)
- Keyfobs for easy global mode selection
- (TODO) Web-based dashboard for monitoring

## Hardware requirement

- Raspberry Pi Zero W (x3)
- I2C-based temperature sensor
- Continuous rotation servo
    - For curtain control
- Stepper with controller
    - For window control
- Servo (x4)
    - For AC mode selection (x2)
    - For duct gate control (x2)
- Relay
    - For fan control
- RF keyfob(s) with receiver
    - For global mode selection
- Other stuff
    - Power adapters
    - Breadboards
    - Jump wires
    - Headers

## Layout

- `controller-0`: The central controller, running `F-G0`
    - Servo x4
    - RF receiver
- `controller-1`: The bedroom controller, running `H1-G1`
    - Temperature sensor
    - Continuous rotation servo
    - Stepper with controller
    - Relay
- `controller-2`: The living room controller, running `H2` and `H0-C`
    - Temperature sensor

## Hardware setup

Read the source code and you will figure out.

## Software setup

### Prepare cross-compile toolchain

1. **On a Linux machine**, install [crosstools-ng](https://crosstool-ng.github.io/)
1. Get the source code:

    ```bash
    git clone --depth=1 https://github.com/b1f6c1c4/e-curtain.git
    cd e-curtain
    ```

1. Run the following:

    ```bash
    cd ct-ng
    ct-ng build.128 # Depends on how many cores you have
    ```

### MATLAB Simulink code generation

You simply generate `script/libdumbac.slx` and put the result in a folder called `libdumbac`.
You may need licences for doing this.

We also need you know your MATLAB installation, so run the following:

```bash
ln -s /opt/MATLAB/R2020a matlab
```

### Compile

```bash
make build -j128 # Depends on how many cores you have
```

### Deploy

```bash
make HOST0=192.168.x.x HOST1=192.168.x.x HOST2=192.168.x.x deploy
```

You need to put weather API key into `$HOST2:/etc/e-curtain/weather.json`.

Data log are written to `$HOST2:/var/log/e-curtain.bin`.

## License

MIT
