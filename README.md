# Linux GPIO driver

Raspberry Pi에서 동작하는 Linux GPIO LED Device Driver 

## Architecture

```text
User Application
      |
      | open / read / write
      v
/dev/gpio_led
      |
      v
Linux Platform Driver
      |
      v
GPIO Subsystem
      |
      v
GPIO17
      |
      v
LED

```

```
sudo apt update
sudo apt install -y raspberrypi-kernel-headers device-tree-compiler
```