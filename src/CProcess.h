#pragma once

// Includes
#include <zmq.hpp>
#include <nan.h>
#include <memory>
#include "CVideoBuffer.h"
#include "CGeoCam.h"
#include "CMuxer.h"

struct TSharedData
{
	// Attributes
	uv_async_t 		m_asyncSendVideo;		// Async listener so that we can trigger the video send callback in the libuv thread from the muxer thread
	uv_loop_t 		*m_pDefaultLoop;		// Default loop for executing code in the main V8 event loop thread
	Nan::Callback 	*m_pCallback;			// Contains a user defined javascript callback that takes in a buffer of muxed video data
	
	CVideoBuffer 	m_inputBuffer;			// Contains frames coming in from the GC-6500
	//CVideoBuffer 	m_outputBuffer;			// Contains muxed frames to be sent out to JS-land through the registered callback
	
	bool			m_isComposingInitFrame;
	bool			m_holdBuffer;
	
	zmq::context_t m_zmqContext;
    	zmq::socket_t m_zmqPublisher;

	// Methods
	TSharedData()
		: m_pDefaultLoop( uv_default_loop() )
		, m_pCallback( nullptr )
		, m_isComposingInitFrame( false )
		, m_zmqContext( 1 )
		, m_zmqPublisher( m_zmqContext, ZMQ_PUB )
	{
	}
	
	void Cleanup()
	{		
		m_isComposingInitFrame 	= false;
		m_holdBuffer		= false;
		
		// Reset video buffers
		m_inputBuffer.Clear();
		//m_outputBuffer.Clear();
	}
	
	void SendAsyncVideoSignal()
	{
		uv_async_send( &m_asyncSendVideo );
	}
};

class CProcess
{
public:
	// Attributes
    TSharedData m_sharedData;
	
	// Methods
	CProcess();
	
	int StartThread();
	void StopThread();
	
private:
	// Attributes
	uv_thread_t 				m_bgThread;

	std::unique_ptr<CGeoCam> 	m_pGeoCam;
	std::unique_ptr<CMuxer> 	m_pMuxer;

	bool						m_initialized;
	bool						m_stopThread;
	
	// Methods
	int Initialize();
	void Update();
	void Cleanup();
	
	static void ThreadLoop( void *arg );
	static void SendVideoData( uv_async_t *handle );
	static void EmptyFreeCallback( char* data, void* hint );
};

