#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <cstdlib>
#include "Mstage.h"

using namespace std;

int main(int argc, char **argv)
{
   int fd;
   char *dev = "/dev/ttyUSB0";
   int start = 1;
   Mstage M;      /*Creat an object by using class Mstage*/
   fd = M.OpenDev(dev);
   cerr << "Device opened" << endl;
   sleep(1);
   if (fd>0)
      M.set_speed(fd, 9600);
   else 
   {
      printf("Can't Open Serial Port!\n");
      exit(0);
   }
   cerr << "Speed set" << endl;
   if(M.set_Parity(fd,8,2,'N') == FALSE) 
   {
      printf("Set Parity Error\n");
      exit(0);
   }
   cerr << "Parity set" << endl;
   M.axisdir(fd, 1, 1);
   M.vel(fd, 2, 2);
   M.calib(fd);
   M.moay(fd, 10.566);
   sleep(1);
   M.pos(fd);
   close(fd);
   cerr << "Closed" << endl;
}









