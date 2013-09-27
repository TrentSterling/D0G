//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//===========================================================================//

#ifndef ARRAYSTACK_H
#define ARRAYSTACK_H
#pragma once

#include <assert.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif
#include "List.h"

template <class T> class ArrayStack
{
protected:
	T *data;
	int m_stackDepth;
	int m_maxNumElements;
	
public:
	ArrayStack( int maxNumElements )
	{
		data = new T[maxNumElements];
		m_maxNumElements = maxNumElements;
		m_stackDepth = 0;
		assert( data );
	}
	
	void Push( T elem )
	{
		data[m_stackDepth++] = elem;
		if( m_stackDepth > m_maxNumElements )
		{
#ifdef __ANDROID__
			__android_log_print(ANDROID_LOG_WARN, "ArrayStack", "ArrayStack overflow");
#else
			printf( "ArrayStack overflow\n" );
#endif
			assert( 0 );
		}
	}
	
	T Pop( void )
	{
		if( m_stackDepth == 0 )
		{
#ifdef __ANDROID__
			__android_log_print(ANDROID_LOG_WARN, "ArrayStack", "ArrayStack underflow");
#else
			printf( "ArrayStack underflow\n" );
#endif
			assert( 0 );
		}
		return data[--m_stackDepth];
	}

	bool IsEmpty()
	{
		return ( m_stackDepth == 0 );
	}
	
	int GetDepth()
	{
		return m_stackDepth;
	}
};


#endif // ARRAYSTACK_H
