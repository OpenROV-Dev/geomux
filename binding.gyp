{
    "targets":
    [
        {
            "target_name":  "geomux",
            "sources":      [ '<!@(ls -1 src/*.cpp)' ],

            "link_settings":
            {
		"ldflags":
		[
		  "-L/usr/lib/geocamera/",
		  "-L/usr/lib/"
		],

		"libraries":
		[
		  "-lzmq",

		  "-lmxcam",
		  "-lmxuvc",

		  "-lavformat",
		  "-lavcodec",
		  "-lavutil",
		  "-lswresample",
		  "-lswscale",
		  "-lx264"
		],
	    },


            "include_dirs":
            [
		"<!(node -e \"require('nan')\")",
		"/usr/include/geocamera/",
		"/usr/include/"
	    ],
	    'defines':
	    [
	      'VIDEO_BACKEND=\"v4l2\"'
	    ],
            "cflags!": [ "-fno-exceptions" ],
	    "cflags":
	    [
	      "-std=c++11 -Wno-literal-suffix"
	    ],
	    "cflags_cc!": [ "-fno-exceptions" ]
        },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
    ]
}
