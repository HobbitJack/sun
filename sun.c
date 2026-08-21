#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "sun.g.h"

#ifndef DATADIR
#define DATADIR "/usr/local/share/sun/"
#endif

#define ERRBUF_S(PATH) strlen(progname) + strlen(PATH) + 3

#define RAD (atan(1)/45)
#define DEG (45/atan(1))

#define BUFSIZE 64

#define MOD(VAL, EXPR, MODULUS) {                              \
                                   VAL = fmod(EXPR, MODULUS);  \
                                   if (VAL < 0)                \
                                   	VAL += MODULUS;        \
                                }

#define PARSE_CITY_STR(CITY) {                            \
				parse_city(CITY, &city);  \
				if (city == NULL)         \
					errno = 1;        \
			     }

#define TOKEN_CITY_STR(CITY) {                                                   \
				name = strtok(CITY, "\t");                       \
				lattitude = strtod(strtok(NULL, "\t"), &buf);    \
				if (*buf != '\0')                                \
                                	errno = 1;                               \
				longitude = strtod(strtok(NULL, "\t\n"), &buf);  \
				if (*buf != '\0')                                \
                                	errno = 1;                               \
                             }


static char *progname;
static int status;

static struct gengetopt_args_info args;

time_t
parse_date(char *str)
{
	struct tm t = { 0 };
	char *end;
	time_t date;

	date = time(NULL);
	if (args.utc_given)
		gmtime_r(&date, &t);
	else
		localtime_r(&date, &t);

	if ((end = strptime(str, "%x", &t)) && (*end == '\0'))
		return mktime(&t);
	else if ((end = strptime(str, "%Ex", &t)) && (*end == '\0'))
		return mktime(&t);
	else if ((end = strptime(str, "%Y-%m-%d", &t)) && (*end == '\0'))
		return mktime(&t);
	else if ((end = strptime(str, "%Y/%m/%d", &t)) && (*end == '\0'))
		return mktime(&t);
	else if ((end = strptime(str, "%Y %m %d", &t)) && (*end == '\0'))
		return mktime(&t);
	else if ((end = strptime(str, "%b %d %y", &t)) && (*end == '\0'))
		return mktime(&t);
	else if ((end = strptime(str, "%b %d", &t)) && (*end == '\0'))
		return mktime(&t);
	
	fprintf(stderr, "%s: %s: Could not unambiguously parse date string\n", progname, str);
	exit(1);
}

double
parse_seg(char *seg)
{
	int h, m;
	double s;
	
	if (sscanf(seg, "%d:%d:%lf", &h, &m, &s) == 3)
		return (abs(h) + abs(m/60) + abs(s/3600)) * copysign(1, h);
	else if (sscanf(seg, "%d.%d.%lf", &h, &m, &s) == 3)
		return (abs(h) + abs(m/60) + abs(s/3600)) * copysign(1, h);
	else if (sscanf(seg, "%lf", &s) == 1)
		return s;

	fprintf(stderr, "%s: %s: Invalid sexigesimal literal\n", progname, seg);
	exit(1);
}

void
parse_city(char *city, char **out)
{
	FILE *fp;
	char *buf1, *buf2;
	char *line;

	size_t n;
	ssize_t r;

	fp = fopen(DATADIR "cities.dat", "r");
	if (fp == NULL)
	{
		fprintf(stderr, "%s: %s: No such file, or file not readable\n", progname, DATADIR "cities.dat");
		exit(1);
	}

	n = 0;

	while ((r = getline(&line, &n, fp)) != -1)
	{
		buf1 = malloc(sizeof(char) * (strlen(line)+1));
		strcpy(buf1, line);
		buf2 = strtok(buf1, "\t");

		if (!strcasecmp(city, buf2))
		{
			*out = malloc(sizeof(char) * (strlen(line)+1));
			strcpy(*out, line);
			free(line);
			free(buf1);
			return;
		}
	}

	out = NULL;
	return;
}

time_t
jd2posix(double jd)
{
	return (time_t)trunc((jd - 2440587.5) * 86400);
}

double
posix2jd(time_t ts)
{
	return ts/86400.0 + 2440587.5;
}

time_t*
compute_times(double lattitude, double longitude, double elevation, time_t date, int sunangle)
{
	time_t *times = malloc(2 * sizeof(time_t));
	double n, J, M, C, l, T, D, cosH;

	n = ceil(posix2jd(date) - 2451545 + 0.0008);

	//mean solar time
	J = n - longitude/360;

	//mean anomaly
	MOD(M, 357.5291 + 0.98560028 * J, 360)

	//equation of the center
	C = 1.9148 * sin(M*RAD) + 0.02 * sin(2 * M * RAD) + 0.0003 * sin(3 * M * RAD);

	//ecliptic longitude
	MOD(l, M + C + 180 + 102.9372, 360)

	//solar noon
	T = 2451545 + J + 0.0053*sin(M * RAD) - 0.0069*sin(2 * l * RAD);

	//declination
	D = asin(sin(l * RAD) * sin(23.4397 * RAD)) * DEG;
	
	//cosine of hour angle
	cosH = (sin(-RAD * (0.833 + sunangle + copysign(1.76, elevation)*sqrt(fabs(elevation))/60)) - (sin(lattitude * RAD) * sin(D * RAD)))/(cos(lattitude * RAD) * cos(D * RAD));
	
	if (abs(cosH) > 1)
		return NULL;

	times[0] = jd2posix(T - acos(cosH)*DEG/360);
	times[1] = jd2posix(T + acos(cosH)*DEG/360);
	
	return times;
}

void
print_table()
{
	if (args.astronomical_given || args.nautical_given || args.civil_given)
		printf("Dawn\tDusk\n");
	else
		printf("Sunrise\tSunset\n");
	return;
}

void
print_table_city()
{
	printf("City\t");
	print_table();
	return;
}

void
print_times(time_t *times)
{
	struct tm tm1;
	struct tm tm2;
	char b1[BUFSIZE] = { 0 };
	char b2[BUFSIZE] = { 0 };
	
	if (times == NULL)
	{
		printf("No sunset or sunrise\n");
		return;
	}

	if (args.utc_given)
	{
		gmtime_r(&times[0], &tm1);
		strftime(b1, BUFSIZE-1, "%X", &tm1);
		gmtime_r(&times[1], &tm2);
		strftime(b2, BUFSIZE-1, "%X", &tm2);
	}
	else
	{
		localtime_r(&times[0], &tm1);
		strftime(b1, BUFSIZE-1, "%X", &tm1);
		localtime_r(&times[1], &tm2);
		strftime(b2, BUFSIZE-1, "%X", &tm2);
	}

	printf("%s\t%s\n", b1, b2);		
	return;
}

void
print_times_city(char *city, time_t *times)
{
	if (args.table_given)
		printf("%s\t", city);
	else
		printf("%s: ", city);
	print_times(times);
	return;
}

int
main(int argc, char *argv[])
{
	int i;
	int sunangle;
	double lattitude, longitude, elevation;
	char *name;
	char *city;
	char *buf;
	time_t *times;
	time_t date;

	setlocale(LC_ALL, "");

	progname = basename(argv[0]);
	status = 0;

	if (ggo(argc, argv, &args))
		return 1;

	if (args.help_given)
	{
		ggo_print_help();
		return 0;
	}

	if (args.version_given)
	{
		ggo_print_version();
		return 0;
	}

	if (args.date_given)
		date = parse_date(args.date_arg);
	else
		date = time(NULL);

	if (args.civil_given)
		sunangle = 6;
	if (args.nautical_given)
		sunangle = 12;
	if (args.astronomical_given)
		sunangle = 18;

	if (args.inputs_num)
	{
		if (args.table_given) print_table_city();
		for (i=0; i<args.inputs_num; i++)
		{
			city = NULL;
			errno = 0;
			
			PARSE_CITY_STR(args.inputs[i])
			if (errno)
			{
				fprintf(stderr, "%s: %s: Not in database\n", progname, args.inputs[i]);
				status = 1;
				continue;
			}
			errno = 0;
				
			TOKEN_CITY_STR(city)
			if (errno)
			{
				fprintf(stderr, "%s: %s: Could not parse\n", progname, args.inputs[i]);
				status = 1;
				continue;
			}
			errno = 0;		

			times = compute_times(lattitude, longitude, args.elevation_given ? args.elevation_arg : 0, date, sunangle);
			print_times_city(name, times);
		}

		return status;
	}

	if (args.lattitude_given && args.longitude_given)
	{
		lattitude = parse_seg(args.lattitude_arg);
		longitude = parse_seg(args.longitude_arg);
	}
	else if (args.lattitude_given || args.longitude_given)
	{
		fprintf(stderr, "%s: Lattitude and longitude must be given together\n", progname);
		return 1;
	}
	else
	{
		if (getenv("SUN_HOME_CITY") == NULL)
		{
			fprintf(stderr, "%s: Missing operand\n", progname);
			return 1;
		}

		city = NULL;
		errno = 0;

		PARSE_CITY_STR(getenv("SUN_HOME_CITY"))
		if (errno)
		{
			fprintf(stderr, "%s: %s: Not in database\n", progname, getenv("SUN_HOME_CITY"));
			return 1;
		}
		errno = 0;
				
		TOKEN_CITY_STR(city)
		if (errno)
		{
			fprintf(stderr, "%s: %s: Could not parse\n", progname, getenv("SUN_HOME_CITY"));
			return 1;
		}
		errno = 0;
	}

	times = compute_times(lattitude, longitude, args.elevation_given ? args.elevation_arg : 0, date, sunangle);
	if (args.table_given) print_table();
	print_times(times);
	return 0;
}
