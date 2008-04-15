#include <inttypes.h>

float rintf(float x)
{
#ifdef UNDER_CE
	//MS999 - FIXME - not correct 
	return (int)x;
#else
    __asm 
	{
        fld x
        frndint
    }
#endif
}

double rint(double x)
{
#ifdef UNDER_CE
	//MS999 - FIXME - not correct 
	return (int)x;
#else
    __asm 
	{
        fld x
        frndint
	}
#endif

}

/*ms999 - orig
long int lrintf(float flt)
{
	int intgr;

	_asm
	{       
		fld flt
		fistp intgr
	}

	return intgr;
*/
long int lrintf(double flt){

#ifdef UNDER_CE
	//MS999 - FIXME - not correct 
	return (long int)flt;
#else
	int intgr;
	_asm
	{       
		fld flt
		fistp intgr
	}
    return intgr;
#endif

}

double getNan()
{
	union
	{
		double d;
		int64_t x;
	} v;

	v.x = 0x7FF80000;	// gcc returns this for 0.0 / 0.0

	return v.d;
}
