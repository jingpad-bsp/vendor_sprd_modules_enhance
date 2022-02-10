#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <hardware/enhance.h>

int show_bgcolor(void) {

    int fd = open("/sys/class/display/dispc0/bg_color", O_WRONLY);
    if (fd < 0) {
        printf("%s: open bg_color file failed\n", __func__);
        return -errno;
    }

    if (write(fd, "0xffffff", strlen("0xffffff")) < 0) {
        close(fd);
        printf("%s: write bg_color file failed\n", __func__);
        return -errno;
    }

    close(fd);

    return 0;
}

int show_ui(void) {

    int fd = open("/sys/class/display/dispc0/refresh", O_WRONLY);
    if (fd < 0) {
        printf("%s: open file refresh failed\n", __func__);
        return -errno;
    }

    if (write(fd, "1", 1) < 0) {
        close(fd);
        printf("%s: write refresh file failed\n", __func__);
        return -errno;
    }

    close(fd);

    return 0;
}

int main(int argc, char *argv[]) {

    enhance_device_t* enhance;
    const hw_module_t* module;

    if (argc != 2) {
        printf("please use: flash_test temperature_value\n");
        return -1;
    }

    if (hw_get_module(ENHANCE_HARDWARE_MODULE_ID, &module)) {
        printf("load enhance.so failed!\n");
        return -1;
    }

    if (module->methods->open(module, "flash",
                reinterpret_cast<hw_device_t**>(&enhance))) {
        printf("open enhance.so failed!\n");
        return -1;
    }

    int temperature = strtol(argv[1], NULL, 0);
    if (enhance->set_value(temperature))
        printf("set temperature %d failed\n", temperature);

    if (show_bgcolor())
        printf("show background color failed\n");

    sleep(1);

    if (show_ui())
        printf("show UI failed\n");

    if (enhance->set_value(0))
        printf("set temperature 0 failed\n");

    return 0;
}

