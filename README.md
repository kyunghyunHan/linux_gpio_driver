# Raspberry Pi Linux GPIO Driver

Raspberry Pi Linux 환경에서 GPIO LED를 제어하는 Linux Device Driver 실습

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
GPIO2
      |
      v
LED
```

---

## 1. Raspberry Pi 접속

Raspberry Pi의 IP 확인

```bash
hostname -I
```

Mac에서 SSH 접속

```bash
ssh xxx@raspberrypi.local
```

또는 IP로 접속

```bash
ssh xxx@xxx.xxx.x.xx
```

---

## 2. 개발 환경 설치

패키지 업데이트

```bash
sudo apt update
```

기본 개발 도구 및 Git 설치

```bash
sudo apt install -y build-essential git
```

Device Tree Compiler 및 Raspberry Pi Kernel Header 설치

```bash
sudo apt install -y linux-headers-rpi-v8 device-tree-compiler
```

설치 확인

```bash
gcc --version
make --version
git --version
```

현재 Kernel 버전 확인

```bash
uname -r
```

Kernel Header 확인

```bash
ls -l /lib/modules/$(uname -r)/build
```

---

## 3. 프로젝트 Clone

```bash
cd ~
git clone https://github.com/kyunghyunHan/linux_gpio_driver.git
```

프로젝트 이동

```bash
cd ~/linux_gpio_driver
```

파일 확인

```bash
ls
```

---

## 4. Source 파일 확인

```bash
ls -l src
```

Makefile에서 다음 파일을 사용하는 경우

```makefile
gpio_led-objs := src/gpio_led.o
```

실제 파일도 다음과 같아야 한다.

```text
src/gpio_led.c
```

파일명이 `led.c`인 경우

```bash
mv src/led.c src/gpio_led.c
```

---

## 5. GPIO 설정

Device Tree Overlay 확인

```bash
nano dts/gpio_led_overlay.dts
```

GPIO2를 사용하는 경우

```dts
led-gpios = <&gpio 2 0>;
```

Raspberry Pi 4 기준

```text
GPIO2 = Physical Pin 3
GND   = Physical Pin 6
```

GPIO2는 I2C SDA로도 사용되는 핀이므로 I2C 사용 여부에 주의한다.

---

## 6. Kernel Module 빌드

```bash
make
```

성공 확인

```bash
ls -l gpio_led.ko
```

`gpio_led.ko`는 Linux Kernel에 로드할 Kernel Module이다.

---

## 7. Device Tree Overlay 빌드

```bash
make dtbo
```

성공 확인

```bash
ls -l gpio_led.dtbo
```

---

## 8. User Application 빌드

```bash
make user
```

성공 확인

```bash
ls -l test_led
```

---

## 9. 전체 빌드 결과 확인

```bash
ls -l gpio_led.ko gpio_led.dtbo test_led
```

다음 3개가 생성되어야 한다.

```text
gpio_led.ko
gpio_led.dtbo
test_led
```

- `gpio_led.ko` : Linux Kernel Module
- `gpio_led.dtbo` : Device Tree Overlay
- `test_led` : User Space 테스트 프로그램

---

# 여기까지 완료

현재까지 완료한 작업

```text
Git Clone
    ↓
Kernel Header 설치
    ↓
Device Tree Compiler 설치
    ↓
Kernel Module Build
    ↓
gpio_led.ko 생성
    ↓
Device Tree Overlay Build
    ↓
gpio_led.dtbo 생성
    ↓
User Application Build
    ↓
test_led 생성
```

---

# 집에서 이어서 할 작업

## 10. LED 배선

GPIO2를 사용하는 경우

```text
Raspberry Pi GPIO2 (Physical Pin 3)
            |
            |
         220~330Ω
           저항
            |
            |
        LED 긴 다리 (+)
        LED 짧은 다리 (-)
            |
            |
     GND (Physical Pin 6)
```

GPIO 번호와 Physical Pin 번호를 혼동하지 않는다.

```text
GPIO2 ≠ Physical Pin 2

GPIO2 = Physical Pin 3
```

---

## 11. Device Tree Overlay 설치

```bash
sudo cp gpio_led.dtbo /boot/firmware/overlays/
```

설정 파일 열기

```bash
sudo nano /boot/firmware/config.txt
```

맨 아래에 추가

```text
dtoverlay=gpio_led
```

저장 후 재부팅

```bash
sudo reboot
```

---

## 12. 재부팅 후 SSH 접속

```bash
ssh xxx@raspberrypi.local
```

프로젝트 이동

```bash
cd ~/linux_gpio_driver
```

---

## 13. Kernel Module 로드

```bash
sudo insmod gpio_led.ko
```

Module 확인

```bash
lsmod | grep gpio_led
```

---

## 14. Kernel Log 확인

```bash
sudo dmesg | tail -30
```

Driver의 `probe()`가 정상적으로 실행됐는지 확인한다.

---

## 15. Device File 확인

```bash
ls -l /dev/gpio_led
```

정상적으로 Driver가 등록되었다면 다음 Device File이 존재해야 한다.

```text
/dev/gpio_led
```

구조

```text
test_led
   |
   | open()
   | write()
   v
/dev/gpio_led
   |
   v
Kernel Driver
   |
   v
GPIO Subsystem
   |
   v
GPIO2
   |
   v
LED
```

---

## 16. LED 테스트

```bash
sudo ./test_led
```

프로그램에서

```text
1
```

LED ON

```text
0
```

LED OFF

```text
q
```

프로그램 종료

---

## 17. Kernel Module 제거

테스트가 끝나면

```bash
sudo rmmod gpio_led
```

확인

```bash
lsmod | grep gpio_led
```

아무것도 나오지 않으면 제거된 상태이다.

---

## 18. 문제 발생 시 확인

Driver가 정상적으로 로드됐는지

```bash
lsmod | grep gpio_led
```

Device File이 생성됐는지

```bash
ls -l /dev/gpio_led
```

Kernel Log 확인

```bash
sudo dmesg | tail -30
```

GPIO2의 I2C 설정 확인

```bash
grep -n "dtparam=i2c" /boot/firmware/config.txt
```

---

## 19. Raspberry Pi 종료

실습 종료 후 안전하게 종료

```bash
sudo poweroff
```

또는

```bash
sudo shutdown -h now
```

종료가 완료된 후 전원을 분리한다.

---

# 프로젝트 목표

Raspberry Pi에서 Linux Platform Driver를 구현하고 Device Tree를 통해 GPIO를 Driver에 연결한다.

User Space 프로그램은 `/dev/gpio_led` Device File에 `open()` / `write()`를 수행하고, Kernel Driver가 Linux GPIO Subsystem을 통해 실제 GPIO를 제어한다.

최종 구조:

```text
User Space
    |
    | System Call
    v
Linux Kernel
    |
    | Device Driver
    v
GPIO Subsystem
    |
    v
Raspberry Pi GPIO
    |
    v
LED
```