#pragma once
 
// Includes
#include <mxuvc.h>
#include <libmxcam.h>

// Defines
#define GEOCAM_DEFAULT_DEVICE_OFFSET 0		// TODO: Eventually remove this construct and have the javascript or some other agent request a specific interface

// Forward declarations
struct TSharedData;

class CGeoCam
{
public:
	TSharedData *m_pSharedData;

	CGeoCam( TSharedData *sharedDataIn );
	CGeoCam( int deviceOffsetIn, TSharedData *sharedDataIn );
	
	virtual ~CGeoCam();

	bool StartVideo();
	bool StopVideo();
	bool IsAlive();
	bool ForceIFrame();
	
	void Cleanup();
	
	static void VideoCallback( unsigned char *dataBufferOut, unsigned int bufferSizeIn, video_info_t infoIn, void *userDataIn );
};