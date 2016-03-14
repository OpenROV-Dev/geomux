// Includes
#include "CVideoBuffer.h"
#include <iostream>

// Namespace
using namespace std;

CVideoBuffer::CVideoBuffer( size_t defaultReservedBytesIn )
	: k_containerSize( defaultReservedBytesIn )
	, m_data( new uint8_t[ k_containerSize ] )
	, m_endIndex( 0 )
	, m_remainingCapacity( k_containerSize )
{
	// Init synch variables
	uv_mutex_init( &m_mutex );
	uv_cond_init( &m_dataAvailableCondition );
}

uint8_t* CVideoBuffer::Begin()
{
	// Return the pointer to the beginning of the buffer 
	return m_data.get();
}

uint8_t* CVideoBuffer::End()
{
	// Return the pointer to the past-the-end element of the buffer. This is where the theoretical next element would go.
	return m_data.get() + m_endIndex;
}

size_t CVideoBuffer::GetSize()
{
	return m_endIndex;
}

size_t CVideoBuffer::GetRemainingCapacity()
{
	return m_remainingCapacity;
}

size_t CVideoBuffer::GetReservedSize()
{
	return k_containerSize;
}
	
void CVideoBuffer::Clear()
{
	m_endIndex 			= 0;
	m_remainingCapacity = k_containerSize;
}

bool CVideoBuffer::Write( uint8_t *rawBufferIn, size_t bufferSizeIn, bool shouldSignalConditionIn )
{	
	// Make sure input buffer exists
	if( rawBufferIn == nullptr )
	{
		cerr << "Unable to perform write. Non-existent input buffer!" << endl;
		return false;
	}	

	uv_mutex_lock( &m_mutex );
	{
		// Check capacity
		if( bufferSizeIn > m_remainingCapacity )
		{
			cerr << "Video buffer full. Resetting state and dropping old frames." << endl;
			Clear();
		}
		
		// Copy the source buffer to the end of the buffer
		memcpy( End(), rawBufferIn, bufferSizeIn );
		
		// Update index and capacity
		m_endIndex 			+= bufferSizeIn;
		m_remainingCapacity -= bufferSizeIn;
		
		if( shouldSignalConditionIn )
		{
			// Signal to the consumer that data is available
			uv_cond_signal( &m_dataAvailableCondition );
		}
	}
	uv_mutex_unlock( &m_mutex );
	
	return true;
}

