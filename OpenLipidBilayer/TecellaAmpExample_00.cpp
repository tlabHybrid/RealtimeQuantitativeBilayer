/******************************************************************************
// TecellaAmpExample_00.cpp
//
// This code uses TecellaAmp.lib to aqcuire curent from Tecella PICO amplifier.
//   Specific to Tecella amplifiers, so you need to replace this one if you need amplifiers of other manufacturers.
// 
// Modified from TecellaAmp API v1.6.3.
******************************************************************************/

#include "TecellaAmpExample_00.h"


/******************************************************************************
* Setup Gui  - Almost the same as the raw TecellaAmp API
******************************************************************************/
// This function displays what features are supported and
// what valid values are for certain settings.
//
void setup_gui(TECELLA_HNDL h)
{
	//Get properties of the DLL currently loaded.
	TECELLA_LIB_PROPS lib_props;
	tecella_get_lib_props(&lib_props);
	wprintf(L"\nTecellaAmp.dll Version: %d.%d.%d\n",
		lib_props.v_maj, lib_props.v_min, lib_props.v_dot);

	//Get various hardware properties / features.
	//The GUI should disable/limit parts of the interface
	//   that aren't supported by hardware.
	TECELLA_HW_PROPS hw_props;
	tecella_get_hw_props(h, &hw_props);
	wprintf(L"\nSome of the hardware properties: \n");
	wprintf(L"\tAmplifier: %s\n", hw_props.device_name);
	wprintf(L"\tSerial #: %s\n", hw_props.serial_number);
	wprintf(L"\tFirmware Version: %d\n", hw_props.hwvers);
	wprintf(L"\tChannel Count: %d\n", hw_props.nchans);
	wprintf(L"\tNumber of selectable sources: %d\n", hw_props.nsources);
	wprintf(L"\tNumber of selectable gains: %d\n", hw_props.ngains);
	wprintf(L"\tNumber of selectable stimuli: %d\n", hw_props.nstimuli);
	wprintf(L"\tMin. Sample Period: %lf\n", hw_props.sample_period_min);
	wprintf(L"\tMax. Sample Period: %lf\n", hw_props.sample_period_max);
	wprintf(L"\tMaximum number of segments per stimuli: %d\n", hw_props.max_stimulus_segments);
	wprintf(L"\tCan be used as an oscilloscope: %s\n", hw_props.supports_oscope ? L"Yes" : L"No");
	wprintf(L"\tSupports voltage stimulus: %s\n", hw_props.supports_vcmd ? L"Yes" : L"No");
	wprintf(L"\tSupports current stimulus: %s\n", hw_props.supports_icmd ? L"Yes" : L"No");

	//Individual units of the same model may have different gain choices
	//Determine what those choices are...
	// [10M, 100M, 1G, 3.3G, 10G]
	const wchar_t* source_label;
	wprintf(L"\nGain Choices:\n");
	for (int g = 0; g < hw_props.ngains; ++g)
	{
		tecella_get_gain_label(h, g, &source_label);
		wprintf(L"\t%s\n", source_label);
	}

	//Individual units of the same model may have different source choices
	//Determine what those choices are...
	// [None, Head, VModel]
	const wchar_t* gain_label;
	wprintf(L"\nSource Choices:\n");
	for (int s = 0; s < hw_props.nsources; ++s)
	{
		tecella_get_source_label(h, s, &gain_label);
		wprintf(L"\t%s\n", gain_label);
	}

	//Iterate through all the registers and determine what's supported.
	//The GUI should disable changes to unsupported registers.
	//The GUI should also limit register values to the proper range.
	const int TECELLA_REGISTER_COUNT = 8;
	TECELLA_REGISTER all_regs[TECELLA_REGISTER_COUNT] = {
		TECELLA_REG_CFAST,
		TECELLA_REG_CSLOW_A,
		TECELLA_REG_CSLOW_B,
		TECELLA_REG_CSLOW_C,
		TECELLA_REG_CSLOW_D,
		TECELLA_REG_RSERIES,
		TECELLA_REG_LEAK,
		TECELLA_REG_JP
	};

	wprintf(L"\nSupported Registers:\n");
	for (int i = 0; i < TECELLA_REGISTER_COUNT; ++i)
	{
		TECELLA_REG_PROPS reg_props;
		tecella_get_reg_props(h, all_regs[i], &reg_props);

		if (reg_props.supported) {
			wprintf(L"%s:\n", reg_props.label);
			wprintf(L"\tValue Units: %s\n", reg_props.units);
			wprintf(L"\tMin Value: %lf\n", reg_props.v_min);
			wprintf(L"\tMax Value: %lf\n", reg_props.v_max);
			wprintf(L"\tValue LSB / Interval / Precision: %lf\n", reg_props.v_lsb);
		}
	}

}


/******************************************************************************
* Source and Gain - Modified (fixing bugs.)
******************************************************************************/
// This function simply sets up the source and gain
// on a per-channel basis.
//
void setup_source_and_gain(TECELLA_HNDL h)
{
	TECELLA_HW_PROPS hw_props;
	tecella_get_hw_props(h, &hw_props);

	// [None, Head, VModel]
	int channel = 0;
	int source = 1;
	const wchar_t* source_label;
	tecella_get_source_label(h, source, &source_label);
	wprintf(L"\nSelecting source for channel %d to %s...\n", channel + 1, source_label);
	tecella_chan_set_source(h, channel, source);

	// Bug fix.  Somehow, the source won't change to Head unless we forcefully and randomly switch them several times.
	tecella_chan_set_source(h, 0, 0);
	tecella_chan_set_source(h, 0, 1);
	tecella_chan_set_source(h, 0, 2);
	tecella_chan_set_source(h, 0, 1);


	// [10M, 100M, 1G, 3.3G, 10G]
	int gain = 2;
	const wchar_t* gain_label;
	tecella_get_gain_label(h, gain, &gain_label);
	wprintf(L"\nSelecting gain for channel %d to %s...\n", channel + 1, gain_label);
	tecella_chan_set_gain(h, channel, gain);

	// Bug fix.  Somehow, the gain won't change to 1G unless we forcefully and randomly switch them several times.
	tecella_chan_set_gain(h, 0, 1);
	tecella_chan_set_gain(h, 0, 2);
	tecella_chan_set_gain(h, 0, 3);
	tecella_chan_set_gain(h, 0, 2);

	//wprintf(L"\nNevermind, setting all channels to lowest gain...\n");
	//tecella_chan_set_gain(h, TECELLA_ALLCHAN, TECELLA_GAIN_A);
}


/******************************************************************************
* Auto Compensation - Modified (added compensation functions.)
******************************************************************************/
// Auto offset zeroes out the graph.
// Auto compensation sets Leak, Cfast, and the Cslows to remove any
//  parasitics in the response.
//
void setup_auto_compensation(TECELLA_HNDL h)
{
	// Calibrates the amplifier.
	wprintf(L"\nRunning internal calibration...\n");
	tecella_calibrate_all(h);

	//Compensating individual channels isn't supported yet,
	// so we don't pass in a channel array here.
	wprintf(L"\nRunning Auto Offset...\n");
	tecella_auto_offset(h);

	wprintf(L"\nRunning Auto Compensation...\n");
	tecella_auto_comp(h);

	int channel = 0;
	wprintf(L"\tResults for channel %d:\n", channel + 1);

	double value;
	TECELLA_REG_PROPS reg_props;

	tecella_get_reg_props(h, TECELLA_REG_JP, &reg_props);
	if (reg_props.supported) {
		tecella_chan_get(h, TECELLA_REG_JP, channel, &value);
		wprintf(L"\t%s: %lf %s\n", reg_props.label, value, reg_props.units);
	}
	tecella_get_reg_props(h, TECELLA_REG_JP_FINE, &reg_props);
	if (reg_props.supported) {
		tecella_chan_get(h, TECELLA_REG_JP_FINE, channel, &value);
		wprintf(L"\t%s: %lf %s\n", reg_props.label, value, reg_props.units);
	}
	tecella_get_reg_props(h, TECELLA_REG_LEAK, &reg_props);
	if (reg_props.supported) {
		tecella_chan_get(h, TECELLA_REG_LEAK, channel, &value);
		wprintf(L"\t%s: %lf %s\n", reg_props.label, value, reg_props.units);
	}
	tecella_get_reg_props(h, TECELLA_REG_CFAST, &reg_props);
	if (reg_props.supported) {
		tecella_chan_get(h, TECELLA_REG_CFAST, channel, &value);
		wprintf(L"\t%s: %lf %s\n", reg_props.label, value, reg_props.units);
	}
	tecella_get_reg_props(h, TECELLA_REG_CSLOW_A, &reg_props);
	if (reg_props.supported) {
		tecella_chan_get(h, TECELLA_REG_CSLOW_A, channel, &value);
		wprintf(L"\t%s: %lf %s\n", reg_props.label, value, reg_props.units);
	}
	tecella_get_reg_props(h, TECELLA_REG_CSLOW_B, &reg_props);
	if (reg_props.supported) {
		tecella_chan_get(h, TECELLA_REG_CSLOW_B, channel, &value);
		wprintf(L"\t%s: %lf %s\n", reg_props.label, value, reg_props.units);
	}
	tecella_get_reg_props(h, TECELLA_REG_CSLOW_C, &reg_props);
	if (reg_props.supported) {
		tecella_chan_get(h, TECELLA_REG_CSLOW_C, channel, &value);
		wprintf(L"\t%s: %lf %s\n", reg_props.label, value, reg_props.units);
	}
	tecella_get_reg_props(h, TECELLA_REG_CSLOW_D, &reg_props);
	if (reg_props.supported) {
		tecella_chan_get(h, TECELLA_REG_CSLOW_D, channel, &value);
		wprintf(L"\t%s: %lf %s\n", reg_props.label, value, reg_props.units);
	}

	//wprintf(L"\nRunning Auto Artifact Removal...\n");
	//tecella_auto_artifact_update(h);


	// Bug fix.  Somehow, the offset values are not well set by the auto_comp functions. 
	// Manually set the value to 5 mV.
	tecella_get_reg_props(h, TECELLA_REG_JP, &reg_props);
	if (reg_props.supported) {
		tecella_chan_set(h, TECELLA_REG_JP, channel, 6.0);
		tecella_chan_get(h, TECELLA_REG_JP, channel, &value);
		wprintf(L"\t%s: %lf %s\n", reg_props.label, value, reg_props.units);
	}
	tecella_get_reg_props(h, TECELLA_REG_JP_FINE, &reg_props);
	if (reg_props.supported) {
		tecella_chan_set(h, TECELLA_REG_JP_FINE, channel, -1.0);
		tecella_chan_get(h, TECELLA_REG_JP_FINE, channel, &value);
		wprintf(L"\t%s: %lf %s\n", reg_props.label, value, reg_props.units);
	}
}


/******************************************************************************
* Per-channel settings - Modified (predetermined the Bessel cutoff frequency to 1 kHz.)
******************************************************************************/
// This function shows how to set any of the registers from TECELLA_REGISTER.
// Note that proper units should be used.
//
// If you don't care about the units, you can use the tecella_reg_set_pct(,,,)
// functions instead.
//
void setup_per_channel_settings(TECELLA_HNDL h)
{
	wprintf(L"\nSetting up per channel settings...\n");

	TECELLA_HW_PROPS hw_props;
	tecella_get_hw_props(h, &hw_props);

	//Autocomp already set up leak, cfast, and the cslows
	// so lets change some other settings.
	int ivalue;
	double dvalue;
	TECELLA_REG_PROPS reg_props;

	//Bessel
	wprintf(L"Setting bessel cutoff freq for all channels:\n");
	double bessel_cutoff_freq_kHz = 1.0;
	tecella_bessel_freq2value(h, bessel_cutoff_freq_kHz, &ivalue);
	tecella_chan_set_bessel(h, TECELLA_ALLCHAN, ivalue);
	wprintf(L"\tRequested frequency: %lf kHz\n", bessel_cutoff_freq_kHz);
	tecella_bessel_value2freq(h, ivalue, &bessel_cutoff_freq_kHz);
	wprintf(L"\tActual frequency: %lf kHz\n", bessel_cutoff_freq_kHz);

	//Disable a register if it can be disabled, otherwise set it to it's min value.
	tecella_get_reg_props(h, TECELLA_REG_RSERIES, &reg_props);
	wprintf(L"Disabling %s for all channels.\n", reg_props.label);
	if (reg_props.can_be_disabled) {
		tecella_chan_set_enable(h, TECELLA_REG_RSERIES, TECELLA_ALLCHAN, false);
	}
	else {
		tecella_chan_set(h, TECELLA_REG_RSERIES, TECELLA_ALLCHAN, reg_props.v_min);
	}
}


/******************************************************************************
* Stimulus  - Modified (set the hold stimulus value to 50 mV.)
******************************************************************************/
// This function shows how to program the stimulus for both
// single stimulus systems and multi stimulus systems.
void setup_stimulus(TECELLA_HNDL h)
{
	TECELLA_HW_PROPS hw_props;
	tecella_get_hw_props(h, &hw_props);

	wprintf(L"\nSetting up the stimuli...\n");

	int channel = 0; 
	double voltage = 50e-3;
	tecella_stimulus_set_hold(h, voltage);
	
	//wprintf(L"\nSetting up the software filter...\n");
	//tecella_sw_filter_enable(h, true);
	//tecella_sw_filter_auto_downsample(h, true);
	
	//Create a stimulus
	const int SEGMENT_COUNT = 3;
	TECELLA_STIMULUS_SEGMENT stimulus[SEGMENT_COUNT] = {
		{TECELLA_STIMULUS_SEGMENT_SET, 50e-3, 0, 250e-3, 0},   //50 milliVolts, 250ms (.25 seconds)
		{TECELLA_STIMULUS_SEGMENT_SET, 50e-3, 0, 500e-3, 0},   //50 milliVolts, 500ms (.5 seconds)
		{TECELLA_STIMULUS_SEGMENT_SET, 50e-3, 0, 250e-3, 0},   //50 milliVolts, 250ms (.25 seconds)
	};

	//Program all available stimuli
	for (int i = 0; i < hw_props.nstimuli; ++i)
	{
		wprintf(L"\tProgramming stimulus %d to a %lf V pulse.\n", i + 1, stimulus[1].value);
		int step_count = 1;
		int repeat_count = 1;
		tecella_stimulus_set(h, stimulus, SEGMENT_COUNT, step_count, repeat_count, i);

		//get the actuall programmd vcmd, which may differ due to the precision of the hardware.
		int returned_segment_count;
		tecella_stimulus_get(h, stimulus, SEGMENT_COUNT, &returned_segment_count, &step_count, &repeat_count, i);
		wprintf(L"\t            stimulus %d is a %lf V pulse in reality.\n", i + 1, stimulus[1].value);

		//make it such that the stimulus are different.
		stimulus[1].value += 10e-3;
	}
}


/******************************************************************************
* Acquire function
* Acquire WITHOUT Callback  - Modified
    * Added the arguments (pointers for data returning)
    * Deleted the file export functions
	* Change the constant values to enable 5 kHz sampling.
	    * sample_period_multiplier = 8
		* buffer_size = 1250
		* acquisition loop num = 4
    * Bug fix: changed the scale value after tecella_acquire_i2d_scale() to 1e12. (It was 1e9 in the raw API, but I guess pico = 1e-12.)
******************************************************************************/
// This function acquires the digitized current value from the amplifier
// and return the value to the specified pointer.
void acquire_without_callback(TECELLA_HNDL h, double* timestamp_arg, double* destination_arg)
{
	TECELLA_HW_PROPS hw_props;
	tecella_get_hw_props(h, &hw_props);

	//Sets the API's internal buffer to hold up to 2 seconds worth of data per channel.
	tecella_acquire_set_buffer_size(h, 20000 * 2);

	//Unset the callback functions
	tecella_stimulus_set_callback(h, 0);
	tecella_acquire_set_callback(h, 0);

	//start acquisition
	wprintf(L"\tStarting acquisition.\n");
	int sample_period_multiplier = 8;
	tecella_acquire_start(h, sample_period_multiplier, false); 
	wprintf(L"\tAcquisition thread started, main thread will now read data as it is acquired.\n");

	//read the data for each channel one at a time.
	int channel = 0;
	const int buffer_size = 1250;
	short samples[buffer_size];
	unsigned int samples_requested = buffer_size;
	unsigned int samples_returned = 0;

	bool more_samples_left = true;
	int idx = 0;
	while (more_samples_left)
	{
		// Stop the acquisition if the 5000> samples are collected.
		// the loop will continue until there are no more samples.
		idx++;
		if (idx >= 5) {
			tecella_acquire_stop(h);
			wprintf(L"Stopping acquisition.\n");
		}

		//tecella_acquire_read_* blocks until the number of requested samples are ready
		//  or until acquisition has ended.
		unsigned long long timestamp;
		bool last_sample_flag;
		tecella_acquire_read_i(h, channel, samples_requested, samples, &samples_returned, &timestamp, &last_sample_flag);
		if (last_sample_flag && samples_returned == 0) {
			more_samples_left = false;
		}

		//output samples to the destination pointers
		if (idx <= 4) {
			double scale;
			tecella_acquire_i2d_scale(h, channel, &scale);
			scale *= 1e12;   // convert scale from amps to picoamps
			for (unsigned int i = 0; i < samples_returned; ++i) {
				timestamp_arg[(idx - 1) * buffer_size + i] = double(timestamp + i);
				destination_arg[(idx - 1) * buffer_size + i] = scale * samples[i];
			}
		}	
		wprintf(L"\tAcquired %d samples from all channels. \n", samples_returned);
	}
	tecella_acquire_stop(h);
	wprintf(L"\tAcquisition complete.\n");
}


/******************************************************************************
* Acquire stop
******************************************************************************/
void acquire_stop(TECELLA_HNDL h) {
	tecella_acquire_stop(h);
	wprintf(L"\tAcquisition stopped.\n");
}
