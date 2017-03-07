#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

//  parsedate.c, written by Chris.McDonald@uwa.edu.au


/*  This file provides the single function:

	char *parsedate(const char *s, struct tm *tp);

    It accepts a date as a string (s) and, if the string can be successfully
    parsed as a date, the broken down date is returned via tp.
    If the parsing is successful, NULL is returned, otherwise the
    return value provides a brief description of the first error found.

    Compile with:	cc -std=c99 -Wall -Werror -pedantic -c parsedate.c
 */

#if	!defined(USA)
#define	BRITISH
#endif

#if	defined(__linux__)
extern char		*strdup(const char *);
#endif

#define	MAXTOKENS	64
#define	AMBIGUOUS	(-1)
#define	INVALID		(-1)
#define	UNKNOWN		(-1)

typedef enum
	{INT=1, MONTH, DAY, AM, PM, ITH, MINUS, SLASH, COLON} DTOKEN;

static char *months[] = {
	"january", "february", "march", "april", "may", "june", "july",
	"august", "september", "october", "november", "december", 0 };
  
static char *dow[] = {
	"sunday", "monday", "tuesday", "wednesday", "thursday",
	"friday", "saturday", 0 };
    
static char *ith[] = {
	"st", "nd", "rd", "th" , 0};

static int mdays[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

struct {
    DTOKEN	token;
    int		value;
} tokens[MAXTOKENS];

static	int	ntokens	= 0;


// ------------------- date twiddling routines -------------------------

// days() -- how many days were in a year.
static int days_in_year(int y)
{
    if(y < 1970) y += 1900;
    if(((y % 4) == 0) && ( (y % 100) || ((y % 400)==0) )) y = 366;
    else y = 365;
     return y;
}

// jan1() -- return day of the week of jan 1 of given year
static int jan1(int yr)
{
    int y, d; // normal gregorian calendar one extra day per four years
  
    y = yr;
    d = 4+y+(y+3)/4;
  
    // julian calendar, regular gregorian less three days per 400
  
    if(y > 1800) {
	d -= (y-1701)/100;
	d += (y-1601)/400;
    }
    if(y > 1752) d += 3;	// take care of weirdness at 1752.
    return(d%7);
}

// doyr() -- insert partial day of year given yr and mon into tm struct
static void doyr(struct tm *tp)
{
    int i,j=0;

    mdays[1]	= (tp->tm_mon > 1 && days_in_year(tp->tm_year) == 366) ? 29:28;
    for(i=0 ; i < tp->tm_mon ; i++)
	j += mdays[i];
    tp->tm_yday = j + tp->tm_mday - 1;
}

// dowk() -- insert day of week given year and day of year into tm struct
static void dowk(struct tm *tp)
{
    tp->tm_wday = (jan1(tp->tm_year+1900) + tp->tm_yday) % 7;
}


// ------------------ string matching routines -------------------------

// prefix() -- return how many chars of s1 prefix s2
static int prefix(char *s1, char *s2)
{
    int i = 0 ;
  
    for(; *s1 == *s2 ; s1++,s2++,i++)
	if(*s2 == '\0') break;
    return(*s1 == '\0' ? i : 0);
}

// unambiguous() -- look up str in list for non-ambiguous (prefix) match
static int unambiguous(char *str, char **listp)
{
    int i,j,k;
    int ambig = 0, ret = (-2);
  
    for(i = 0,k = 0 ; *listp ; listp++,k++)
	if((j  = prefix(str,*listp)) > i) {
	    ambig = 0;
	    i = j;
	    ret = k + 1;
	}
	else if(j && (i == j)) ++ambig;
    return(ambig ? AMBIGUOUS : ret);
}

// exact() -- like unambiguous() but demands exact match
static int exact(char *str, char **listp)
{
    int i = 0;
  
    for(i=0 ; *listp ; listp++,i++)
	if(strcmp(str,*listp) == 0)  return i;
     return -1;
}


// ---------------------------------------------------------------------

static char *my_tokenize(char *sp)
{
    char	*wp, word[64];
    static char	errbuf[128];

    int		val;

    ntokens	= 0;
    while(*sp) {
	while(*sp == ' ' || *sp == '\t' || *sp == ',')
	    sp++ ;
	if(*sp == '\0')
	    break;
	wp	= word;
	if(isalpha(*sp)) {
	    while(isalpha(*sp) || *sp == '.')
		*wp++ = *sp++ ;
	    *wp	= '\0';

	    if(strcmp("am", word) == 0)
		tokens[ntokens++].token	= AM;
	    else if(strcmp("pm", word) == 0)
		tokens[ntokens++].token	= PM;

	    else if((val = unambiguous(word, months)) >= 0) {
		tokens[ntokens].token	= MONTH;
		tokens[ntokens].value	= val-1;
		++ntokens;
	    }
	    else if((val = exact(word, ith)) >= 0) {
		tokens[ntokens].token	= ITH;
		tokens[ntokens].value	= val;
		++ntokens;
	    }
	    else if((val = unambiguous(word, dow)) >= 0) {
		tokens[ntokens].token	= DAY;
		tokens[ntokens].value	= val-1;
		++ntokens;
	    }
	    else {
		sprintf(errbuf, "invalid word '%s'", word);
		 return errbuf;
	    }
	    continue;
	}
	if(isdigit(*sp)) {
	    val	 = 0;
	    while(isdigit(*sp)) {
		val = val*10 + (*sp - '0');
		++sp;
	    }
	    tokens[ntokens].token	= INT;
	    tokens[ntokens].value	= val;
	    ++ntokens;
	    continue;
	}
	if(*sp == '/' || *sp == ':' || *sp == '-') {
	    *wp	= *sp;
	    while(*sp == *wp)
		++sp;
	    if(*wp == '/')
		tokens[ntokens++].token	= SLASH;
	    else if(*wp == ':')
		tokens[ntokens++].token	= COLON;
	    else
		tokens[ntokens++].token	= MINUS;
	    continue;
	}
	else {
	    sprintf(errbuf, "invalid character '%c'", *sp);
	     return errbuf;
	}
    }
    return NULL;
}


static int parsecalendar(int *nt, struct tm *tp)
{
    int	n	= *nt;
    int	val1, val2;

/*  LOOK FOR:
		[WEEKDAY] 25-dec-yy
		[WEEKDAY] dec-25-yy
		[WEEKDAY] 25-12-yy	iff british
		[WEEKDAY] 12-25-yy	if !british
   */

    if(tokens[n].token == DAY) {
	tp->tm_wday	= tokens[n].value;
	++n ;
    }
    if(tokens[n].token == INT) {				// INT ...
	val1	= tokens[n].value;
	++n;
	if(val1 <= 31 && tokens[n].token == ITH)
	    ++n;
	if(tokens[n].token == MINUS || tokens[n].token == SLASH)
	    ++n;
	if(tokens[n].token == MONTH) {
	    val2	= tokens[n].value;
	    ++n;
	    tp->tm_mday = val1; tp->tm_mon = val2;		// 25-dec
	}
	else if(tokens[n].token == INT) {
	    val2 = tokens[n].value;
	    ++n;
#if	defined(BRITISH)
	    tp->tm_mday = val1, tp->tm_mon = val2-1;		// 25/12
#else
	    tp->tm_mday = val2, tp->tm_mon = val1-1;		// 12/25
#endif
	}
	else
	    return false;
    }
    else if(tokens[n].token == MONTH) {				// dec ... 
	val1	= tokens[n].value;
	++n;
	if(tokens[n].token == MINUS || tokens[n].token == SLASH)
	    ++n;
	if(tokens[n].token == INT) {
	    val2 = tokens[n].value;
	    ++n;
	    if(val2 <= 31 && tokens[n].token == ITH)
		++n;
	    tp->tm_mday = val2; tp->tm_mon = val1;		// dec-25
	}
	else
	    return false;
    }

    if(tokens[n].token == INT && (n+1)<ntokens && tokens[n+1].token == COLON) {
	*nt = n;						// CAL hh:
	return true;
    }

    if(tokens[n].token == MINUS || tokens[n].token == SLASH)
	++n;

    if(tokens[n].token != INT)
	return false;
    tp->tm_year = tokens[n].value;				// 25-dec-96
    ++n;

    *nt = n;
    return true;
}


static int parsetime(int *nt, struct tm *tp)
{
    int	n	= *nt;
    int	val;

//  LOOK FOR	[WEEKDAY] HH [:MM [:SS]] [AM|PM]
    if(tokens[n].token == DAY) {
	tp->tm_wday	= tokens[n].value;
	++n ;
    }
    if(tokens[n].token != INT)
	return false;
    val	= tokens[n].value;
    ++n;

    if(val >= 0 && val <= 23) {
	if(tokens[n].token != COLON &&
	   tokens[n].token != AM    && tokens[n].token != PM)
		return false;

	tp->tm_hour	= val;			// have HH
	tp->tm_min	= 0;
	tp->tm_sec	= 0;
	if(tokens[n].token == COLON && tokens[n+1].token == INT) {

	    ++n;
	    val = tokens[n].value;
	    if(val < 0 || val > 59)
		return false;
	    tp->tm_min = val;			// have HH:MM

	    ++n;
	    if(tokens[n].token == COLON && tokens[n+1].token == INT) {
		++n;
		val = tokens[n].value;
		if(val < 0 || val > 59)
		    return false;
		tp->tm_sec = val;		// have HH:MM:SS
		++n;
	    }
	    else
		tp->tm_sec = 0;
	}
	if(tokens[n].token == AM) {		// have XX AM
	    if(tp->tm_hour >= 12)
		return false;
	    if(tp->tm_hour == 12)		// 12am is midnght
		tp->tm_hour = 0;
	    ++n;
	}
	else if(tokens[n].token == PM) {	// have XX PM
	    if(tp->tm_hour == 0)
		return false;
	    if(tp->tm_hour < 12)		// 12pm is midday
		tp->tm_hour += 12;
	    ++n;
	}
	*nt = n;
	return true;				// have HH [:MM [:SS]][AM|PM]
    }
    return false;
}

static char *parsetokens(struct tm *tp)
{
    int	nt = 0;

//  NOW SEARCH FOR THE TIME, POSSIBLY FOLLOWED BY THE DATE
    nt	= 0;
    if(parsetime(&nt, tp) == true) {
	if(nt < ntokens && parsecalendar(&nt, tp) == false)
	     return "cannot parse date";
    }

//  OR THE DATE, POSSIBLY FOLLOWED BY THE TIME
    else {
	nt = 0;
        if(parsecalendar(&nt, tp) == true) {
	    if(nt < ntokens && parsetime(&nt, tp) == false)
		 return "cannot parse time";
	}
    }

    if(nt < ntokens && tokens[nt].token == INT && tp->tm_year == UNKNOWN) {
	tp->tm_year = tokens[nt].value;
	++nt;
    }
    return NULL;
}


static char *validate(struct tm *tp)
{
    int		NOW, given_wday;
    struct tm	*tm_now;

    NOW		= (int)time((time_t *)0L);
    tm_now	= localtime((time_t *)&NOW);

//  MAKE MANY DEFAULTS EQUALS TO TODAY'S DETAILS
    if(tp->tm_year == UNKNOWN) {
	tp->tm_year	= tm_now->tm_year;
	if(tp->tm_mon == UNKNOWN && tp->tm_mday == UNKNOWN) {
	    tp->tm_mon	= tm_now->tm_mon;
	    tp->tm_mday	= tm_now->tm_mday;
	}
    }
    if(tp->tm_hour == UNKNOWN) {
	tp->tm_hour	= 0;
	tp->tm_min	= 0;
    }
    if(tp->tm_sec == UNKNOWN)
	tp->tm_sec = 0;

    if(tp->tm_mon > 11) {
	tp->tm_mon = UNKNOWN;
	 return "invalid month";
    }
    if(tp->tm_hour > 23) {
	tp->tm_hour = UNKNOWN;
	 return "invalid month";
    }
    if(tp->tm_min > 59) {
	tp->tm_min = UNKNOWN;
	 return "invalid minute";
    }
    if(tp->tm_sec > 59) {
	tp->tm_sec = UNKNOWN;
	 return "invalid second";
    }

    if(tp->tm_mday < 1)
	goto badday;

    if(tp->tm_mday > mdays[tp->tm_mon]) {
	if(tp->tm_mon == 1 && tp->tm_mday == 29 &&
		days_in_year(tp->tm_year) == 365)	// check for Feb 29
		goto badday;
	}

//  FILL IN DAY OF YEAR, DAY OF WEEK (THESE CALLS MUST BE
//  IN THIS ORDER AS dowk() NEEDS dOYR() )

    if(tp->tm_year >= 1900)
	tp->tm_year -= 1900;
    doyr(tp);
    given_wday	= tp->tm_wday;
    dowk(tp);
    if(given_wday != UNKNOWN && given_wday != tp->tm_wday) {
	tp->tm_wday = UNKNOWN;
	 return "invalid weekday";
    }

//  CALL mktime() JUST TO DETERMINE DAYLIGHT SAVINGS TIME
    if(mktime(tp) != (-1))
	return NULL;
    else
	return "invalid date";

badday : tp->tm_mday = UNKNOWN;
     return "invalid day of month";
}

// ------------------------------------------------------------------------

char *parsedate(const char *s, struct tm *tp)
{
    char	*sp, *orig;

    ntokens	= 0;
    memset(tokens, 0, sizeof(tokens));
    memset(tp, UNKNOWN, sizeof(struct tm));
    sp = orig	= strdup(s);

    while(*sp) {
	if(isupper(*sp))
	    *sp = tolower(*sp);
	++sp;
    }

    if((sp = my_tokenize(orig))  == NULL	&&
       (sp = parsetokens(tp))    == NULL	&&
       (sp = validate(tp))       == NULL	)
		;
    free(orig);
    return sp;
}

//  vim:set sw=4 ts=8

