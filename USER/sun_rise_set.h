#ifndef __SUN_RISE_SET_H
#define __SUN_RISE_SET_H

#include "sys.h"
#include <math.h>

#define PI 3.1415926


typedef struct SunRiseSetTime		//日出日落时间
{
	u8 rise_h;
	u8 rise_m;
	
	u8 set_h;
	u8 set_m;
}SunRiseSetTime_S;


double GetDayLength(u16 Year,u8 Month,u8 Day, double longitude, double latitude);//longitude经度latitude纬度
SunRiseSetTime_S GetSunTime(u16 year,u8 month,u8 day, double longitude, double latitude);
double DayLen(u16 year, u8 month, u8 day, double lon, double lat,double altit, s32 upper_limb);
void ToLocalTime(u8*time, double utTime);
void Sunpos(double d,double *lon,double *r);
void Sun_RA_dec(double d, double *RA, double *dec, double *r);
void SunRiset(u16 year, u8 month, u8 day, double lon, double lat,double altit, s32 upper_limb, double* trise, double* tset);
u32 Days_since_2000_Jan_0(u16 y, u8 m, u8 d);
double Revolution(double x);
double Rev180(double x);
double GMST0(double d);
double Sind(double x);
double Cosd(double x);
double Tand(double x);
double Atand(double x);
double Asind(double x);
double Acosd(double x);
double Atan2d(double y, double x);















































#endif
