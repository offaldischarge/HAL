#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

int main(){
  int fd, fd_led;
  char read_buffer[4];
  char write_one[] = "1";
  char write_zero[] = "0";

  fd = open("/dev/i2c-1", O_RDONLY);
  fd_led = open("/sys/class/gpio/gpio26/value", O_WRONLY);
  ioctl(fd, 0x0703, 0x48);

  while(1){
    read(fd, read_buffer, 4);
    printf("Read data: %i\n", read_buffer[0]);

    if(read_buffer[0] == 32){
      write(fd_led, write_one, strlen(write_one));
      fprintf(stdout, "Warning, temperature over 32\n");
    } else {
      write(fd_led, write_zero, strlen(write_zero));
    }
    sleep(1);
  }

}
