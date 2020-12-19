#include <stdlib.h>

#include "date.h"

#define DAYS_IN_MONTH 30
#define MONTHS_IN_YEAR 12

//Returns boolean value indicating if the date params are valid
static bool isDateValid(int day, int month, int year);

struct Date_t
{
	int day;
	int month;
	int year;
};

Date dateCreate(int day, int month, int year)
{
	if(!isDateValid(day, month, year))
	{
		return NULL;
	}
	Date date = (Date) malloc(sizeof(*date));
	if(!date)
	{
		return NULL;
	}
	date->day = day;
	date->month = month;
	date->year = year;
	return date;
}


static bool isDateValid(int day, int month, int year)
{
    if(day < 1 || day > DAYS_IN_MONTH)
    {
        return false;
    }
    if(month < 1 || month > MONTHS_IN_YEAR)
    {
        return false;
    }
    return true;

}

void dateDestroy(Date date)
{
    if(date)
    {
        free(date);
    }
}


Date dateCopy(Date date)
{
    if(!date)
    {
        return NULL;
    }
    return dateCreate(date->day, date->month, date->year);
}


bool dateGet(Date date, int* day, int* month, int* year)
{
	if(!date || !day || !month || !year)
	{
		return false;
	}
	*day = date->day;
	*month = date->month;
	*year = date->year;
	return true;
}

int dateCompare(Date date1, Date date2)
{
    if(!date1 || !date2)
    {
        return 0;
    }
	int year_delta = date1->year - date2->year;
	int month_delta = date1->month - date2->month;
	int day_delta = date1->day - date2->day;
	if(year_delta != 0)
	{
		return year_delta;
	}
	if(month_delta != 0)
	{
		return month_delta;
	}
    return day_delta;
}

void dateTick(Date date)
{
	if(!date)
	{
		return;
	}
	if(date->day < DAYS_IN_MONTH)
	{
		date->day++;
		return;
	}
	if(date->month < MONTHS_IN_YEAR)
	{
		date->month++;
		date->day = 1;
		return;
	}
	date->year++;
	date->month = 1;
	date->day = 1;
	return;
}