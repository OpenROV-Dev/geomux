// Includes
#include "CGeoCam.h"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <cstring>

 #include <chrono>

#include "CProcess.h"

// Namespaces
using namespace std;

CGeoCam::CGeoCam( TSharedData *sharedDataIn )
	: CGeoCam( GEOCAM_DEFAULT_DEVICE_OFFSET, sharedDataIn )
{
	
}

CGeoCam::CGeoCam( int deviceOffsetIn, TSharedData *sharedDataIn )
	: m_pSharedData( sharedDataIn )
{
	// Set a custom device offset
	stringstream options( "dev_offset=" );
	options << deviceOffsetIn;
	
	// Initialize mxuvc
	if( mxuvc_video_init( "v4l2", options.str().c_str() ) )
	{
		throw std::runtime_error( "Failed to initialize mxuvc!" );
	}
	
	// Register our video callback
	if( mxuvc_video_register_cb( CH_MAIN, CGeoCam::VideoCallback, this ) )
	{
		Cleanup();
		throw std::runtime_error( "Failed to register video callback!" );
	}
}

CGeoCam::~CGeoCam()
{
	Cleanup();
}

bool CGeoCam::StartVideo()
{
	return ( mxuvc_video_start( CH_MAIN ) == 0 );
}

bool CGeoCam::StopVideo()
{
	return ( mxuvc_video_stop( CH_MAIN ) == 0 );
}

bool CGeoCam::ForceIFrame()
{
	return ( mxuvc_video_force_iframe( CH_MAIN ) == 0 );
}

bool CGeoCam::IsAlive()
{
	// Check to see if camera is available. False if its been unplugged or USB host is not responding
	// Note: Documentation is wrong. 1 == Alive, 0 == Dead
	return ( mxuvc_video_alive() == 1 );
}

void CGeoCam::Cleanup()
{
	// Deinitialize mxuvc resources and exit. Automatically calls mxuvc_video_stop()
	mxuvc_video_deinit();
}

// This gets called by the MXUVC library every time we have a NAL available
void CGeoCam::VideoCallback( unsigned char *dataBufferOut, unsigned int bufferSizeIn, video_info_t infoIn, void *userDataIn )
{
	CGeoCam *cam = (CGeoCam*)userDataIn;

	//auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	//cerr << "Update at: " << now << endl;

	// Write data to the video buffer
	cam->m_pSharedData->m_inputBuffer.Write( dataBufferOut, bufferSizeIn );
	
	// Releases the buffer back to the MXUVC
	mxuvc_video_cb_buf_done( CH_MAIN, infoIn.buf_index );
}
