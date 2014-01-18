//
//
//#include <stdint.h>
////#include "LPC17xx.h"
//
////#include "hardware.h"
//#include <float.h>
////#include <tgmath.h>
//#include <math.h>
////#include <complex.h>
//#include "Ephemeris.h"
//
///*
//double px, py, pz;
//double d, N, Lm, Ls, Ms, Mm;
//double mlon, mlat, slon, slat;
//double E, i;
//double w, a, e, M;
//double xsun, ysun, zsun;
//double pRA, pDecl, LST, lat, lon, az, al, Rs;
//double zone, dst;
//double UT, oblecl, L, E0, E1;
//double j;
//double x, y, z, v, lng;
//*/
//double az, al;
//double PI=3.141592654;
//
//void ephemeris(float lat, float lon, float UT, float *azimuth, float *altitude)
//{
//
//	int day = 17;
//	int month = 7;
//	int year =2010;
//	int hr = 12;
//	int mn = 25;
//	int sc = 53;
//
//	double a1, a2, a3, lattitude, longitude;
//
//	//north is positive
//	a1 =  43;//latitude day
//	a2 =  15;//latitude min
//	a3 =  49;//latitude sec
//	lattitude = a1 + (a2 + a3 / 60.0D) / 60.0D;
//
//	//east is positive
//	a1 = -86;//longitude day
//	a2 = -1;//longitude min
//	a3 = -25;//longitude sec
//	longitude = a1 + (a2 + a3 / 60.0D) / 60.0D;
//
//	//loadit();
//	calculate(year, month, day, hr, mn, sc, lattitude, longitude);
//
//	*azimuth=az;
//	*altitude=-al;
//}
//
//
//
//
//
////    Mathematic section.
////    This function returns the integer of a number.
//int intr(double num)
//{
//	int n;
//	n = (int)floor(fabs(num));
//	if (num < 0) n = n * -1;
//	return n;
//}
//
////    This function rounds a number by specified decimals.
//double rnd(double num, int num2)
//{
//	num = round(num * pow(10.0, (double)num2)) / pow(10.0, (double)num2);
//	return num;
//}
//
////    This function return the sign of a number.
//int sgn(double num)
//{
//	if (num < 0) return -1;
//	if (num > 0) return 1;
//	return 0;
//}
//
////    Decimal sine
//double dsin(double num)
//{
//	return (double)sin(num * PI / (double)180);
//}
//
////    Decimal cosine
//double dcos(double num)
//{
//	return (double)cos(num*PI/180);
//}
//
////    Decimal tangent
//double dtan(double num)
//{
//	return (double)tan(num*PI / (double)180);
//}
//
////    Decimal arcsine
//double darcsin(double num)
//{
//	double y;
//	if (num == 1)
//	{
//		y = 1.570796327F;
//	}
//	else
//	{
//		y = atan(num / sqrt((double)-1 * num * num + 1));
//	}
//	return y*(double)180/PI;
//}
//
////    Decimal arccosine
//double darccos(double num)
//{
//	double y;
//	if (num == 1)
//	{
//		y = 0;
//	}
//	else
//	{
//		y = 1.570796327F - atan(num / sqrt((double)-1 * num * num + 1));
//	}
//	return y * (double)180 / PI;
//}
//
////    Decimal arctangent
//double datan(double num)
//{
//	return atan(num)*(double)180/PI;
//}
//
////    Decimal arctangent2
//double datan2(double y, double x)
//{
//	double theta=0;
//	if (x > 0)
//	{
//		theta = atan(y / x);
//	}
//	else if (y >= 1 && x < 0)
//	{
//		theta = atan(y / x) + PI;
//	}
//	else if (y < 0 && x < 0)
//	{
//		theta = atan(y / x) - PI;
//	}
//	else if (y > (double)0.0 && x == (double)0.0)
//	{
//		theta = PI / 2;
//	}
//	else if (y < (double)0.0 && x == (double)0.0)
//	{
//		theta = -PI / 2;
//	}
//	else if (y == (double)0.0 && x == (double)0.0)
//	{
//		//undefined
//	}
//
//	return theta * (double)180 / PI;// Math.Atan2((double)y, (double)x) * 180 / Math.PI;
//}
//
//
//
////This function normalizes the number to 360 degrees
//double rev(double x)
//{
//	return x - floor(x / 360.0) * 360.0;
//}
//
//
////    Mathematic section exit.
////    Planet routines section.
//
////    This function returns location of specified planet
//void planet(int desc, double LST, double lat, double Rs, double M, double e,double i, double a, double w, double d, double N, double Ls, double Ms, double xsun, double ysun, double zsun, double *slon, double *slat)
//{
//	//desc = 1 = "Moon"
//	//desc = 2 = "Jupiter
//	//desc = 3 = "Saturn"
//	//desc = 4 = "Uranus"
//
//	double x, y, z, r, v;
//	double mlon=0;
//	double mlat=0;
//	double pRA, pDecl;
//
//	double oblecl = (double)23.4393 - (double)3.563E-7 * d;
//	M = rev(M);
//	//Itteratively compute E, the eccentric anomaly.
//	double E = M + (double)(180 / PI) * e * dsin(M) * (1 + e * dcos(M));
//	double E0 = 0; double E1 = E;
//	while (fabs(E0 - E1) > .005)
//	{
//		E0 = E1;
//		E1 = E0 - (E0 - (double)(180 /PI) * e * dsin(E0) - M) / (1 - e * dcos(E0));
//	}
//	E = E1;
//
//	//compute rectangular (x,y) coordinates in the plane of the lunar orbit:
//	x = a * (dcos(E) - e);
//	y = a * (dsin(E) * sqrt(1 - e * e));
//
//	//convert this to distance and true anonaly
//	r = sqrt(x * x + y * y);
//	v = datan2(y, x);
//
//	// need for phase
//	double r0 = r;
//
//	//compute the Moon's position in ecliptic coordinates
//	x = r * (dcos(N) * dcos(v + w) - dsin(N) * dsin(v + w) * dcos(i));
//	y = r * (dsin(N) * dcos(v + w) + dcos(N) * dsin(v + w) * dcos(i));
//	z = r * dsin(v + w) * dsin(i);
//
//
//	//perturbations(desc , M, w,ref mlon,ref mlat,ref slon,ref slat); //************************
//	double lonecl = rev(datan2(y, x));
//	double latecl = datan2(z, sqrt(x * x + y * y));
//	r = sqrt((x * x + y * y + z * z)); //Earth radii
//
//	if (desc == 1)//"Moon"
//	{
//		double Mm = M - floor(M / (double)360.0) * (double)360.0; //rev(M);
//		double Lm = rev(N + w + M);
//		double D = Lm - Ls;
//		double F = Lm - N;
//		lonecl = lonecl - (double)1.274 * dsin(Mm - 2 * D);
//		lonecl = lonecl + (double)0.658 * dsin(2 * D);
//		lonecl = lonecl - (double)0.186 * dsin(Ms);
//		lonecl = lonecl - (double)0.059 * dsin(2 * Mm - 2 * D);
//		lonecl = lonecl - (double)0.057 * dsin(Mm - 2 * D + Ms);
//		lonecl = lonecl + (double)0.053 * dsin(Mm + 2 * D);
//		lonecl = lonecl + (double)0.046 * dsin(2 * D - Ms);
//		lonecl = lonecl + (double)0.041 * dsin(Mm - Ms);
//		lonecl = lonecl - (double)0.035 * dsin(D);
//		lonecl = lonecl - (double)0.031 * dsin(Mm + Ms);
//		lonecl = lonecl - (double)0.015 * dsin(2 * F - 2 * D);
//		lonecl = lonecl + (double)0.011 * dsin(Mm - 4 * D);
//
//		latecl = latecl - (double)0.173 * dsin(F - 2 * D);
//		latecl = latecl - (double)0.055 * dsin(Mm - F - 2 * D);
//		latecl = latecl - (double)0.046 * dsin(Mm + F - 2 * D);
//		latecl = latecl + (double)0.033 * dsin(F + 2 * D);
//		latecl = latecl + (double)0.017 * dsin(2 * Mm + F);
//
//		r = r - (double)0.58 * dcos(Mm - 2 * D);
//		r = r - (double)0.46 * dcos(2 * D);
//
//		// needed for phase
//		mlon = lonecl;
//		mlat = latecl;
//	}
//	if (desc == 2)//"Jupiter"
//	{
//		double Mjp = rev((double)19.8950 + (double)0.0830853001 * d);
//		double Mst = rev((double)316.9670 + (double)0.0334442282 * d);
//		//double Mun = rev((double)142.5905 + (double)0.011725806 * d);
//		lonecl = lonecl - (double)0.332 * dsin(2 * Mjp - 5 * Mst - (double)67.6);
//		lonecl = lonecl - (double)0.056 * dsin(2 * Mjp - 2 * Mst + 21);
//		lonecl = lonecl + (double)0.042 * dsin(3 * Mjp - 5 * Mst + 21);
//		lonecl = lonecl - (double)0.036 * dsin(Mjp - 2 * Mst);
//		lonecl = lonecl + (double)0.022 * dcos(Mjp - Mst);
//		lonecl = lonecl + (double)0.023 * dsin(2 * Mjp - 3 * Mst + 52);
//		lonecl = lonecl - (double)0.016 * dsin(Mjp - 5 * Mst - 69);
//	}
//	if (desc == 3)///"Saturn"
//	{
//		double Mjp = rev((double)19.8950 + (double)0.0830853001 * d);
//		double Mst = rev((double)316.9670 + (double)0.0334442282 * d);
//		//double Mun = rev((double)142.5905 + (double)0.011725806 * d);
//		lonecl = lonecl + (double)0.812 * dsin(2 * Mjp - 5 * Mst - (double)67.6);
//		lonecl = lonecl - (double)0.229 * dcos(2 * Mjp - 4 * Mst - 2);
//		lonecl = lonecl + (double)0.119 * dsin(Mjp - 2 * Mst - 3);
//		lonecl = lonecl + (double)0.046 * dsin(2 * Mjp - 6 * Mst - 69);
//		lonecl = lonecl + (double)0.014 * dsin(Mjp - 3 * Mst + 32);
//
//		latecl = latecl - (double)0.020 * dcos(2 * Mjp - 4 * Mst - 2);
//		latecl = latecl + (double)0.018 * dsin(2 * Mjp - 6 * Mst - 49);
//
//		// needed for magnitude
//		*slon = lonecl;
//		*slat = latecl;
//	}
//	if (desc == 4)//"Uranus"
//	{
//		double Mjp = rev((double)19.8950 + (double)0.0830853001 * d);
//		double Mst = rev((double)316.9670 + (double)0.0334442282 * d);
//		double Mun = rev((double)142.5905 + (double)0.011725806 * d);
//		lonecl = lonecl + (double)0.040 * dsin(Mst - 2 * Mun + 6);
//		lonecl = lonecl + (double)0.035 * dsin(Mst - 3 * Mun + 33);
//		lonecl = lonecl - (double)0.015 * dsin(Mjp - Mun + 20);
//	}
//	x = r * dcos(lonecl) * dcos(latecl);
//	y = r * dsin(lonecl) * dcos(latecl);
//	z = r * dsin(latecl);
//	//******************* end of perturbations *******************
//
//
//
//	//x = px; y = py; z = pz;
//
//	if (desc != 1) { x = x + xsun; y = y + ysun; z = z + zsun; }
//
//	double x0 = x;
//	double y0 = y * dcos(oblecl) - z * dsin(oblecl);
//	double z0 = y * dsin(oblecl) + z * dcos(oblecl);
//
//	r = sqrt(x0 * x0 + y0 * y0 + z0 * z0);
//
//	// need for phase
//	double R0 = r;
//
//	double RA = datan2(y0, x0)/ 15;
//	while (RA > 24) { RA = RA - 24; }
//	while (RA < 0) { RA = RA + 24; }
//	double Decl = datan2(z0, sqrt(x0 * x0 + y0 * y0));
//
//	pRA = RA;
//	pDecl = Decl;
//
//
//	//topocentric(desc, r); *********************
//	RA = pRA;
//	Decl = pDecl;
//	double mpar = darcsin(1 / r); if (desc != 1) mpar = ((double)8.794 / 3600) / r;
//	double gclat = lat - (double)0.1924 * dsin(2 * lat);
//	double rho = (double)0.99883 + (double)0.00167 * dcos(2 * lat);
//	double HA = (LST - RA) * 15;
//	double g = datan(dtan(gclat) / dcos(HA));
//	pRA = RA * 15 - mpar * rho * dcos(gclat) * dsin(HA) / dcos(Decl);
//	pRA = pRA / 15;
//	while (pRA > 24) { pRA = pRA - 24; }
//	while (pRA < 0) { pRA = pRA + 24; }
//	pDecl = Decl - mpar * rho * dsin(gclat) * dsin(g - Decl) / dsin(g);
//
//	//************* topocentric end *****************
//
//
//	Decl = pDecl;
//	RA = pRA;
//
//	HA = (LST - RA) * 15;
//
//	x = dcos(HA) * dcos(Decl);
//	y = dsin(HA) * dcos(Decl);
//	z = dsin(Decl);
//
//
//	double xhor = x * dsin(lat) - z * dcos(lat);
//	double yhor = y;
//	double zhor = x * dcos(lat) + z * dsin(lat);
//
//	az = datan2(yhor, xhor) + 180;
//	al = darcsin(zhor);
//
//
//	/*
//	double elong = darccos((Rs * Rs + R0 * R0 - r0 * r0) / (2 * Rs * R0));
//	double FV = darccos((r0 * r0 + R0 * R0 - Rs * Rs) / (2 * r0 * R0));
//	if (desc == 1)
//	{
//		elong = darccos(dcos(*slon - mlon) * dcos(mlat));
//		FV = 180 - elong;
//	}
//	double phase = (1 + dcos(FV)) / 2;
//	*/
//}
//
//
//
////Planet routines section exit.
//
//
//
////    cMoon routines section.
//double cMoon_day(double Lm, double Ls, double Mm, double Ms)
//{
//	// Calculate illumination (synodic) phase
//	double ag = (double)rnd((double)(cMoon_long(Lm,Ls,Mm,Ms) / 360.0D) * 29.0D, 0.0D);
//	if (ag <= 0.0D || ag >= 29.0D) ag = 0.0D;
//	return ag;
//}
//
//
//double cMoon_long(double Lm, double Ls, double Mm, double Ms)
//{
//	double D = Lm - Ls;
//	double x = (double)6.3 * dsin(Mm);
//	x = x + (double)1.3 * dsin(2.0D * D - Mm);
//	x = x + (double)0.7 * dsin(2.0D * D);
//	x = x - (double)1.9 * dsin(Ms);
//	return (double)rev(D + x);
//}
//
//double cMoon_phase(double ag)
//{
//	double iag = 0;
//	if (ag >= 1 && ag <= 6) iag = 1;
//	if (ag == 7) iag = 2;
//	if (ag >= 8 && ag <= 13) iag = 3;
//	if (ag >= 14 && ag <= 15) iag = 4;
//	if (ag >= 16 && ag <= 20) iag = 5;
//	if (ag == 21) iag = 6;
//	if (ag >= 22 && ag <= 28) iag = 7;
//	return iag;
//}
//
////    cMoon routines section exit.
//
////    The purpose of this function is to return all calculations.
//void calculate(int year, int month, int day, int hr, int mn, int sc, double latitude, double longitude )
//{
//	// http://stjarnhimlen.se/comp/tutorial.html#7
//	double r;
//	double xhor, yhor, zhor;
//	double GST, HA, sunal;
//	double x, y, z, v;
//	double j;
//	double UT, oblecl, L, E0, E1;
//	double w, a, e, M;
//	double xsun, ysun, zsun;
//	double E, i;
//	double slon, slat;
//	double d, N,  Ls, Ms, Lm, Mm;
//	double LST, lat, lon, Rs;
//
//	lat = latitude;
//	lon = longitude;
//
//	//Calculate Universal Time
//	UT = hr + mn / (double)60 + sc / (double)3600;
//	//Calculate Julian Time
//	j = (year * (double)365.25) + (double)DayOfYear(year, month, day) + UT / 24.0 + 1721044.0;
//	//Calculate number of days from 2000 Jan 0.0 TDT, which is the same as 1999 Dec 31.0 TDT, i.e. precisely at midnight TDT at the start of the last day of this century.
//	d = j - (double)2451543.5;
//
//// sun
//	w = (double)282.9404 + (double)4.70935E-5 * d;      //longitude of perihelion
//	a = (double)1.000000;                               //mean distance, a.u.
//	e = (double)0.016709 - (double)1.151E-9 * d;        //eccentricity
//	M = rev((double)356.0470 + (double)0.9856002585 * d);    //mean anomaly
//
//	M = rev(M);
//	L = rev(w + M);  //Sun's mean longitude
//
//	// needed for perturbations or moon day calc
//	Ms = M;
//	Ls = L;
//
//
////redundant code...
//	oblecl = (double)23.4393 - (double)3.563E-7 * d; //obliquity of the ecliptic
//	M = rev(M);
//	//itterate to find E
//	E = M + (180 / PI) * e * dsin(M) * (1 + e * dcos(M));
//	E0 = 0; E1 = E;
//	while (fabs(E0 - E1) > .005)
//	{
//		E0 = E1;
//		E1 = E0 - (E0 - (180 / PI) * e * dsin(E0) - M) / (1 - e * dcos(E0));
//	}
//	E = E1;
//	//Sun's rectangular coordinates in the plane of the ecliptic, where the X axis points towards the perihelion:
//	x = a * (dcos(E) - e);
//	y = a * (dsin(E) * sqrt(1 - e * e));
//	//Convert to distance and true anomaly:
//	r = sqrt(x * x + y * y);
//	v = datan2(y, x);
/////
//
//	slon = rev(v + w);
//
//	// needed for phase angle
//	xsun = r * dcos(slon);
//	ysun = r * dsin(slon);
//	zsun = (double)0.0;
//	Rs = r;
//	//slon = rev(datan2(y, x));
//	slat = datan2( zsun, sqrt(xsun * xsun + ysun * ysun));
//
//	//// need for phase
//	GST = rev((L / 15) + 12) ;//Sidereal Time at the Greenwich meridian
//	while (GST > 24) { GST = GST - 24; }
//	while (GST < 0) { GST = GST + 24; }
//
//	LST = GST + UT + lon / 15; //Local Siderial Time:
//	while (LST > 24) { LST = LST - 24; }
//	while (LST < 0) { LST = LST + 24; }
//
//	// sun az/al
//	double x0 = xsun;
//	double y0 = ysun * dcos(oblecl) - zsun * dsin(oblecl);
//	double z0 = ysun * dsin(oblecl) + zsun * dcos(oblecl);
//
//	r = sqrt(x0 * x0 + y0 * y0 + z0 * z0);
//	double RA = datan2(y0, x0) / 15;
//	while (RA > 24) { RA = RA - 24; }
//	while (RA < 0) { RA = RA + 24; }
//	double Decl = datan2(z0, sqrt(x0 * x0 + y0 * y0));
//
//	HA = (LST - RA) * 15;
//
//	x = dcos(HA);
//	y = dsin(HA) * dcos(Decl);
//	z = dsin(Decl);
//
//	xhor = x * dsin(lat) - z * dcos(lat);
//	yhor = y;
//	zhor = x * dcos(lat) + z * dsin(lat);
//
//	az = datan2(yhor, xhor) + 180;
//	al = darcsin(zhor); sunal = al;
//
//	// Moon
//	N = (double)125.1228 - (double)0.0529538083 * d;
//	i = (double)5.1454;
//	w = (double)318.0634 + (double)0.1643573223 * d;
//	a = (double)60.2666;
//	e = (double)0.054900;
//	M = (double)115.3654 + (double)13.0649929509 * d;
//	Mm = rev(M);
//	Lm = rev(N + w + M); // moon day calc
//
//	planet(1, LST,lat,Rs, M, e, i, a, w, d,N,Ls,Ms, xsun, ysun, zsun, &slon, &slat);//"Moon" = 1
//
//	//txtAltitude.Text = Microsoft.VisualBasic.Conversion.Str(al);
//	//txtAzimuth.Text = Microsoft.VisualBasic.Conversion.Str(az);
//}
//
//
//int DayOfYear(int year, int month, int day)
//{
//	int num = day;
//	int year4 = year / 4;
//	int year100 = year / 100;
//	int year400 = year / 400;
//
//	if (month > 1) num += 31; //January
//	if (month > 2) //February
//	{
//		num += 28;
//		//Check for leap year
//		if (year == year4 * 4 & year != year100 * 100 & year != year400 * 400)
//			num++;
//	}
//	if (month > 3) num += 31; //March
//	if (month > 4) num += 30; //April
//	if (month > 5) num += 31; //May
//	if (month > 6) num += 30; //June
//	if (month > 7) num += 31; //July
//	if (month > 8) num += 31; //August
//	if (month > 9) num += 30; //September
//	if (month > 10) num += 31; //October
//	if (month > 11) num += 30; //November
//	if (month > 12) num += 31; //December
//
//	return num;
//}
