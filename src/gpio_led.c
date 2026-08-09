#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "gpio_led"

struct gpio_led_device {
    struct gpio_desc *led_gpio;
    struct miscdevice miscdev;
};

static ssize_t gpio_led_write(
    struct file *file,
    const char __user *buf,
    size_t count,
    loff_t *ppos)
{
    struct miscdevice *miscdev;
    struct gpio_led_device *led_dev;
    char value;

    if (count == 0)
        return -EINVAL;

    miscdev = file->private_data;

    led_dev = container_of(
        miscdev,
        struct gpio_led_device,
        miscdev
    );

    if (copy_from_user(&value, buf, 1))
        return -EFAULT;

    if (value == '1') {

        gpiod_set_value_cansleep(
            led_dev->led_gpio,
            1
        );

        pr_info("gpio_led: LED ON\n");

    } else if (value == '0') {

        gpiod_set_value_cansleep(
            led_dev->led_gpio,
            0
        );

        pr_info("gpio_led: LED OFF\n");

    } else {

        pr_warn(
            "gpio_led: invalid value. use 0 or 1\n"
        );

        return -EINVAL;
    }

    return count;
}

static ssize_t gpio_led_read(
    struct file *file,
    char __user *buf,
    size_t count,
    loff_t *ppos)
{
    struct miscdevice *miscdev;
    struct gpio_led_device *led_dev;

    char result[3];

    int value;
    int len;

    miscdev = file->private_data;

    led_dev = container_of(
        miscdev,
        struct gpio_led_device,
        miscdev
    );

    /*
     * cat 명령은 EOF가 나올 때까지
     * 계속 read()를 호출한다.
     */
    if (*ppos != 0)
        return 0;

    value = gpiod_get_value_cansleep(
        led_dev->led_gpio
    );

    len = snprintf(
        result,
        sizeof(result),
        "%d\n",
        value
    );

    if (count < len)
        return -EINVAL;

    if (copy_to_user(
            buf,
            result,
            len
        ))
        return -EFAULT;

    *ppos += len;

    return len;
}

static const struct file_operations gpio_led_fops = {
    .owner = THIS_MODULE,
    .read = gpio_led_read,
    .write = gpio_led_write,
};

static int gpio_led_probe(
    struct platform_device *pdev)
{
    struct gpio_led_device *led_dev;

    int ret;

    dev_info(
        &pdev->dev,
        "gpio_led: probe called\n"
    );

    led_dev = devm_kzalloc(
        &pdev->dev,
        sizeof(*led_dev),
        GFP_KERNEL
    );

    if (!led_dev)
        return -ENOMEM;

    /*
     * Device Tree의
     *
     * led-gpios = <...>;
     *
     * 를 가져온다.
     */
    led_dev->led_gpio = devm_gpiod_get(
        &pdev->dev,
        "led",
        GPIOD_OUT_LOW
    );

    if (IS_ERR(led_dev->led_gpio)) {

        dev_err(
            &pdev->dev,
            "gpio_led: failed to get GPIO\n"
        );

        return PTR_ERR(
            led_dev->led_gpio
        );
    }

    /*
     * /dev/gpio_led 설정
     */
    led_dev->miscdev.minor =
        MISC_DYNAMIC_MINOR;

    led_dev->miscdev.name =
        DEVICE_NAME;

    led_dev->miscdev.fops =
        &gpio_led_fops;

    led_dev->miscdev.parent =
        &pdev->dev;

    ret = misc_register(
        &led_dev->miscdev
    );

    if (ret) {

        dev_err(
            &pdev->dev,
            "gpio_led: misc_register failed\n"
        );

        return ret;
    }

    platform_set_drvdata(
        pdev,
        led_dev
    );

    dev_info(
        &pdev->dev,
        "gpio_led: /dev/gpio_led created\n"
    );

    return 0;
}

static void gpio_led_remove(
    struct platform_device *pdev)
{
    struct gpio_led_device *led_dev;

    led_dev = platform_get_drvdata(
        pdev
    );

    gpiod_set_value_cansleep(
        led_dev->led_gpio,
        0
    );

    misc_deregister(
        &led_dev->miscdev
    );

    dev_info(
        &pdev->dev,
        "gpio_led: driver removed\n"
    );
}

/*
 * Device Tree:
 *
 * compatible = "kyunghyun,gpio-led";
 *
 * 와 매칭
 */
static const struct of_device_id
gpio_led_of_match[] = {

    {
        .compatible =
            "kyunghyun,gpio-led"
    },

    { }
};

MODULE_DEVICE_TABLE(
    of,
    gpio_led_of_match
);

static struct platform_driver gpio_led_driver = {

    .probe = gpio_led_probe,
    .remove = gpio_led_remove,

    .driver = {

        .name = "gpio_led",

        .of_match_table =
            gpio_led_of_match,
    },
};

module_platform_driver(
    gpio_led_driver
);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KyungHyun Han");
MODULE_DESCRIPTION(
    "Raspberry Pi Linux GPIO LED Driver"
);
MODULE_VERSION("1.0");