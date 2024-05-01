/**********************************************************************************************************
*					Copyright (C) 1998 Excel Tech Ltd. All rights reserved.
*
* FILE:	GuidUtil.cpp
*
* HEADER:	$Header: GuidUtil.cpp 14    11/06/01 8:40a Atopa $
*
* DESCRIPTION:	Definition of functions used for debugging
*
**********************************************************************************************************/
#include "..\StdAfx.h"
#include "GuidUtil.h"
#include "BinVariant.h"

#pragma warning(push)
#pragma warning(disable: 4097) // typedef-name 'GUID' used as synonym for class-name '_GUID'

NXLTekCom::CGuid::CGuid()
	:
	GUID( GUID_NULL )
{
}

NXLTekCom::CGuid::CGuid(
	const GUID &i_rGuid)
	:
	GUID( i_rGuid )
{
}

NXLTekCom::CGuid::CGuid(
	const CGuid &i_rGuid)
	:
	GUID( i_rGuid )
{
}

#pragma warning(pop)

NXLTekCom::CGuid::CGuid(
	const char *i_pString)  // Unicode OK - pairs with OLECHAR version
{
	SetFromString( i_pString );
}

NXLTekCom::CGuid::CGuid(
	const OLECHAR *i_pString)
{
	SetFromString( i_pString );
}

NXLTekCom::CGuid::CGuid(
	const _tstring &i_rString)
{
	SetFromString( i_rString.c_str() );
}

NXLTekCom::CGuid::~CGuid()
{
}

void NXLTekCom::CGuid::SetFromString(
	const char *i_pString)  // Unicode OK - pairs with OLECHAR version
{
	USES_CONVERSION;  // Unicode OK
	SetFromString( A2COLE( i_pString ) );
}

void NXLTekCom::CGuid::SetFromString(
	const OLECHAR *i_pString)
{
	CComBSTR guid_string;

	// fix an error that if the string does not have curly braces {} the API call will fail
	if ((i_pString[0] == _T('{')) && (i_pString[wcslen(i_pString) - 1] == _T('}')))
	{
		guid_string.Append(i_pString);
	}
	else 
	{
		if (i_pString[0] != _T('{'))
		{
			guid_string = L"{";
			guid_string.Append(i_pString);
			if (i_pString[wcslen(i_pString) - 1] != _T('}'))
			{
				guid_string.Append(L"}");
			}
		}
		else if (i_pString[wcslen(i_pString) - 1] != _T('}'))
		{
			guid_string.Append(i_pString);
			guid_string.Append(L"}");
		}
	}	
	if ( FAILED( CLSIDFromString( guid_string , this ) ) )
	{
		*this = GUID_NULL;
	}

}

_tstring NXLTekCom::CGuid::GetAsString() const
{
	const int BufferSize = GUID_STRING_LEN;
	OLECHAR StringBuffer[ BufferSize ];

	// StringFromGuid2 will return the number of bytes in the string - we know it should be GUID_STRING_LEN(39)
//	XLTEK_VERIFY( BufferSize == StringFromGUID2( *this, StringBuffer, BufferSize ) );
	// String buffer will now be NULL terminated
	USES_CONVERSION;  // Unicode OK
	return _tstring( OLE2T( StringBuffer ) );
}

void NXLTekCom::CGuid::CreateNew()
{
	::CoCreateGuid( this );
//	ASSERT( *this != GUID_NULL );
}

bool NXLTekCom::operator<(
	const NXLTekCom::CGuid& i_rGuid1, 
	const NXLTekCom::CGuid& i_rGuid2)
{
    return memcmp( &i_rGuid1, &i_rGuid2, sizeof(NXLTekCom::CGuid) ) < 0;  // Unicode OK
}

_tistream &NXLTekCom::operator>>(
	_tistream &i_rInputStream, NXLTekCom::CGuid &i_rGuid)
{
	USES_CONVERSION;  // Unicode OK
	_tstring GuidString;
	i_rInputStream >> GuidString;
	NXLTekCom::CGuid ActualGuid;
	if ( FAILED( CLSIDFromString( const_cast<OLECHAR *>(T2COLE( GuidString.c_str() ) ), &ActualGuid ) ) )
	{
		// This may throw - the value of the input guid is undefined
		i_rInputStream.setstate( std::ios_base::failbit );
	}
	if ( i_rInputStream )
	{
		i_rGuid = ActualGuid;
	}
	return i_rInputStream;
}

_tostream &NXLTekCom::operator<<(
	_tostream &i_rOutputStream, const NXLTekCom::CGuid &i_rGuid)
{
	i_rOutputStream << i_rGuid.GetAsString();

	return i_rOutputStream;
}

// Variant support

NXLTekCom::CGuid::CGuid(const VARIANT &var)
{
	*this = static_cast< StructInVariant<CGuid> > (var);
}

VARIANT NXLTekCom::CGuid::GetAsVariant() const
{
	CComVariant tmp(static_cast<  StructInVariant<CGuid> > (*this));
	VARIANT res;
	VariantInit(&res);
	tmp.Detach(&res);
	return res;
		//ConvertToVariant(*this);
}

// BSTR support

NXLTekCom::CGuid::CGuid(const _bstr_t &bstr)
{
	SetFromString(static_cast<char *> (bstr));  // Unicode OK
}

/**********************************************************************************************************
*
* End of file $Workfile: GuidUtil.cpp $
*
**********************************************************************************************************/
