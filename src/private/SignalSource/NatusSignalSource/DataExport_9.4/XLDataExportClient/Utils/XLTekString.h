/**********************************************************************************************************
*					Copyright (C) 1998 Excel Tech Ltd. All rights reserved.
*
* FILE:	XLTekString.h
*
* HEADER:	$Header: /XLTekUtils/source/General/XLTekString.h 8     4/06/99 8:53a Glen $
*
* DESCRIPTION:	XLTek string header file.  Allows us to use _tstrings which isolate us from ANSI/UNICODE
* issues.  Also contains prototypes for useful string functions.
*
**********************************************************************************************************/
#if !defined(XLTEK_INCLUDED_XLTEK_STRING_H)
#define XLTEK_INCLUDED_XLTEK_STRING_H

#if defined(EXAMPLE_CODE)

// If you wish to include this file, copy and paste the following three lines:
#if !defined(XLTEK_INCLUDED_XLTEK_STRING_H)
#include "XLTekUtils/Source/General/XLTekString.h"
#endif // not XLTEK_INCLUDED_XLTEK_STRING_H

#endif // EXAMPLE_CODE

#if !defined(XLTEK_SYSTEM_INCLUDED_TCHAR_H)
#define XLTEK_SYSTEM_INCLUDED_TCHAR_H
#include <tchar.h>
#endif // not XLTEK_SYSTEM_INCLUDED_TCHAR_H

#undef XLTEK_USE_STD_STRING
/**********************************************************************************************************
* DEFINE:	XLTEK_USE_STD_STRING
*
* DESCRIPTION:	Whether or not to use the string taken from the std
* namespace.  The default is to use it, however we may wish to create
* our own (say for driver work).
*
**********************************************************************************************************/
#define XLTEK_USE_STD_STRING

#if defined(XLTEK_FORCE_USE_STD_STRING)
#define XLTEK_USE_STD_STRING
#endif // XLTEK_FORCE_USE_STD_STRING
#if defined(XLTEK_UNFORCE_USE_STD_STRING)
#undef XLTEK_USE_STD_STRING
#endif // XLTEK_UNFORCE_USE_STD_STRING

#if defined(XLTEK_USE_STD_STRING)

#if !defined(XLTEK_SYSTEM_INCLUDED_STRING)
#define XLTEK_SYSTEM_INCLUDED_STRING
#include <xstddef> // necessary due to explicit warning changes in yvals.h
#pragma warning( push )
#pragma warning( disable: 4018 4097 4100 4146 4244 4245 4511 4512 4663 4786)
#include <string>
#pragma warning( pop )
#endif // not XLTEK_SYSTEM_INCLUDED_STRING

typedef std::basic_string< TCHAR > _tstring;

#else // not XLTEK_USE_STD_STRING
#error XLTEK_USE_STD_STRING default case!
#endif // not XLTEK_USE_STD_STRING

// Bring in the character conversion macros
#if defined(_MFC_VER)

#if !defined(XLTEK_SYSTEM_INCLUDED_AFX_PRIV_H)
#define XLTEK_SYSTEM_INCLUDED_AFX_PRIV_H
#include <afxpriv.h>
#endif // not XLTEK_SYSTEM_INCLUDED_AFX_PRIV_H

#else // not _MFC_VER

// Must be ATL
#if !defined(XLTEK_SYSTEM_INCLUDED_ATL_BASE_H)
#define XLTEK_SYSTEM_INCLUDED_ATL_BASE_H
#include <atlbase.h>
#endif // not XLTEK_SYSTEM_INCLUDED_ATL_CONV_H
#if !defined(XLTEK_SYSTEM_INCLUDED_ATL_CONV_H)
#define XLTEK_SYSTEM_INCLUDED_ATL_CONV_H
#include <atlconv.h>
#endif // not XLTEK_SYSTEM_INCLUDED_ATL_CONV_H

#endif // not _MFC_VER

#endif // not XLTEK_INCLUDED_XLTEK_STRING_H
/**********************************************************************************************************
*
* End of file $Workfile: XLTekString.h $
*
**********************************************************************************************************/
