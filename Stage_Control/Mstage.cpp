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

#define M_CMD_CAL   "!cal\n"
#define M_CMD_POS   "?pos\n"

using namespace std;

void Mstage::set_speed(int fd, int speed)
{
   int speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300, 
      	              B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300 };
   int name_arr[] = { 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300, 
   	                  115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300 };
   int i;
   int status;
   struct termios Opt;
   tcgetattr(fd, &Opt);
   for ( i=0; i<sizeof(speed_arr)/sizeof(int); i++)
   {
      if (speed == name_arr[i])
      {
         tcflush(fd, TCIOFLUSH);
         cfsetispeed(&Opt, speed_arr[i]);
         cfsetospeed(&Opt, speed_arr[i]);
         status = tcsetattr(fd, TCSANOW, &Opt);
         if  (status != 0)
         {  
            perror("tcsetattr fd1");
            return;
         }
         tcflush(fd,TCIOFLUSH);
      }
   }
}

int Mstage::set_Parity(int fd, int databits, int stopbits, int parity)
{
   struct termios options;
   if  ( tcgetattr(fd, &options)  !=  0)
   {
      perror("SetupSerial 1");
      return (FALSE);
   }
   options.c_cflag &= ~CSIZE;
   switch (databits)
   {
      case 7:
             options.c_cflag |= CS7;
             break;
      case 8:
             options.c_cflag |= CS8;
             break;
      default:
             fprintf(stderr,"Unsupported data size\n");
             return (FALSE);
   }
   switch (parity)
   {
      case 'n':
      case 'N':
         options.c_cflag &= ~PARENB;   /* Clear parity enable */
         options.c_iflag &= ~INPCK;     /* Enable parity checking */
         break;
      case 'o':
      case 'O':
         options.c_cflag |= (PARODD | PARENB);
         options.c_iflag |= INPCK;             /* Disnable parity checking */
         break;
      case 'e':
      case 'E':
         options.c_cflag |= PARENB;     /* Enable parity */
         options.c_cflag &= ~PARODD;
         options.c_iflag |= INPCK;       /* Disnable parity checking */
         break;
      case 'S':
      case 's':
         options.c_cflag &= ~PARENB;
         options.c_cflag &= ~CSTOPB;
         break;
      default:
         fprintf(stderr,"Unsupported parity\n");
         return (FALSE); 
   }
   switch (stopbits)
   {
      case 1:
         options.c_cflag &= ~CSTOPB;
         break;
      case 2:
         options.c_cflag |= CSTOPB;
         break;
      default:
         fprintf(stderr,"Unsupported stop bits\n");
         return (FALSE);
   }
   /* Set input parity option */
   if (parity != 'n')
      options.c_iflag |= INPCK;
   tcflush(fd,TCIFLUSH);
   options.c_cc[VTIME] = 150; /* 15 seconds */
   options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
   if (tcsetattr(fd,TCSANOW,&options) != 0)
   {
      perror("SetupSerial 3");
      return (FALSE);
   }
   return (TRUE);
}

int Mstage::OpenDev(char *Dev)
{
   int fd = open( Dev, O_RDWR );         
   if (-1 == fd)
   {
      perror("Can't Open Serial Port");
      return -1;
   }
   else 
      return fd;
}

void Mstage::calib(int Fd)
{
   char buff[6];
   int res = write(Fd, M_CMD_CAL, strlen(M_CMD_CAL));
   if (res < 0)
   {
   	cerr << "Error writing calibration command" << endl;
   	return;
   }
   //wait until we read something
   int numbytes = read(Fd, buff, 6);
   //cerr << buff << endl;  
   cerr << "Calibrated" << endl;
 }

void Mstage::pos(int Fd)
{
   char buff[15];
   int res = write (Fd, M_CMD_POS, strlen(M_CMD_POS));
   if (res < 0)
   {
   	cerr << "Error writing postion feedback command" << endl;
   	return;
   }
   int numbytes = read(Fd, buff, 15);
   cerr << "Current Position: " << buff << endl; 
}

void Mstage::axisdir(int Fd, int x, int y)
{
    char cmd_axisdir[13];
    sprintf (cmd_axisdir, "!axisdir %d %d\n", x, y);
    int res = write (Fd, cmd_axisdir, strlen(cmd_axisdir));
    if (res < 0)
    {
   	 cerr << "Error writing axis direction command" << endl;
   	 return;
    }
    cerr << "Axis direction setted" << endl;
}

void Mstage::vel(int Fd, float x, float y)
{
    char cmd_vel[23];
    sprintf(cmd_vel, "!vel %f %f\n", x, y);
    int res = write (Fd, cmd_vel, strlen(cmd_vel));
    if (res < 0)
    {
   	 cerr << "Error writing motor velocity command" << endl;
   	 return;
    }
    cerr << "X axis velocity: " << x << " r/s" << endl;
    cerr << "Y axis velocity: " << y << " r/s" << endl;	
}

void Mstage::moa(int Fd, float x, float y)
{
    char cmd_moa[25];
    char buff[6];
    sprintf(cmd_moa, "!moa %f %f\n", x, y);
    int res = write (Fd, cmd_moa, strlen(cmd_moa));
    if (res < 0)
    {
         cerr << "Error writing moving command" << endl;
         return;
    }
    int numbytes = read(Fd, buff, 6);
    cerr << "Command Executed" << endl; 
}

void Mstage::moax(int Fd, float x)
{
    char cmd_moax[17];
    char buff[7];
    sprintf(cmd_moax, "!moa x %f\n", x);
    int res = write (Fd, cmd_moax, strlen(cmd_moax));
    if (res < 0)
    {
         cerr << "Error writing moving command" << endl;
         return;
    }
    int numbytes = read(Fd, buff, 7);
    cerr << "Command Executed" << endl;	 
}

void Mstage::moay(int Fd, float x)
{
    char cmd_moay[17];
    char buff[7];
    sprintf(cmd_moay, "!moa y %f\n", x);
    int res = write (Fd, cmd_moay, strlen(cmd_moay));
    if (res < 0)
    {
         cerr << "Error writing moving command" << endl;
         return;
    }
    int numbytes = read(Fd, buff, 7);
    cerr << "Command Executed" << endl;	 
}
