/******************************************************************************
// SenseAmplifier.cpp
//
// This code is basically a wrapper of TecellaAmpExample_00.cpp, which is more specific to Tecella amplifier.
// You can wrap other amplifiers in a similar manner.
******************************************************************************/

#include "MyHelper.h"
#include "TecellaAmp.h"
#include "TecellaAmpExample_00.h"

TECELLA_HNDL h;

// Connection and initialization of the amplifier.
int setupAmplifier(int choice) {

	// To review the console outputs, the setting in
	// Projects > Property > Configuration Property > Linker > System > Subsystem 
	// to "Console".  You can change this to "Windows" to hide the console window.
	tecella_debug("tecella_debug.log");

	//Initialize the device.
	TECELLA_ERRNUM err;
	if (err = tecella_initialize(&h, TECELLA_HW_MODEL_AUTO_DETECT)) {
		wprintf(tecella_error_message(err));
		return 1;
	}
	//Determine what's supported and configure the GUI.
	setup_gui(h);

	//Change various settings of the device
	if (choice == 0) {
		setup_auto_compensation(h);
	}
	setup_source_and_gain(h);
	setup_per_channel_settings(h);
	setup_stimulus(h);

	return 0;
}

// Conduct the acquisition. (This function is just a wrapper)
void readAmplifier(double* timestamp, double* destination, int dataIndex_loop_num) {
	acquire_without_callback(h, timestamp, destination);

	// The timestamp in the above function is not working well, so this code manually inputs the timestamps.
	for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
		timestamp[idx] = dataIndex_loop_num + idx / double(SAMPLE_FREQ);
	}
}

// Stop the acquisition.
void stopAmplifier() {
	acquire_stop(h);
}

// Completely terminates the connection of amplifier and releases allocated memories.
int finalizeAmplifier() {
	tecella_finalize(h);
	return 0;
}


// (Re)set the value for membrane holding voltage.  Input example: 50 -> 50 mV.
void changeVoltageAmplifier(int value) {
	double voltage = value * 0.001;
	tecella_stimulus_set_hold(h, voltage);
}