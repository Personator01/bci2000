/**********************************************************************************************************
*					Copyright (C) 1998 Excel Tech Ltd. All rights reserved.
*
* FILE:	XLTekStream.h
*
* HEADER:	$Header: /WacsDatabase/External/XLTekUtils/source/General/XLTekStream.h 6     12/23/98 9:57a Glen $
*
* DESCRIPTION:	Stream headers that use generic character type mappings
*
**********************************************************************************************************/
#if !defined(XLTEK_INCLUDED_XLTEK_STREAM_H)
#define XLTEK_INCLUDED_XLTEK_STREAM_H

#if defined(EXAMPLE_CODE)

// If you wish to include this file, copy and paste the following three lines:
#if !defined(XLTEK_INCLUDED_XLTEK_STREAM_H)
#include "XLTekUtils/Source/General/XLTekStream.h"
#endif // not XLTEK_INCLUDED_XLTEK_STREAM_H

#endif // EXAMPLE_CODE

#if !defined(XLTEK_SYSTEM_INCLUDED_TCHAR_H)
#define XLTEK_SYSTEM_INCLUDED_TCHAR_H
#include <tchar.h>
#endif // not XLTEK_SYSTEM_INCLUDED_TCHAR_H


#undef XLTEK_USE_STD_STREAM

/**********************************************************************************************************
* DEFINE:	XLTEK_USE_STD_STREAM
*
* DESCRIPTION:	Whether or not to use the streams taken from the
* std library.  The default is to use them, but we may want to use others - say for driver work.
*
**********************************************************************************************************/
#define XLTEK_USE_STD_STREAM

#if defined(XLTEK_FORCE_USE_STD_STREAM)
#define XLTEK_USE_STD_STREAM
#endif // XLTEK_FORCE_USE_STD_STREAM
#if defined(XLTEK_UNFORCE_USE_STD_STREAM)
#undef XLTEK_USE_STD_STREAM
#endif // XLTEK_UNFORCE_USE_STD_STREAM

#if defined(XLTEK_USE_STD_STREAM)

#if !defined(XLTEK_SYSTEM_INCLUDED_FSTREAM)
#define XLTEK_SYSTEM_INCLUDED_FSTREAM
#include <xstddef> // necessary due to explicit warning changes in yvals.h
#pragma warning( push )
#pragma warning( disable: 4018 4097 4100 4146 4244 4245 4511 4512 4663 4786)
#include <fstream>
#pragma warning( pop )
#endif // not XLTEK_SYSTEM_INCLUDED_FSTREAM
#if !defined(XLTEK_SYSTEM_INCLUDED_SSTREAM)
#define XLTEK_SYSTEM_INCLUDED_SSTREAM
#include <xstddef> // necessary due to explicit warning changes in yvals.h
#pragma warning( push )
#pragma warning( disable: 4018 4097 4100 4146 4244 4245 4511 4512 4663 4786)
#include <sstream>
#pragma warning( pop )
#endif // not XLTEK_SYSTEM_INCLUDED_SSTREAM

typedef std::basic_ostream< TCHAR > _tostream;
typedef std::basic_istream< TCHAR > _tistream;
typedef std::basic_ofstream< TCHAR > _tofstream;
typedef std::basic_ifstream< TCHAR > _tifstream;
typedef std::basic_ostringstream< TCHAR > _tostringstream;
typedef std::basic_istringstream< TCHAR > _tistringstream;

#else // not XLTEK_USE_STD_STREAM
#error XLTEK_USE_STD_STREAM default case!
#endif // not XLTEK_USE_STD_STREAM

/**********************************************************************************************************
* NAMESPACE:	NXLTekStream
*
* DESCRIPTION:	Protects XLTek specific global function from polluting the global
* namespace.
*
**********************************************************************************************************/
namespace NXLTekStream
{

void ResetStream(
	_tostringstream &i_Stream);

} // namespace NXLTekStream

#endif // not XLTEK_INCLUDED_XLTEK_STREAM_H
/**********************************************************************************************************
*
* End of file $Workfile: XLTekStream.h $
*
**********************************************************************************************************/
