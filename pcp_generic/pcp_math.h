/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_math.h
 * Created    : 07/05/00
 * Owner      : pcppopper
 * Revised on : 07/05/00
 * Comments   : Parts are from glib
 *              
 *              
 ****************************************************************/

#ifndef __PCP_MATH_H
#define __PCP_MATH_H

#include "pcp_generic.h"

#define Math_Clamp(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define Math_TransposeValues(x, y)	((x) ^= (y) ^= (x) ^= (y))

FOREXPORT int Math_ClosestPrime(int nNumber);

#endif /* __PCP_MATH_H */
