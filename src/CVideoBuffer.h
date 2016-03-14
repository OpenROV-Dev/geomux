#pragma once

// Includes
#include <nan.h>
#include <cstdint>
#include <sys/types.h>
#include <memory>

class CVideoBuffer
{
public:
	// Attributes		
	uv_mutex_t 						m_mutex;
	uv_cond_t 						m_dataAvailableCondition;
	
	// Methods
	CVideoBuffer( size_t defaultAllocationSizeIn = 5000000 ); 	// Default to ~5MB
	
	size_t GetSize();
	size_t GetRemainingCapacity();
	size_t GetReservedSize();
	
	uint8_t* Begin();
	uint8_t* End();
	
	void Clear();
	bool Write( uint8_t *rawBufferIn, size_t bufferSizeIn, bool shouldSignalConditionIn = true );
	
private:
	// Attributes
	const size_t					k_containerSize;	
	std::unique_ptr<uint8_t[]>		m_data;
	size_t 							m_endIndex;
	size_t 							m_remainingCapacity;
};