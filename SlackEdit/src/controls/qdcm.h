/*

Quick'n'Dirty ControlManager (QDCM): 
- Could I have some more GUI please?

*********************************
$ Workfile: qdcm.h				$
$ Author: Jon Lech Johansen		$
$ Date: 99-06-25 22:06			$
$ Revision: 1					$
*********************************

	Written by Jon Johansen
	e-mail:	digitech@mmadb.no
	web:	http://mmadb.no/jon
	Copyright © 1999 Jon Johansen. All Rights Reserved.

	Quick'n'Dirty ControlManager is based on very advanced c++ oriented 
	mfc code by Bogdan Ledwig <bogdanle@opole.mtl.pl> - http://bogdanle.cjb.net/. 
	
	The fact that I have been forced to write it from "scratch" and spent
	90 hours coding and testing it, makes me consider this code "my" code.
	This code is not heavy optimized, and I'm sure it could be, so feel
	free to send me suggestions, comments, improvements and flames (I'll read
	them and have a good laugh, and then they'll go directly to /dev/null).

	This code may be used in compiled form in any way you wish. This
	file may be redistributed unmodified by any means PROVIDING it is 
	not sold for profit without the author's written consent, and 
	providing that this notice and the author's name is included. If the 
	source code is used in any commercial product then a statement 
	along the lines of "Portions Copyright (C) 1999 Jon Johansen" 
	must be included in the startup banner, "About" box or printed 
	documentation. An email letting the author know that you are using
	it would be nice as well.

	This software is provided "as is" without express or implied warranty. 
	Use it at you own risk! The author accepts no liability for any damages 
	to your computer or data this product may cause.

History:
	1.0	1999-07-01 - FPR (That's First Public Release and not Fast Page Ram! ;)

*/

#ifndef __QDCM_H
#define __QDCM_H

//extern "C"
//{
	extern void __stdcall QDCM_Install(BOOL, BOOL, DWORD);
/*										|		|	  |
										|		|	  |-> Id of thread you wish to QDCM.
										|		|	  |-> To QDCM: (Who do you wish to 
										|		|	  |-> Quick'n'Dirty Control Manage today?)
										|		|	  |-> Use -1 if current thread
										|		|-> Only use QDCM on dialogs?
										|-> Do you want to QDCM right away? If FALSE,
										|-> you have to use QDCM_Enable() later. 
*/
	extern void __stdcall QDCM_Remove();
	extern void __stdcall QDCM_Enable();
//}

#endif /* __QDCM_H */
