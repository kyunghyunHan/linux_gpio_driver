#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/gpio_led"

int main(void)
{
    int fd;
    char input;

    fd = open(
        DEVICE_PATH,
        O_RDWR
    );

    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf(
        "Linux GPIO LED Driver Test\n"
    );

    printf(
        "1 : LED ON\n"
        "0 : LED OFF\n"
        "q : Quit\n"
    );

    while (1) {

        printf("> ");

        if (scanf(
                " %c",
                &input
            ) != 1) {

            break;
        }

        if (input == 'q')
            break;

        if (
            input != '0' &&
            input != '1'
        ) {

            printf(
                "Enter 0 or 1\n"
            );

            continue;
        }

        if (write(
                fd,
                &input,
                1
            ) < 0) {

            perror("write");

            break;
        }
    }

    close(fd);

    return 0;
}