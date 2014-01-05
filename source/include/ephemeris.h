//Hardware function header file for puzzle box mainboard
//9/3/2011 - PDS


#ifndef _EPHEMERIS_H_
#define _EPHEMERIS_H_

//#define I2CDEV LPC_I2C1

//#define GPIO_INTERRUPT_PRIORITY		0x1F

//void HWInit (void);

//void LED (unsigned char led, unsigned char state);

//void EINT3_IRQHandler (void);


void ephemeris(float lat, float lon, float UT, float *azimuth, float *altitude);

int intr(double num);
double rnd(double num, int num2);
int sgn(double num);
double dsin(double num);
double dcos(double num);
double dtan(double num);
double darcsin(double num);
double darccos(double num);
double datan(double num);
double datan2(double y, double x);
double rev(double x);

void calculate(int year, int month, int day, int hr, int mn, int sc, double latitude, double longitude );

void planet(int desc, double LST, double lat, double Rs, double M, double e,double i, double a, double w, double d, double N, double Ls, double Ms, double xsun, double ysun, double zsun, double *slon, double *slat);
double cMoon_day(double Lm, double Ls, double Mm, double Ms);
double cMoon_long(double Lm, double Ls, double Mm, double Ms);
double cMoon_phase(double ag);
void calculate(int year, int month, int day, int hr, int mn, int sc, double latitude, double longitude );
int DayOfYear(int year, int month, int day);



#endif