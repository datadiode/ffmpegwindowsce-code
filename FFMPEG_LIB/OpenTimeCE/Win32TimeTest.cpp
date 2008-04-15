/////////////////////////////////////////////////////////////////////
//
// Legalese:
// This file is part of the OpenNETCF time.h port for Windows CE.
// You are free to use, modify or distribute this code or any
// derivative work that you create.  This code is provided WITHOUT
// WARRANTY and OpenNETCF does NOT claim in any way that this code 
// is fit for any specific or general use and holds NO RESPONSIBILITY
// for consequences of using any of it.  It is simply provided as-is.
//
// About:
// This is part of a free and open project to provide a simply way to
// port that uses time.h functions to Windows CE.  For the latest
// code, or to submit fixes, feature additions, etc. visit:
//
// http://www.opennetcf.com
//
// Version 0.01 - March 22, 2007
//
/////////////////////////////////////////////////////////////////////
//
// WinTime.cpp : Defines the entry point for the console application.
//
// 
// Sample output of the test program:
//-----------------------------------------------------------------------------
// Tue Mar 06 21:30:34 2007
//
// Feels like week 09 of 2007
// Today is Tuesday, 03/06/07
// It is 30 minutes past the hour 09.
//
// Program execution time = 0.000000 seconds.
// Program execution time = 0 ms since system start.
//
/////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <commctrl.h>
#include "time_ce.h"

// The test program will exercise all the methods in the time_ce library.
// printf statements are leftovers from testing on Windows XP.  You won't be
// be able to see any output to a console on a Pocket PC but they were left in 
// for those using CE devices with a Console or those wanting to use the library
// on the desktop.
//
// As part of the verification that the methods produced the correct results
// this code was run on a Windows XP system and the corresponding time.h 
// methods were added along side the time_ce methods.  This gave a pretty good
// feeling that the code was/is doing what is supposed to do.
//
int _tmain(int argc, _TCHAR* argv[])
{
	time_t_ce start_ce, start_ce2, finish_ce, now_ce;
	struct tm *tmptr_ce;
	struct tm *gmptr_ce;
	char * c_ce;
	char tBuff_ce[80];
	double duration_ce;
	SYSTEMTIME gmtTime;
    int i = 0;

	DWORD secs = 0;

	// Get the start time
	//
	GetSystemTime(&gmtTime);
	start_ce = time_ce(0);
	printf("start_ce: %x\n", start_ce);

	// Get the current time using another way of calling time
	//
	start_ce2 = time_ce(&now_ce);
	printf("start_ce2: %x\n", start_ce2);

	// Convert the "now" time_t value into a time "tm" structure
	//
	tmptr_ce = localtime_ce(&now_ce);

	finish_ce = mktime_ce(tmptr_ce);
	printf("finish_ce: %x\n", finish_ce);

	gmptr_ce = gmtime_ce(&now_ce);

	// Show the current time in ascii format
	//
	c_ce = asctime_ce(gmptr_ce);
	printf("gmptr_ce: %s\n", c_ce);

	tmptr_ce = localtime_ce(&now_ce);

	// Show the current time in ascii format
	//
	c_ce = asctime_ce(tmptr_ce);
	printf("tmptr_ce: %s\n", c_ce);
 
	// Show the different format types available using the strftime function
	strftime_ce(tBuff_ce, 80, "Feels like week %U of %Y", tmptr_ce);
	printf("ce: %s\n", tBuff_ce);

	strftime_ce(tBuff_ce, 80, "Today is %A, %x, %Z", tmptr_ce);
    printf("ce: %s\n", tBuff_ce);

	strftime_ce(tBuff_ce, 80, "It is %M minutes past the hour %I.", tmptr_ce);
    printf("ce: %s\n", tBuff_ce);

	strftime_ce(tBuff_ce, 80, "Test 1ce: %A, %a, %B, %b", tmptr_ce);
	printf("%s\n", tBuff_ce);

	strftime_ce(tBuff_ce, 80, "Test 2ce: %c, %d, %H, %I", tmptr_ce);
	printf("%s\n", tBuff_ce);

	strftime_ce(tBuff_ce, 80, "Test 3ce: %j, %m, %M, %p", tmptr_ce);
	printf("%s\n", tBuff_ce);

	strftime_ce(tBuff_ce, 80, "Test 4ce: %S, %U, %W, %w", tmptr_ce);
	printf("%s\n", tBuff_ce);

	strftime_ce(tBuff_ce, 80, "Test 5ce: %x%y, %Xx, %yY, %Yw", tmptr_ce);
	printf("%s\n", tBuff_ce);

	strftime_ce(tBuff_ce, 80, "Test 6ce: %Z", tmptr_ce);
	printf("%s\n", tBuff_ce);

    // Lets test the time difference function
    finish_ce = time_ce(0);
	duration_ce = difftime_ce(finish_ce, start_ce);
	printf("ce: Program execution time = %f seconds.\n", duration_ce);

	// Show the clock duration
	printf("ce: Program execution time = %ld ms since system start.\n", clock_ce());

	return 0;
}

