#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

int main(){
  int fd, fd_led, fd_web;

  char read_buffer[4];
  char write_one[] = "1";
  char write_zero[] = "0";
  char write_temp[64];

  fd = open("/dev/i2c-1", O_RDONLY);
  fd_led = open("/sys/class/gpio/gpio26/value", O_WRONLY);
  fd_web = open("/www/pages/index.html", O_WRONLY);

  ioctl(fd, 0x0703, 0x48);

  while(1){
    read(fd, read_buffer, 4);
    sprintf(write_temp, "<html><body><h1>Temperature: %i</h1></body></html>", read_buffer[0]);
    lseek(fd_web, 0, SEEK_SET);
    write(fd_web, write_temp, strlen(write_temp));
    sleep(1);
  }

}
