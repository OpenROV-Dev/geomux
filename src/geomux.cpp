// Includes
#include <iostream>

#include "CProcess.h"
#include "Utility.h"

using namespace std;

// Create process object
std::unique_ptr<CProcess> process;

void StartThread( const Nan::FunctionCallbackInfo<v8::Value>& args ) 
{
	if( !process )
	{
		args.GetReturnValue().Set( -1 );
		return;
	}
	
	// Start the background thread
	int ret = process->StartThread();
	
	// Set return value
	args.GetReturnValue().Set( ret );
}

void RegisterCallback( const Nan::FunctionCallbackInfo< v8::Value>& args )
{
	if( !process )
	{
		args.GetReturnValue().Set( -1 );
		return;
	}
	
	// Create callback from input function
	process->m_sharedData.m_pCallback = new Nan::Callback( args[0].As<v8::Function>() );
	
	if( !process->m_sharedData.m_pCallback )
	{
		// Failed
		args.GetReturnValue().Set( false );
	}
	else
	{
		// Success
		args.GetReturnValue().Set( true );
	}
}

void FinishedFrame( const Nan::FunctionCallbackInfo< v8::Value>& args )
{
	// uv_mutex_lock( &process->m_sharedData.m_outputBuffer.m_mutex );
	// process->m_sharedData.m_isSendingFrame = false;
	// process->m_sharedData.m_outputBuffer.Clear();
	// uv_mutex_unlock( &process->m_sharedData.m_outputBuffer.m_mutex );
	
	// auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	// cerr << "Finish at: " << now << endl;
}

void Initialize( v8::Local<v8::Object> exports ) 
{
	exports->Set( Nan::New( "StartThread" ).ToLocalChecked(), Nan::New<v8::FunctionTemplate>( StartThread )->GetFunction() );
	exports->Set( Nan::New( "RegisterCallback" ).ToLocalChecked(), Nan::New<v8::FunctionTemplate>( RegisterCallback )->GetFunction() );
	exports->Set( Nan::New( "FinishedFrame" ).ToLocalChecked(), Nan::New<v8::FunctionTemplate>( FinishedFrame )->GetFunction() );
	
	try
	{
		process = util::make_unique<CProcess>();
	}
	catch( std::exception &e )
	{
		cerr << "Exception creating CProcess instance: " << e.what() << endl;
	}
}

NODE_MODULE( geomux, Initialize )