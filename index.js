var zmq = require('zmq');
var binary = require('node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname, './package.json')));
var addon = require(binding_path);
var Readable = require('stream').Readable;
var util = require('util');
var initFrame = null;
var initFrame_counter = 0;
const EventEmitter = require('events');
var e = new EventEmitter();

var initFrameSub = zmq.socket('sub');
var subscriber = zmq.socket('sub');

subscriber.connect( "ipc:///tmp/geomux.ipc" );
initFrameSub.connect( "ipc:///tmp/geomux.ipc" );

var initFilter = "init";
var filter = "geo";

initFrameSub.subscribe( initFilter );
subscriber.subscribe( filter );

initFrameSub.on( 'message', function(topic, data)
{
    initFrame = data;
    e.emit('initData', initFrame);
	//console.log( "Got init frame: " );
        //console.log( data.length );
} );


subscriber.on( 'message', function(topic,data) {
	//console.log( "Got video frame: " );
	//console.log( data.length );
        e.emit('data',data);
} );

/*
util.inherits(VideoStream, Readable);
// Create VideoStream from readableStream prototype
function VideoStream(opt) {
  Readable.call(this, opt);
}

// Custom read function
VideoStream.prototype._read = function (n) {
};

///////////////////////////
var teststream = new VideoStream();
teststream.setEncoding('binary');
*/

// Run the callback passed to the c++ process, lets c++ process write a packet of data to the stream
addon.RegisterCallback(function (dataIn) {
  //console.log( dataIn );
  //teststream.push(dataIn);
  //addon.FinishedFrame();	
});

/*
var fs = require('fs');
// Listen for events on the video stream
teststream.on('data', function (chunk) {
  if (initFrame_counter == 1) {
    initFrame += chunk;
    initFrame_counter++;
    teststream.emit('initData', initFrame);
  }
  if (initFrame_counter == 0) {
    initFrame = chunk;
    initFrame_counter++;
  }  //  console.log('got %d bytes of data', chunk.length );
     //Test code that dumps to a file. Be sure to remove the file if it exists first
     //  fs.appendFileSync('/tmp/myfile.mp4',chunk,"binary");
});
teststream.on('end', function () {
  console.log('there will be no more data.');
});

*/

// Start camera processor thread
console.log(addon.StartThread());
module.exports = {
  videoStream: e,
  initFrame: initFrame
};
