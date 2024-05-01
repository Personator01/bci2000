/**********************************************************************************************************
*					Copyright (C) 2000 Excel Tech Ltd. All rights reserved.
*
* FILE:	binvariant.h
*
* HEADER:	$Header: /XLTekUtils/source/Com/binvariant.h 8     1/11/01 7:25p Itulchinsky $
*
* Created: 8/10/00 11:04:26 AM by Ilia Tulchinsky
* Last Modified: 8/10/00 11:04:26 AM
* DESCRIPTION: Definitions for converting arbitrary data structres to and from VARIANT	
* Read XLTEKUtils\Documentation\BinVariant.doc for more details.	
**********************************************************************************************************/
#if !defined(XLTEK_INCLUDED_BINVARIANT_UTIL_H)
#define XLTEK_INCLUDED_BINVARIANT_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**********************************************************************************************************
* CLASS:	
*
* DESCRIPTION: A class that implements conversion of an array of class T objects into VARIANT and from VARIANT	
*
**********************************************************************************************************/

template<class T>
class BinaryDataInVariant
{
public:
	BinaryDataInVariant(const T *aData,int count);
	BinaryDataInVariant();
	BinaryDataInVariant(VARIANT *aVar);
	BinaryDataInVariant(const VARIANT &aVar);
	virtual ~BinaryDataInVariant();
public:
	void setData(const T *aData,int count);
	operator const VARIANT () const;
	const VARIANT* operator&() const;
	VARIANT* operator&();
	int getData(T *aData,int count) const;
	int getCount() const;
	VARIANT copyToVARIANT() const;
private:	
	void destroy();
private:

	VARIANT var;
};


/**********************************************************************************************************
* CLASS:	StructInVariant;
*
* DESCRIPTION:	Implements a conversion of a user defined structure to and from variant.
* Is able to construct from the reference or a pointer to the structure and also from VARIANT
* and conversly is has operators for converting to the structure type and to VARIANT
* Is templated on the type of the structure.
**********************************************************************************************************/
template<class T>
class StructInVariant : public BinaryDataInVariant<T>
{
public:
	StructInVariant(const T& aData);
	StructInVariant(const T* aData);
	StructInVariant();
	StructInVariant(const VARIANT &aVar);
	void setData(const T& aData);
	void getData(T& aData) const;
	operator const T() const;
private:
	T m_converted;
	bool m_isDirty;
};


// Implementation

//BinaryDataInVariant
template<class T>
inline
//BinaryDataInVariant<T>::BinaryDataInVariant<T>(const T *aData,int count)
BinaryDataInVariant<T>::BinaryDataInVariant(const T *aData,int count)
{
	var.vt=VT_EMPTY;
	setData(aData,count);
}

template<class T>
inline
BinaryDataInVariant<T>::BinaryDataInVariant()		
{
	var.vt=VT_EMPTY;
}

template<class T>
inline
BinaryDataInVariant<T>::BinaryDataInVariant(VARIANT *aVar)
{
	var.vt=VT_EMPTY;
	VariantCopy(&var,aVar);
}

template<class T>
inline
BinaryDataInVariant<T>::BinaryDataInVariant(const VARIANT &aVar)
{
	var.vt=VT_EMPTY;
	VariantCopy(&var,const_cast<VARIANT *>(&aVar));
}

template<class T>
inline
BinaryDataInVariant<T>::~BinaryDataInVariant()
{
	destroy();
}

template<class T>
inline
void BinaryDataInVariant<T>::setData(const T *aData,int count)
{
	destroy();
	SAFEARRAYBOUND bound[1];
	bound[0].lLbound = 0;
	DWORD size = count*sizeof(T);  // Unicode OK
	bound[0].cElements = size;
	var.parray = SafeArrayCreate(VT_UI1,1,bound);
	void *ppvData;
	SafeArrayAccessData(var.parray,&ppvData);
	memcpy(ppvData,(void *)(aData),size);  // Unicode OK
	SafeArrayUnaccessData(var.parray);
	var.vt = VT_ARRAY | VT_UI1;
}

template<class T>
inline
BinaryDataInVariant<T>::operator const VARIANT() const
{
	/*
	VARIANT res;
	VariantInit(&res);
	VariantCopy(&res,const_cast<VARIANT *>(&var));
	*/
	return var;
}

template<class T>
inline
const VARIANT* BinaryDataInVariant<T>::operator&() const
{
	return &var;
}

template<class T>
inline
VARIANT* BinaryDataInVariant<T>::operator&()
{
	return &var;
}

template<class T>
inline
int BinaryDataInVariant<T>::getData(T *aData,int count) const
{
	if (var.vt !=VT_EMPTY) {	
		if (count>getCount())
			count = getCount();
		void *ppvData;
		SafeArrayAccessData(var.parray,&ppvData);
		memcpy(reinterpret_cast<void *>(aData),ppvData,count*sizeof(T));  // Unicode OK
		SafeArrayUnaccessData(var.parray);
		return count;
	}
	return 0;
}

template<class T>
inline
int BinaryDataInVariant<T>::getCount() const
{
	if (var.vt !=VT_EMPTY) {
		long res;
		SafeArrayGetUBound(var.parray,1,&res);
		return (res+1)/sizeof(T);  // Unicode OK
	} 
	return 0;
}

template<class T>
inline
void BinaryDataInVariant<T>::destroy()
{
	if (var.vt!=VT_EMPTY) {
		SafeArrayDestroy(var.parray);
		var.vt = VT_EMPTY;
	}
}

template<class T>
inline
VARIANT BinaryDataInVariant<T>::copyToVARIANT() const
{
	VARIANT res;
	VariantInit(&res);
	XLTEK_VERIFY(VariantCopy(&res,const_cast<VARIANT *>(&var))==S_OK);
	return res;
}

//StructInVariant

template<class T>
inline
StructInVariant<T>::StructInVariant(const T& aData)
 : BinaryDataInVariant<T>(&aData,1),m_isDirty(true)
{
}

template<class T>
inline
StructInVariant<T>::StructInVariant(const T* aData)
 : BinaryDataInVariant<T>(aData,1),m_isDirty(true) 
{
}

template<class T>
inline
StructInVariant<T>::StructInVariant()
: m_isDirty(true) 
{
}

template<class T>
inline
StructInVariant<T>::StructInVariant(const VARIANT &aVar)
 : BinaryDataInVariant<T>(aVar),m_isDirty(true) 
{
}

template<class T>
inline
void StructInVariant<T>::setData(const T& aData) 
{
	BinaryDataInVariant<T>::setData(&aData,1);
	m_isDirty=true;
}

template<class T>
inline
void StructInVariant<T>::getData(T& aData) const
{
	BinaryDataInVariant<T>::getData(&aData,1); 
}

template<class T>
inline
StructInVariant<T>::operator const T () const
{
	if (m_isDirty) {
		getData(const_cast<StructInVariant<T> *>(this)->m_converted); // this is not really violating the constness
		const_cast<StructInVariant<T> *>(this)->m_isDirty = false;
	}
	return m_converted;
}	



#endif // XLTEK_INCLUDED_BINVARIANT_UTIL_H
/**********************************************************************************************************
*
* End of file $Workfile: binvariant.h $
*
**********************************************************************************************************/
