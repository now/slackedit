/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_spin.h
 * Created    : not known (before 12/03/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:29:23
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_SPIN_H
#define __PCP_SPIN_H

#include "pcp_generic.h"

#define Spin_GetAccel(hwndCtl, cAccel, paAccels)	((DWORD)SNDMSG((hwndCtl), UDM_GETACCEL, (WPARAM)(cAccels), (LPARAM)(LPUDACCEL)(paAccel)))
#define Spin_GetBase(hwndCtl)						((DWORD)SNDMSG((hwndCtl), UDM_GETBASE, 0, 0L))
#define Spin_GetBuddy(hwndCtl)						((DWORD)SNDMSG((hwndCtl), UDM_GETBUDDY, 0, 0L))
#define Spin_GetPos(hwndCtl)						((DWORD)SNDMSG((hwndCtl), UDM_GETPOS, 0, 0L))
#define Spin_GetRange(hwndCtl)						((DWORD)SNDMSG((hwndCtl), UDM_GETRANGE, 0, 0L))
#define Spin_GetRange32(hwndCtl)					((DWORD)SNDMSG((hwndCtl), UDM_GETRANGE32, 0, 0L))
#define Spin_GetUnicodeFormat(hwndCtl)				((DWORD)SNDMSG((hwndCtl), UDM_GETUNICODEFORMAT, 0, 0L))
#define Spin_SetAccel(hwndCtl, nAccels, aAccels)	((DWORD)SNDMSG((hwndCtl), UDM_SETACCEL, (WPARAM)(nAccels), (LPARAM)(LPUDACCEL)(aAccels)))
#define Spin_SetBase(hwndCtl, nBase)				((DWORD)SNDMSG((hwndCtl), UDM_SETBASE, (WPARAM)(nBase), OL))
#define Spin_SetBuddy(hwndCtl, hwndBuddy)			((DWORD)SNDMSG((hwndCtl), UDM_SETBUDDY, (WPARAM)(HWND)(hwndBuddy), OL))
#define Spin_SetPos(hwndCtl, nPos)					((DWORD)SNDMSG((hwndCtl), UDM_SETPOS, 0, (LPARAM)MAKELONG((short)(nPos), 0))
#define Spin_SetRange(hwndCtl, nUpper, nLower)		((DWORD)SNDMSG((hwndCtl), UDM_SETRANGE, 0, (LPARAM)MAKELONG((short)(nUpper), (short)(nLower))))
#define Spin_SetRange32(hwndCtl, iLow, iHigh)		((DWORD)SNDMSG((hwndCtl), UDM_SETRANGE32, (WPARAM)(int)(iLow), (LPARAM)(int)(iHigh)))
#define Spin_SetUnicodeFormat(hwndCtl, fUnicode)	((DWORD)SNDMSG((hwndCtl), UDM_SETUNICODEFORMAT, (WPARAM)(BOOL)(fUnicode), OL))

#endif /* __PCP_SPIN_H */
