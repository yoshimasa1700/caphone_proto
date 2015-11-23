#include <pebble.h>

#ifndef ACCSENSORFILTER
#define ACCSENSORFILTER


static double tempx = 0.0;
static double tempy = 0.0;
static double tempz = 0.0;

static double prex = 0.0;
static double prey = 0.0;
static double prez = 0.0;

double my_sqrt( double num )
{
  double a, p, e = 0.001, b;

  a = num;
  p = a * a;
  while( p - num >= e )
    {
      b = ( a + ( num / a ) ) / 2;
      a = b;
      p = a * a;
    }
  return a;
}

static double accel_sensor_filter(AccelData *data){
  double sensorx = (double)data[0].x / 100.0;
  double sensory = (double)data[0].y / 100.0;
  double sensorz = (double)data[0].z / 100.0;

  // lpf
  tempx = sensorx * 0.1 + tempx * 0.9;
  tempy = sensory * 0.1 + tempy * 0.9;
  tempz = sensorz * 0.1 + tempz * 0.9;

  double accx = sensorx - tempx;
  double accy = sensory - tempy;
  double accz = sensorz - tempz;

  double dx = accx - prex;
  double dy = accy - prey;
  double dz = accz - prez;

  prex = accx;
  prey = accy;
  prez = accz;

  double vec_length = my_sqrt(dx * dx + dy * dy + dz * dz);

  return vec_length;
}

#endif
