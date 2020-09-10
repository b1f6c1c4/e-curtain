# e-curtain

> Minimalistic IoT on Raspberry Pi

## Features

- **Everything is DIY**
- Automatic bedroom temperature control for optimal sleep quality
- Automatic room ventilation for optimal air quality
- Automatic weather information retrival
- Automatic room temperature measurement
- Automatic curtain control (open/close)
- Automatic window control (open/close)
- Automatic AC cooling/heating/ventilating/off mode selection
- Automatic duct gate (cut-off) control (on/off)
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

- `controller-0`: The central controller
    - Servo x4
    - RF receiver
- `controller-1`: The bedroom controller
    - Temperature sensor
    - Continuous rotation servo
    - Stepper with controller
    - Relay
- `controller-2`: The living room controller
    - Temperature sensor

## Hardware setup

Read the source code and you will figure out.

## Software setup

1. Get the source code:

    ```bash
    git clone --depth=1 https://github.com/b1f6c1c4/e-curtain.git
    ```

1. Edit `hosts` to discover other controllers:

    ```bash
    sudo vim /etc/hosts
    ```

1. Enable `systemd` service:

    ```bash
    touch controller-X
    sudo ln -s $PWD/e-curtain.service /etc/systemd/system/
    sudo systemctl enable e-curtain
    ```

## License

MIT
