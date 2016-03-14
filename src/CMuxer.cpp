// Includes
#include "CMuxer.h"
#include <iostream>

#include "CProcess.h"

using namespace std;

CMuxer::CMuxer( TSharedData *sharedDataIn )
	: m_pSharedData( sharedDataIn )
	, m_last( std::chrono::high_resolution_clock::now() )
{
}

CMuxer::~CMuxer()
{
}
	
void CMuxer::Initialize()
{
	// av_log_set_level(AV_LOG_TRACE);
	
	// Initialize libavcodec, and register all codecs and formats.
	av_register_all();
	
	////////////// INPUT 
	
	if( !( m_pInputFormatContext = avformat_alloc_context() ) ) 
	{
        cerr << "Failed to allocate format context!" << endl;
		return;
    }
	
	m_pInputAvioContextBuffer = (uint8_t*)av_malloc( m_avioContextBufferSize );
	
	// Set buffer to put data in, set buffer size, set data source, and packet function
    m_pInputAvioContext = avio_alloc_context( m_pInputAvioContextBuffer, m_avioContextBufferSize, 0, this, &ReadPacket, NULL, NULL );
	
	if( !m_pInputAvioContext ) 
	{
       	cerr << "Failed to allocate avio context!" << endl;
		return;
    }
	
    m_pInputFormatContext->pb 		= m_pInputAvioContext;
	m_pInputFormatContext->flags 	= AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_NOBUFFER;
	
	//////////// OUTPUT
		
	avformat_alloc_output_context2( &m_pOutputFormatContext, NULL, "mp4", NULL );
	
    if( !m_pOutputFormatContext ) 
	{
       	cerr << "Could not create output context!" << endl;
        return;
    }
	
	m_pOutputFormat 			= m_pOutputFormatContext->oformat;
	m_pOutputAvioContextBuffer 	= (uint8_t*)av_malloc(m_avioContextBufferSize);
	
	// Set buffer to put data in, set buffer size, set data source, and packet function
    m_pOutputAvioContext = avio_alloc_context( m_pOutputAvioContextBuffer, m_avioContextBufferSize, 1, m_pSharedData, NULL, &WritePacket, NULL );
	
	if( !m_pOutputAvioContext ) 
	{
       	cerr << "Failed to allocate output avio context!" << endl;
		return;
    }
	
    m_pOutputFormatContext->pb 		= m_pOutputAvioContext;
	m_pOutputFormatContext->flags 	= AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;
}

void CMuxer::Cleanup()
{	
	avformat_close_input( &m_pInputFormatContext );
	
    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if( m_pInputAvioContext ) 
	{
        //av_freep( &m_pInputAvioContext->buffer );
        av_freep( &m_pInputAvioContext) ;
    }
}

void CMuxer::Update()
{
	if( !m_formatAcquired )
	{
		if( !m_pInputFormatContext->iformat )
		{			
			// Lock
			cerr << "format lock" << endl;
			uv_mutex_lock(&m_pSharedData->m_inputBuffer.m_mutex);
			
			AVProbeData probeData;
			probeData.buf 		= m_pSharedData->m_inputBuffer.Begin();
			probeData.buf_size 	= m_pSharedData->m_inputBuffer.GetSize();
			probeData.filename 	= "";
	
			int score = AVPROBE_SCORE_MAX / 4;
	
			// This seems to help get enough data for proper muxing. write_header is missing some avcC data if you dont
			if( probeData.buf_size > 300000 )
			{
				m_pInputFormatContext->iformat = av_probe_input_format2( &probeData, 1, &score );
			}
			else
			{
				// Unlock
				cerr << "format unlock A" << endl;
				uv_mutex_unlock( &m_pSharedData->m_inputBuffer.m_mutex );
				return;
			}
			
		 	if( !m_pInputFormatContext->iformat )
			{
				m_pSharedData->m_inputBuffer.Clear();
			}
			
			// Unlock
			cerr << "format unlock B" << endl;
			uv_mutex_unlock( &m_pSharedData->m_inputBuffer.m_mutex );
		}
		
		if( m_pInputFormatContext->iformat )
		{	
			cerr << "Got input format." << endl;
						
			int ret = avformat_open_input( &m_pInputFormatContext, NULL, NULL, NULL );
			
			if (ret < 0)
			{
				cerr << "Could not open input!" << endl;
				return;
			}
			else
			{
				cerr << "Input opened successfully!" << endl;
				
				m_formatAcquired = true;
				
				//av_dump_format(m_pInputFormatContext, 0, 0, 0);
				
				if( avformat_find_stream_info( m_pInputFormatContext, NULL ) < 0 )
				{
					cerr << "Unable to find stream info!" << endl;
				}
				
				// Find the decoder for the video stream
				m_pInputCodecContext = m_pInputFormatContext->streams[0]->codec;
				
				AVCodec *pCodec = avcodec_find_decoder(m_pInputCodecContext->codec_id);
				
				if (pCodec == NULL) 
				{
					cerr << "Failed to find decoder" << endl;
					return;
				}				
				
				// Open codec
				if (avcodec_open2(m_pInputCodecContext, pCodec, NULL ) < 0) 
				{
					cerr << "Failed to open decoder" << endl;
					return;
				}
				
				// Open codec for output
				
				cerr << "Setting up output context..." << endl;
				
				m_pOutputFormatContext->oformat->flags |= AVFMT_ALLOW_FLUSH;
				
				for (size_t i = 0; i < m_pInputFormatContext->nb_streams; i++) 
				{
					cerr << "Adding stream" << endl;
					
					AVStream *in_stream = m_pInputFormatContext->streams[i];
					AVStream *out_stream = avformat_new_stream( m_pOutputFormatContext, in_stream->codec->codec );
					
					if(!out_stream) 
					{
						cerr << "Failed allocating output stream" << endl;
						return;
					}
			
					ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
					
					if (ret < 0) 
					{
						cerr << "Failed to copy context from input to output stream codec context" << endl;
						return;
					}
					
					// TODO: What was this?
					out_stream->codec->codec_tag = 0;					
					
					if (m_pOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
					{
						out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
					}
					
					out_stream->time_base = out_stream->codec->time_base;
				}
				
				m_canMux = true;
				
				av_dict_set( &m_pMuxerOptions, "movflags", "empty_moov+default_base_moof+frag_keyframe", 0 );
				
				// Write Header Frame (ftyp+moov)
				// The header writes twice. This forces it to buffer the two writes together before sending the async signal
				m_pSharedData->m_holdBuffer = true;
				ret = avformat_write_header( m_pOutputFormatContext, &m_pMuxerOptions );
				
				if (ret < 0) 
				{
					cerr << "Error occurred when writing header!" << endl;
					return;
				}
			}
		}
	}
	else
	{
		// Mux into something else here
		if( m_canMux )
		{
			AVPacket packet;
	
			// AVStream *in_stream, *out_stream;

			int ret = av_read_frame(m_pInputFormatContext, &packet);
			
			if (ret < 0)
			{
				return;
			}
	
			// in_stream  = m_pInputFormatContext->streams[packet.stream_index];
			// out_stream = m_pOutputFormatContext->streams[packet.stream_index];
	
			// // Not sure if any of this is needed
			// packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
			// packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF);
			// packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
			// packet.pos = -1;
	
			// Write moof+dat with one frame in it
			// Call the second time with a null packet to flush the buffer and trigger a write_packet
			ret = av_write_frame(m_pOutputFormatContext, &packet);
			ret = av_write_frame(m_pOutputFormatContext, NULL );

			if (ret < 0) 
			{
				return;
			}
			
			av_packet_unref(&packet);
		}
	}
}

// Custom read function for ffmpeg
int CMuxer::ReadPacket( void *muxerIn, uint8_t *avioBufferOut, int avioBufferSizeAvailableIn )
{
	// Convert opaque data to TSharedData
	CMuxer* muxer = (CMuxer*)muxerIn;
	TSharedData *sharedData = muxer->m_pSharedData;
	
	// Figure out how many bytes to copy into 
	int bytesToConsume = sharedData->m_inputBuffer.GetSize();
	
	// If the buffer size is bigger than the avioBuffer, we need to flush the avio buffer to make room, dropping old frames
	if( bytesToConsume > avioBufferSizeAvailableIn )
	{
		// TODO: Revisit
		// Flush
		avio_flush( muxer->m_pInputAvioContext );
		
		// Drop existing packets, since we can't fit them in the buffer. This might help the next time around
		cerr << "readpacket clear lock" << endl;
		uv_mutex_lock( &sharedData->m_inputBuffer.m_mutex );
		sharedData->m_inputBuffer.Clear();
		cerr << "readpacket clear unlock" << endl;
		uv_mutex_unlock( &sharedData->m_inputBuffer.m_mutex );
			
		// Read 0 bytes
		return 0; 
	}

	// Copy video data to avio buffer
	cerr << "readpacket memcpy lock" << endl;
	uv_mutex_lock( &sharedData->m_inputBuffer.m_mutex );
	memcpy( avioBufferOut, sharedData->m_inputBuffer.Begin(), bytesToConsume );
	sharedData->m_inputBuffer.Clear();
	cerr << "readpacket memcpy unlock" << endl;
	uv_mutex_unlock( &sharedData->m_inputBuffer.m_mutex );
	
	return bytesToConsume;
} 

// Custom read function for ffmpeg
int CMuxer::WritePacket( void *sharedDataIn, uint8_t *avioBufferIn, int bytesAvailableIn )
{
	// Convert opaque data to TSharedData
	TSharedData *sharedData = (TSharedData*)sharedDataIn;

	try
	{

	if( sharedData->m_holdBuffer )
	{
		
		if( !sharedData->m_isComposingInitFrame )
		{
			//std::cerr << "writing init frame part1" << std::endl;
			zmq::message_t topic( 4 );
			memcpy( topic.data(), "init", 4);
			sharedData->m_zmqPublisher.send( topic, ZMQ_SNDMORE );
			
			//zmq::message_t payload( bytesAvailableIn );
			//memcpy( payload.data(), (void*)avioBufferIn, bytesAvailableIn );
			//sharedData->m_zmqPublisher.send( payload, ZMQ_SNDMORE );
			
			sharedData->m_isComposingInitFrame = true;

			return 0;
		}
		else
		{
			//std::cerr << "writing init frame part 2" << std::endl;
			zmq::message_t payload( bytesAvailableIn );
			memcpy( payload.data(), (void*)avioBufferIn, bytesAvailableIn );
			sharedData->m_zmqPublisher.send( payload );
			
			sharedData->m_isComposingInitFrame = false;
			sharedData->m_holdBuffer = false;
		}
	}
	else
	{
		//zmq_msg_t topic;
		//zmq_msg_init_size (&topic, 3);
		//memcpy (zmq_msg_data (&topic), "geo", 3);
		//zmq_send(sharedData->m_zmqPublisher, &topic, ZMQ_SNDMORE );
	
		//zmq_msg_t payload;
                //zmq_msg_init_size (&payload, bytesAvailableIn);
                //memcpy (zmq_msg_data (&payload), (void*)avioBufferIn, bytesAvailableIn);
                //zmq_send(sharedData->m_zmqPublisher, &payload, 0 );


		//std::cerr << "Writing payload" << std::endl;
		zmq::message_t topic( 3 );
		memcpy( topic.data(), "geo", 3);
		sharedData->m_zmqPublisher.send( topic, ZMQ_SNDMORE );
		
	
			int out = sharedData->m_zmqPublisher.send((void*)avioBufferIn, bytesAvailableIn, 0 );
			
			//std::cerr << "PAyload send result: " << out <<  std::endl;
			
		}
	}	
	catch (const zmq::error_t& e)
	{	
		std::string errStr = e.what();
		std::cerr << "caught error: " << errStr << std::endl;
	}
	
	
	return bytesAvailableIn;
} 
