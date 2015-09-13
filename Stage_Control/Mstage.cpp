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

#define M_CMD_CAL          "!cal\n"
#define M_CMD_POS          "?pos\n"
#define M_CMD_ADDR         "^A0\n"
#define M_CMD_PROBE_ON     "MN\n"
#define M_CMD_PROBE_OFF    "MF\n"
#define M_CMD_PROBE_STOP   "ST\n"
#define M_CMD_PROBE_DH     "DH\n"
#define M_CMD_PROBE_GH     "GH\n"
#define M_CMD_PROBE_TE     "TE\n"
#define M_CMD_PROBE_TP     "TP\n"

using namespace std;

void Mstage::set_port_speed(int fd, int speed)
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

int Mstage::set_port_Parity(int fd, int databits, int stopbits, int parity)
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

void Mstage::stage_calib(int Fd)
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

void Mstage::stage_pos(int Fd)
{
   char buff[15];
   int res = write (Fd, M_CMD_POS, strlen(M_CMD_POS));
   if (res < 0)
   {
   	cerr << "Error writing postion feedback command" << endl;
   	return;
   }
   int numbytes = read(Fd, buff, 15);
   cerr << "Stage Current Position: " << buff << endl; 
}

void Mstage::stage_axisdir(int Fd, int x, int y)
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

void Mstage::stage_vel(int Fd, float x, float y)
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

void Mstage::stage_moa(int Fd, float x, float y)
{
    char cmd_moa[25];
    char buff[6];
    sprintf(cmd_moa, "!moa %f %f\n", x, y);
    int res = write (Fd, cmd_moa, strlen(cmd_moa));
    if (res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    }
    int numbytes = read(Fd, buff, 6);
    cerr << "Command Executed" << endl; 
}

void Mstage::stage_moax(int Fd, float x)
{
    char cmd_moax[17];
    char buff[7];
    sprintf(cmd_moax, "!moa x %f\n", x);
    int res = write (Fd, cmd_moax, strlen(cmd_moax));
    if (res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    }
    int numbytes = read(Fd, buff, 7);
    cerr << "Command Executed" << endl;	 
}

void Mstage::stage_moay(int Fd, float x)
{
    char cmd_moay[17];
    char buff[7];
    sprintf(cmd_moay, "!moa y %f\n", x);
    int res = write (Fd, cmd_moay, strlen(cmd_moay));
    if (res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    }
    int numbytes = read(Fd, buff, 7);
    cerr << "Command Executed" << endl;	 
}

void Mstage::probe_connect(int Fd)
{
    int res = write(Fd, M_CMD_ADDR, strlen(M_CMD_ADDR));
    if (res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    } 
    cerr << "Probe Motor Connectted" << endl; 
}

void Mstage::probe_on(int Fd)
{
    int res = wirte(Fd, M_CMD_PROBE_ON, strlen(M_CMD_PROBE_ON));
    if (res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    }
    cerr << "Probe turns on" << endl;
}

void Mstage::probe_off(int Fd)
{
    int res = wirte(Fd, M_CMD_PROBE_OFF, strlen(M_CMD_PROBE_OFF));
    if (res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    }
    cerr << "Probe turns off" << endl;
}

void Mstage::probe_ST(int Fd)
{
    int res = wirte(Fd, M_CMD_PROBE_STOP, strlen(M_CMD_PROBE_STOP));
    if (res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    }
    cerr << "Motor stopped" << endl;
}

void Mstage::probe_Ki(int Fd, float Ki)
{
    char cmd_Ki[13];
    sprintf(cmd_Ki, "DI%F\n", Ki);
    int res = wirte(Fd, cmd_Ki, strlen(cmd_Ki));
    if(res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    }
    cerr << "Ki gain is " << Ki << endl;
}

void Mstage::probe_Kd(int Fd, float Kd)
{
    char cmd_Kd[13];
    sprintf(cmd_Kd, "DD%F\n", Kd);
    int res = wirte(Fd, cmd_Kd, strlen(cmd_Kd));
    if(res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    }
    cerr << "Kd gain is " << Kd << endl;
}

void Mstage::probe_Kp(int Fd, float Kp)
{
    char cmd_Kp[13];
    sprintf(cmd_Kp, "DP%F\n", Kp);
    int res = wirte(Fd, cmd_Kp, strlen(cmd_Kp));
    if(res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    }
    cerr << "Kp gain is " << Kp << endl;
}

void Mstage::probe_DH(int Fd)
{
    int res = write(Fd, M_CMD_PROBE_DH, strlen(M_CMD_PROBE_DH));
    if (res < 0)
    {
         cerr << "Error writing in command" << endl;
         return;
    }
    cerr << "Home Defined" << endl;
}

void Mstage::probe_GH(int Fd)
{
     int res = wirte(Fd, M_CMD_PROBE_GH, strlen(M_CMD_PROBE_GH));
     if (res < 0)
     {
         cerr << "Error writing in command" << endl;
         return;
     } 
     cerr << "Motor arrived home" << endl;
}

void Mstage::probe_MA(int Fd, int x)
{
     char cmd_ma[10];
     sprintf(cmd_ma, "MA%d\n", x);
     int res = write(Fd, cmd_ma, strlen(cmd_ma));
     if (res < 0)
     {
         cerr << "Error writing in command" << endl;
         return;
     } 
     cerr << "Moving Command Executed" << endl;
}

void Mstage::probe_MR(int Fd, int x)
{
     char cmd_mr[10];
     sprintf(cmd_ma, "MR%d\n", x);
     int res = write(Fd, cmd_mr, strlen(cmd_mr));
     if (res < 0)
     {
         cerr << "Error writing in command" << endl;
         return;
     } 
     cerr << "Moving Command Executed" << endl;
}

void Mstage::probe_SA(int Fd, int x)
{
     char cmd_sa[10];
     sprintf(cmd_sa, "SA%d\n", x);
     int res = write(Fd, cmd_sa, strlen(cmd_sa));
     if (res < 0)
     {
         cerr << "Error writing in command" << endl;
         return;
     }
     cerr << "Acceleration setted" << endl; 
}

void Mstage::probe_SV(int Fd, int x)
{
     char cmd_sv[10];
     sprintf(cmd_sv, "SV%d\n", x);
     int res = write(Fd, cmd_sv, strlen(cmd_sv));
     if (res < 0)
     {
         cerr << "Error writing in command" << endl;
         return;
     }
     cerr << "Velocity setted" << endl; 
}

void Mstage::probe_TE(int Fd)
{
     char buff[1024];
     int res = write(Fd, M_CMD_PROBE_TE, strlen(M_CMD_PROBE_TE));
     if (res < 0)
     {
         cerr << "Error writing in command" << endl;
         return;
     }
     int numbytes = read(Fd, buff, 1024);
   cerr << "Position error: " << buff << endl;  
}

void Mstage::probe_TP(int Fd)
{
     char buff[1024];
     int res = write(Fd, M_CMD_PROBE_TP, strlen(M_CMD_PROBE_TP));
     if (res < 0)
     {
         cerr << "Error writing in command" << endl;
         return;
     }
     int numbytes = read(Fd, buff, 1024);
   cerr << "Probe Current position: " << buff << endl;  
}