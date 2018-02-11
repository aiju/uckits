#include <u.h>
#include <libc.h>

double
round(double x)
{
	double f, i;

	if(isNaN(x) || isInf(x, 1) || isInf(x, -1))
		return x;
	f = modf(x, &i);	
	if(f <= -0.5)
		i--;
	else if(f >= 0.5)
		i++;
	return i;
}
