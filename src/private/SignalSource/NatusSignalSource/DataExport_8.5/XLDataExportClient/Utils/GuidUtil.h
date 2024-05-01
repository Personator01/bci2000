/**********************************************************************************************************
*					Copyright (C) 1998 Excel Tech Ltd. All rights reserved.
*
* FILE:	GuidUtil.h
*
* HEADER:	$Header: /XLTekUtils/source/Com/GuidUtil.h 17    12/20/01 6:17p Jpunnett $
*
* DESCRIPTION:	Utility functions for working with guids
*
**********************************************************************************************************/
#if !defined(XLTEK_INCLUDED_GUID_UTIL_H)
#define XLTEK_INCLUDED_GUID_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlbase.h>
#include <comdef.h>
#include <string>
#include <functional>
#include <iostream>

#include "XLTekStream.h"
#include "XLTekString.h"

namespace NXLTekCom
{
	enum {GUID_STRING_LEN = 39};
	class CGuid
		:
		public GUID
	{
	public:
		CGuid();
		explicit CGuid(const VARIANT &var);
		CGuid(
			const GUID &i_rGuid);
		CGuid(
			const CGuid &i_rGuid);
		CGuid(
			const char *i_pString);  // Unicode OK - pairs with OLECHAR version
		CGuid(
			const OLECHAR *i_pString);
		CGuid(
			const _tstring &i_rString);
		CGuid(
			const _bstr_t &bstr);
		~CGuid();
		enum { GuidLengthInCharacters = 38 };
		void SetFromString(
			const char *i_pString);  // Unicode OK - pairs with OLECHAR version
		void SetFromString(
			const OLECHAR *i_pString);
		_tstring GetAsString() const;
		VARIANT GetAsVariant() const;
		void CreateNew();
		struct SQL_Less : public std::binary_function<CGuid, CGuid, bool> {
		bool operator()(CGuid x, CGuid y);
		};
	};

	bool operator<(
		const CGuid& i_rGuid1, 
		const CGuid& i_rGuid2);

	_tistream &operator>>(
		_tistream &i_rInputStream, CGuid &i_rGuid);

	_tostream &operator<<(
		_tostream &i_rOutputStream, const CGuid &i_rGuid);

	class FakeGuid : public CGuid
	{
		public:
			FakeGuid(short data2,short data3,long data1=0)
			{
				Data1=data1;
				Data2=data2;
				Data3=data3;
			}
	};

} // namespace NXLTekCom

inline
GUID createNewGUID()
{
	NXLTekCom::CGuid guid;
	guid.CreateNew();  
	return guid;
}

inline
_bstr_t GUID2BSTR(GUID g)
{
	return NXLTekCom::CGuid(g).GetAsString().c_str();
}

inline 
GUID BSTR2GUID(BSTR guidSTR)
{
	return NXLTekCom::CGuid(guidSTR);
}

inline
_tstring GUID2STR(GUID g)
{
	return NXLTekCom::CGuid(g).GetAsString();
}

inline 
GUID STR2GUID(const TCHAR* guidSTR)
{
	return NXLTekCom::CGuid(guidSTR);
}


#endif // not XLTEK_INCLUDED_GUID_UTIL_H
/**********************************************************************************************************
*
* End of file $Workfile: GuidUtil.h $
*
**********************************************************************************************************/
