#include "pvirx_support.h"
#include "pvirx_table.h"
#include "pvirx_user_config.h"

/* below defined "PVI_initialize()" */
enum _eCameraStandard DEFAULT_CAMERA_STANDARD = HDA;
enum _eCameraResolution DEFAULT_CAMERA_RESOLUTION = camera_1280x720p25;

//////////////////////////////////////////////////////////////////////////////////////////////
_stAttrChip PVIRX_ATTR_CHIP[MAX_PVIRX_CHANCNT] = 
{/*{{{*/
	//chan 0 // PARALLEL interface
	{ /*{{{*/
		0x00, //channel address of chan
		1 //unsigned char vinMode; => 1:Single VinP pin, 3:Single VinN pin, 0:Differential VinPN pin.
	},/*}}}*/
	//chan 1 // PARALLEL interface
	{ /*{{{*/
		0x01, //channel address of chan
		1 //unsigned char vinMode; => 1:Single VinP pin, 3:Single VinN pin, 0:Differential VinPN pin.
	},/*}}}*/
	//chan 2 // PARALLEL interface
	{ /*{{{*/
		0x02, //channel address of chan
		1 //unsigned char vinMode; => 1:Single VinP pin, 3:Single VinN pin, 0:Differential VinPN pin.
	},/*}}}*/
	//chan 3 // PARALLEL interface
	{ /*{{{*/
		0x03, //channel address of chan
		1 //unsigned char vinMode; => 1:Single VinP pin, 3:Single VinN pin, 0:Differential VinPN pin.
	}/*}}}*/
};/*}}}*/

//////////////////////////////////////////////////////////////////////////////////////////////////////

