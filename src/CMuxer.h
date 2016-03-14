#pragma once
 
// Includes
#include <nan.h>
#include <cstdint>
#include <sys/types.h>
 #include <chrono>
 
extern "C" 
{ 
	// FFmpeg
	#include <libavutil/avassert.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/opt.h>
	#include <libavutil/log.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/timestamp.h>
	#include "libavutil/pixfmt.h"
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <libavformat/avio.h>
	#include <libavcodec/avcodec.h>
}

// Forward Declarations
struct TSharedData;

class CMuxer
{
public:
	CMuxer( TSharedData *sharedDataIn );
	virtual ~CMuxer();
	
	TSharedData 		*m_pSharedData;

	size_t				m_avioContextBufferSize		= 4000000; // ~4mb

	// Input structures
	AVFormatContext 	*m_pInputFormatContext 		= NULL;
	AVIOContext 		*m_pInputAvioContext 		= NULL;
	AVCodecContext 		*m_pInputCodecContext		= NULL;

	uint8_t*			m_pInputAvioContextBuffer		= NULL;
	
	// Output structures
	AVFormatContext 	*m_pOutputFormatContext 	= NULL;
	AVOutputFormat 		*m_pOutputFormat 			= NULL;
	AVIOContext 		*m_pOutputAvioContext 		= NULL;
	
	uint8_t*			m_pOutputAvioContextBuffer	= NULL;		

	bool 				m_canMux = false;
	bool 				m_formatAcquired = false;
	
	AVDictionary 		*m_pMuxerOptions		= NULL;
	
	std::chrono::high_resolution_clock::time_point m_last;
	
	// Methods
	void Initialize();
	void Update();
	void Cleanup();
	
	// Custom read function for ffmpeg
	static int ReadPacket( void *muxerIn, uint8_t *avioBufferOut, int avioBufferSizeAvailableIn );
	static int WritePacket( void *sharedDataIn, uint8_t *avioBufferIn, int bytesAvailableIn );
};
	
	
	
		
	
				
