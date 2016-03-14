// Includes
#include "CProcess.h"
#include <unistd.h>
#include <iostream>
#include <chrono>

#include "Utility.h"

// Namespaces
using namespace std;

CProcess::CProcess()
	: m_pGeoCam( util::make_unique<CGeoCam>( &m_sharedData ) )
	, m_pMuxer( util::make_unique<CMuxer>( &m_sharedData ) )
	, m_initialized( false )
	, m_stopThread( false )
{
	m_sharedData.m_asyncSendVideo.data = this;
}

int CProcess::StartThread()
{
	// Kick off the muxer thread
	int ret = uv_thread_create( &m_bgThread, ThreadLoop, this );
	
	if( ret < 0 )
	{
		cerr << "Failure creating muxer thread!" << endl;
		return ret;
	}
	
	// Success
	return 0;
}

void CProcess::StopThread()
{
	m_stopThread = true;
}

int CProcess::Initialize()
{
	m_sharedData.m_zmqPublisher.bind("ipc:///tmp/geomux.ipc" );

	// Initialize the async listener for SendVideoData
	int ret = uv_async_init( m_sharedData.m_pDefaultLoop, &m_sharedData.m_asyncSendVideo, SendVideoData );
	
	if( ret < 0 )
	{
		cerr << "Failure initializing async listener for SendVideoData!" << endl;
		return ret;
	}
	
	m_pMuxer->Initialize();
	
	// Start the GC-6500 and have it start pumping data through the pipeline
	if( !m_pGeoCam->StartVideo() )
	{
		cerr << "Failed to start GC6500's video stream!" << endl;
		return -1;
	}
	
	// Wait a bit
	usleep( 50000 );
	
	// Force an initial I-frame to increase the chances of getting good analysis data for the muxer right off the bat
	if( !m_pGeoCam->ForceIFrame() )
	{
		cerr << "Failed to force initial i-frame!" << endl;
		return -1;
	}
	
	return 0;
}

void CProcess::Update()
{
	try
	{
		// Basically use this to sleep the thread when there is nothing to do
		cerr << "Lock" << endl;
		uv_mutex_lock( &m_sharedData.m_inputBuffer.m_mutex );
			while( m_sharedData.m_inputBuffer.GetSize() == 0 )
			{
				cerr << "Entering condition var and unlocking during sleep" << endl;
				uv_cond_wait( &m_sharedData.m_inputBuffer.m_dataAvailableCondition, &m_sharedData.m_inputBuffer.m_mutex );
				cerr << "Relocked by condition var" << endl;
			}
		cerr << "Update() Unlock" << endl;
		uv_mutex_unlock( &m_sharedData.m_inputBuffer.m_mutex );
		
		// TODO: check if there is actually data, not just rely on condition variable
		
		cerr << "Call to CMuxer update" << endl;
		
		// Run the muxer's update function
		m_pMuxer->Update();
	}
	catch(...)
	{
		cerr << "Exception caught in CProcess::Update()" << endl;
	}
}

void CProcess::Cleanup()
{
	// Turn off the camera stream to shut off input
	m_pGeoCam->StopVideo();
	
	// Close the async watcher on the main thread to shut off output
	uv_close( (uv_handle_t*)&m_sharedData.m_asyncSendVideo, NULL );
	
	m_initialized 	= false;
	m_stopThread 	= false;
}

void CProcess::ThreadLoop( void *processIn )
{
	if( !processIn )
	{
		cerr << "No process pointer given to thread!" << endl;
		return;
	}
	
	// Get shared data
	CProcess *proc 			= (CProcess*)processIn;

	// Initialize the process subsystems
	if( proc->Initialize() < 0 )
	{
		cerr << "Failed to initialize process!" << endl;
	}
	else
	{
		// Success
		proc->m_initialized = true;
	}
	
	cerr << "BG Thread running..." << endl;
	
	if( proc->m_initialized )
	{
		while( !proc->m_stopThread ) 
		{
			proc->Update();
		}
	}
	
	// Cleanup
	proc->Cleanup();
	
	cerr << "Muxer Thread finished running!" << endl;
}

void CProcess::SendVideoData( uv_async_t *handle )
{
	// CProcess *proc = (CProcess*)handle->data;
	
	// if( proc->m_sharedData.m_pCallback )
	// {
	// 	Nan::HandleScope scope;
		
	// 	uv_mutex_lock( &proc->m_sharedData.m_outputBuffer.m_mutex );
		
	// 	// Drop the frame and clear the buffer
	// 	Nan::MaybeLocal<v8::Object> buffer = Nan::NewBuffer( (char*)proc->m_sharedData.m_outputBuffer.Begin(), proc->m_sharedData.m_outputBuffer.GetSize(), EmptyFreeCallback, nullptr  );
		
	// 	uv_mutex_unlock( &proc->m_sharedData.m_outputBuffer.m_mutex );
		
	// 	const unsigned argc = 1;
		
	// 	v8::Local<v8::Value> argv[argc] = { buffer.ToLocalChecked() };
		
	// 	proc->m_sharedData.m_pCallback->Call(argc, argv);
	// }

	// auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	// //cerr << "Callback at: " << now << endl;
}


void CProcess::EmptyFreeCallback( char* data, void* hint )
{
	// Do nothing
	cerr << "EmptyFreeCallback triggered" << endl;
}
