#include "udf.h"

DEFINE_CG_MOTION(test, dt, vel, omega, time, dtime)
{
	vel[1]=-0.006986789;
}
