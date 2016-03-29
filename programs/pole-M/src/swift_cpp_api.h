#pragma once

// Define the protocal type of Input data.
enum pole_protype
{
	POLE_PROTYPE_ZEROMQ = 0x00001,
	POLE_PROTYPE_HTTP = 0x00010,
	POLE_PROTYPE_REDIS = 0x00100,
	POLE_PROTYPE_MTTP = 0x01000,
	POLE_PROTYPE_MFPTP = 0x10000
};

int swift_vms_call(void *W);

