#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(){
  int fd;

  char write_one[] = "1";
  char write_zero[] = "0";

  fd = open("/sys/class/gpio/gpio26/value", O_WRONLY);

  while(1){
    write(fd, write_one, strlen(write_one));
    sleep(1);
    write(fd, write_zero, strlen(write_zero));
  }

}
