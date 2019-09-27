#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

int main(){
  int fd, ctlret;
  char read_buffer[4];

  fd = open("/dev/i2c-1", O_RDONLY);
  ctlret = ioctl(fd, 0x0703, 0x42);

  while(1){
    if(ctlret == 0){
        read(fd, read_buffer, 4);
        printf("Read data: %i \n", read_buffer[0]);
        sleep(1);
    } else {
        printf("Error: %s \n", strerror(errno));
        break;
        return -1;
    }
  }

}
