#include <devices.h>

int sys_write(int fd, char * buffer, int size) {
    if (fd != 1) return -1;

    int ret = sys_write_console(buffer, size);

    return ret;
    
}