#ifndef __MSTAGE_H__
#define __MSTAGE_H__

#define TRUE    1
#define FALSE   -1

class Mstage
{
   public:
      void   set_speed(int fd, int speed); /*Set Serial Port Speed*/
      int    set_Parity(int fd, int databits, int stopbits, int parity); /*Set Serial Port Parity*/
      int    OpenDev(char *Dev); /*Open Sereial Port*/
      void   calib(int Fd); /*Stage Calibration*/
      void   pos(int Fd); /*Current Position Coordinate*/
      void   axisdir(int Fd, int x, int y); /*Set Motor Direction*/
      void   vel(int Fd, float x, float y); /*Set Motor Velocity*/
      void   moa(int Fd, float x, float y); /*Move Stage along Both X and Y axis*/
      void   moax(int Fd, float x); /*Move Stage along X Axis*/
      void   moay(int Fd, float x); /*Move Stage along Y Axis*/
};

#endif 
