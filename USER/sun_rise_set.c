#include "sun_rise_set.h"



double Radge = 180.0 / PI;
double Degrad = PI / 180.0;
double Inv360 = 1.0 / 360.0;
double sr,slon;
double r,lon;
double sRA,sdec;
double start;
double end;
s8 rc;
u8 sunrise[2];
u8 sunset[2];
u16 Year;
u8 Month,Day;


// 计算日出日没时间(只需调用此函数)
// </summary>
// <param name="date">日期</param>
// <param name="longitude">经度</param>
// <param name="latitude">纬度</param>
// <returns>日落日出时间</returns>
// <remarks>
// 注：日期最小为2000.1.1号
// 应用时候只调用此函数即可，得到的日出时间存储在数组sunrise中sunrise[0]为小时，sunrise[1]为分钟
// 日落时间存储在数组sunset中，sunset[0]为小时，sunset[1]为分钟，均为十六进制，需换算
// </remarks>
SunRiseSetTime_S GetSunTime(u16 year,u8 month,u8 day, double longitude, double latitude)
{
	SunRiseSetTime_S sun;

    start = 0.0;
    end = 0.0;

    SunRiset(year, month, day, longitude, latitude, -35.0 / 60.0, 1, &start, &end);

    ToLocalTime(&sunrise[0], start);
    ToLocalTime(&sunset[0], end);

	sun.rise_h = sunrise[0];
	sun.rise_m = sunrise[1];

	sun.set_h = sunset[0];
	sun.set_m = sunset[1];

	return sun;
}

u32 Days_since_2000_Jan_0(u16 y, u8 m, u8 d)
{
    return (367L * (y) - ((7 * ((y) + (((m) + 9) / 12))) / 4) + ((275 * (m)) / 9) + (d) - 730530L);
}

double Revolution(double x)
{
    return (x - 360.0 * (floor(x * Inv360)));
}

double Rev180(double x)
{
    return (x - 360.0 * (floor(x * Inv360 + 0.5)));
}

double GMST0(double d)
{
    double sidtim0;

    sidtim0 = Revolution((180.0 + 356.0470 + 282.9404) +
              (0.9856002585 + 4.70935E-5) * d);

    return sidtim0;
}

 double Sind(double x)
{
    return sin(x * Degrad);
}

double Cosd(double x)
{
    return cos(x * Degrad);
}

double Tand(double x)
{
    return tan(x * Degrad);
}

double Atand(double x)
{
    return Radge * atan(x);
}

double Asind(double x)
{
    return Radge * asin(x);
}

double Acosd(double x)
{
    return Radge * acos(x);
}

double Atan2d(double y, double x)
{
    return Radge * atan2(y, x);
}

//longitude经度latitude纬度
double GetDayLength(u16 Year,u8 Month,u8 Day, double longitude, double latitude)
{
    double result = DayLen(Year, Month, Day, longitude, latitude, -35.0 / 60.0, 1);

    return result;
}

//utTime ：start,end
void ToLocalTime(u8*time, double utTime)
{
    s8 hour = (s8)(floor(utTime));
	s8 minute = 0;
    double temp = utTime - hour;

    hour += 8;						//转换为东8区北京时间
    temp = temp * 60;
    minute = (s8)(floor(temp));
    *time = hour;
    time++;
    *time = minute;
}

double DayLen(u16 year, u8 month, u8 day, double lon, double lat,double altit, s32 upper_limb)
{
    double d,  			/* Days since 2000 Jan 0.0 (negative before) */
           obl_ecl,    	/* Obliquity (inclination) of Earth's axis */
           sin_sdecl,  	/* Sine of Sun's declination */
           cos_sdecl,  	/* Cosine of Sun's declination 太阳赤纬的余铉*/
           sradius,    	/* Sun's apparent radius 太阳表面半径*/
           t,			/* Diurnal arc 昼日弧*/
	       cost;

    /* Compute d of 12h local mean solar time */
    d = Days_since_2000_Jan_0(year, month, day) + 0.5 - lon / 360.0;

    /* Compute obliquity of ecliptic (inclination of Earth's axis) */
    obl_ecl = 23.4393 - 3.563E-7 * d;
    //这个黄赤交角时变公式来历复杂，很大程度是经验性的，不必追究。

    /* Compute Sun's position */
    slon = 0.0;
    sr = 0.0;
    Sunpos(d,&slon,&sr);

    /* Compute sine and cosine of Sun's declination */
    sin_sdecl = Sind(obl_ecl) * Sind(slon);
    cos_sdecl = sqrt(1.0 - sin_sdecl * sin_sdecl);
    //用球面三角学公式计算太阳赤纬。

    /* Compute the Sun's apparent radius, degrees */
    sradius = 0.2666 / sr;
    //视半径，同前。

    /* Do correction to upper limb, if necessary */
    if (upper_limb != 0)
	{
		altit -= sradius;
	}

    /* Compute the diurnal arc that the Sun traverses to reach */
    /* the specified altitide altit: */
    //根据设定的地平高度判据计算周日弧长。

    cost = (Sind(altit) - Sind(lat) * sin_sdecl) /
           (Cosd(lat) * cos_sdecl);

    if (cost >= 1.0)
	{
		t = 0.0;                      /* Sun always below altit */
	}
    else if (cost <= -1.0)	//极夜。
	{
		t = 24.0;                     /* Sun always above altit */
	}

    else					//极昼。
	{
		t = (2.0 / 15.0) * Acosd(cost); /* The diurnal arc, hours */
	}

    //周日弧换算成小时计。
    return t;

}

void Sunpos(double d,double *lon,double *r)
{
	double M,//太阳的平均近点角，从太阳观察到的地球（=从地球看到太阳的）距近日点（近地点）的角度。
	       w, //近日点的平均黄道经度。
	       e, //地球椭圆公转轨道离心率。
	       E, //太阳的偏近点角。计算公式见下面。
	       x, y,
	       v;  //真近点角，太阳在任意时刻的真实近点角。


	M = Revolution(356.0470 + 0.9856002585 * d);	//自变量的组成：2000.0时刻太阳黄经为356.0470度,此后每天约推进一度（360度/365天
	w = 282.9404 + 4.70935E-5 * d;					//近日点的平均黄经。

	e = 0.016709 - 1.151E-9 * d;					//地球公转椭圆轨道离心率的时间演化。以上公式和黄赤交角公式一样，不必深究。

	E = M + e * Radge * Sind(M) * (1.0 + e * Cosd(M));
	x = Cosd(E) - e;
	y = sqrt(1.0 - e * e) * Sind(E);
	*r = sqrt(x * x + y * y);
	v = Atan2d(y, x);
	*lon = v + w;

	if (*lon >= 360.0)
	{
		*lon -= 360.0;
	}
}

void Sun_RA_dec(double d, double *RA, double *dec, double *r)
{
	double obl_ecl, x, y, z;

	Sunpos(d, &lon, r);
	//计算太阳的黄道坐标。

	x = (*r)* Cosd(lon);
	y = (*r) * Sind(lon);
	//计算太阳的直角坐标。

	obl_ecl = 23.4393 - 3.563E-7 * d;
	//黄赤交角，同前。

	z = y * Sind(obl_ecl);
	y = y * Cosd(obl_ecl);
	//把太阳的黄道坐标转换成赤道坐标（暂改用直角坐标）。

	*RA = Atan2d(y, x);
	*dec = Atan2d(z, sqrt(x * x + y * y));
	//最后转成赤道坐标。显然太阳的位置是由黄道坐标方便地直接确定的，但必须转换到赤
	//道坐标里才能结合地球的自转确定我们需要的白昼长度。

}

// <summary>
// 日出没时刻计算
// </summary>
// <param name="year">年</param>
// <param name="month">月</param>
// <param name="day">日</param>
// <param name="lon"></param>
// <param name="lat"></param>
// <param name="altit"></param>
// <param name="upper_limb"></param>
// <param name="trise">日出时刻</param>
// <param name="tset">日没时刻</param>
// <returns>太阳有出没现象，返回0 极昼，返回+1 极夜，返回-1</returns>
void SunRiset(u16 year, u8 month, u8 day, double lon, double lat,double altit, s32 upper_limb, double* trise, double* tset)
{
	double d,			/* Days since 2000 Jan 0.0 (negative before) */
	       sradius,    	/* Sun's apparent radius */
	       t,          	/* Diurnal arc */
	       tsouth,     	/* Time when Sun is at south */
	       sidtime,    	/* Local sidereal time */
		   cost;

	       rc = 0;		/* Return cde from function - usually 0 */

	/* Compute d of 12h local mean solar time */
	d = Days_since_2000_Jan_0(year, month, day) + 0.5 - lon / 360.0;
	//计算观测地当日中午时刻对应2000.0起算的日数。

	/* Compute local sideral time of this moment */
	sidtime = Revolution(GMST0(d) + 180.0 + lon);
	//计算同时刻的当地恒星时（以角度为单位）。以格林尼治为基准，用经度差校正。

	/* Compute Sun's RA + Decl at this moment */
	sRA = 0.0;
	sdec = 0.0;
	sr = 0.0;
	Sun_RA_dec(d, &sRA, &sdec, &sr);
	//计算同时刻太阳赤经赤纬。

	/* Compute time when Sun is at south - in hours UT */
	tsouth = 12.0 - Rev180(sidtime - sRA) / 15.0;
	//计算太阳日的正午时刻，以世界时（格林尼治平太阳时）的小时计。

	/* Compute the Sun's apparent radius, degrees */
	sradius = 0.2666 / sr;
	//太阳视半径。0.2666是一天文单位处的太阳视半径（角度）。

	/* Do correction to upper limb, if necessary */
	if (upper_limb != 0)
	{
		altit -= sradius;
	}
	//如果要用上边缘，就要扣除一个视半径。

	/* Compute the diurnal arc that the Sun traverses to reach */
	//计算周日弧。直接利用球面三角公式。如果碰到极昼极夜问题，同前处理。
	/* the specified altitide altit: */

	cost = (Sind(altit) - Sind(lat) * Sind(sdec)) /
	       (Cosd(lat) * Cosd(sdec));

	if (cost >= 1.0)
	{
		rc = -1;
		t = 0.0;
	}
	else
	{
		if (cost <= -1.0)
		{
			rc = +1;
			t = 12.0;      /* Sun always above altit */
		}
		else
		{
			t = Acosd(cost) / 15.0;   /* The diurnal arc, hours */
		}
	}

	/* Store rise and set times - in hours UT */
	*trise = tsouth - t;
	*tset = tsouth + t;
}
















































