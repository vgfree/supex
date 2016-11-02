#pragma once
typedef enum _RETURNvalue
{
	GV_OK = 0,		// succeed
	GV_ERR = -1,		// failed
	GV_FILTER = -2,		// can't find key
	GV_PARSEJSON = -3	// parse json failed
} RETURNvalue;

