obj-m += gpio_led.o

gpio_led-objs := src/gpio_led.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

dtbo:
	dtc -@ \
		-I dts \
		-O dtb \
		-o gpio_led.dtbo \
		dts/gpio_led_overlay.dts

user:
	gcc \
		-Wall \
		-Wextra \
		userspace/test_led.c \
		-o test_led

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f gpio_led.dtbo
	rm -f test_led