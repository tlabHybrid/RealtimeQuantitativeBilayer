// TecellaAmpExample_00.cpp
// Uses TecellaAmp.lib to aqcuire curent from Tecella PICO amplifier.

#include "TecellaAmpExample_00.h"
#include "MyMain.h"

/******************************************************************************
* Main Function  - Deleted (main() is already in main.cpp.  The content is transferred to measure_init() in Measurement.cpp)
******************************************************************************/
/*
int main(int argc, char** argv)
{
	tecella_debug("tecella_debug.log");

	//Initialize the device.
	//Autodetect doesn't work yet, so you must explicitly specify the device model.
	TECELLA_ERRNUM err;
	TECELLA_HNDL h;
	//if( err = tecella_initialize(&h, TECELLA_HW_MODEL_AUTO_DETECT) )	{
	if (err = tecella_initialize(&h, TECELLA_HW_MODEL_AUTO_DETECT)) {
		wprintf(tecella_error_message(err));
		return 1;
	}

	//Determine what's supported and configure the GUI.
	setup_gui(h);  wprintf(L"\nPress any key to continue.\n"); _getch();

	//Utility functions
	// Use only if your hardware supports this.
	//test_utility_set_stimulus(h); wprintf(L"\nPress any key to continue.\n"); _getch();

	//Change various settings of the device
	setup_source_and_gain(h);  wprintf(L"\nPress any key to continue.\n"); _getch();
	setup_auto_compensation(h);  wprintf(L"\nPress any key to continue.\n"); _getch();
	setup_per_channel_settings(h);  wprintf(L"\nPress any key to continue.\n"); _getch();
	setup_stimulus(h);  wprintf(L"\nPress any key to continue.\n"); _getch();
	//Acquire using different methods
	acquire_without_callback(h, false);  wprintf(L"\nPress any key to continue.\n"); _getch();
	acquire_with_callback(h, false);  wprintf(L"\nPress any key to continue.\n"); _getch();
	acquire_without_callback(h, true);  wprintf(L"\nPress any key to continue.\n"); _getch();
	acquire_with_callback(h, true);  wprintf(L"\nPress any key to continue.\n"); _getch();

	//Clean up and exit.
	tecella_finalize(h);
	return 0;
}
*/


/******************************************************************************
* Utility  - Deleted (Unused)
******************************************************************************/
//This function sets the selected stimulus of the 4 additional channels.
/*
void test_utility_set_stimulus(TECELLA_HNDL h)
{
	wprintf(L"\nApplying -200mV, -100mV, 100mV, 200mV to the first four utility channels respectively.\n");
	tecella_stimulus_set_hold(h, -200e-3, 0);
	tecella_stimulus_set_hold(h, -100e-3, 1);
	tecella_stimulus_set_hold(h, 100e-3, 2);
	tecella_stimulus_set_hold(h, 200e-3, 3);

	tecella_utility_set_stimulus(h, 0, 0);
	tecella_utility_set_stimulus(h, 1, 1);
	tecella_utility_set_stimulus(h, 2, 2);
	tecella_utility_set_stimulus(h, 3, 3);

	wprintf(L"Applying -200mV, -100mV, 100mV, 200mV to the first four regular channels respectively.\n");
	tecella_chan_set_stimulus(h, 0, 0);
	tecella_chan_set_stimulus(h, 1, 1);
	tecella_chan_set_stimulus(h, 2, 2);
	tecella_chan_set_stimulus(h, 3, 3);
}
*/


/******************************************************************************
* Setup Gui  - Modified (Almost the same as the bare TecellaAmp API)
******************************************************************************/
//This function doesn't actually setup a GUI, but
// it uses all the API functions that will help
// a GUI determine what features are supported and
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
	int source = 1; //rand() % hw_props.nsources;
	const wchar_t* source_label;
	tecella_get_source_label(h, source, &source_label);
	wprintf(L"\nSelecting source for channel %d to %s...\n", channel + 1, source_label);
	tecella_chan_set_source(h, channel, source);

	// Somehow the source is not バグけし．何故か何回かsourceを行ったり来たりしないとうまくHeadに切り替わらない．
	tecella_chan_set_source(h, 0, 0);
	tecella_chan_set_source(h, 0, 1);
	tecella_chan_set_source(h, 0, 2);
	tecella_chan_set_source(h, 0, 1);


	// [10M, 100M, 1G, 3.3G, 10G]
	int gain = 2; // rand() % hw_props.ngains;
	const wchar_t* gain_label;
	tecella_get_gain_label(h, gain, &gain_label);
	wprintf(L"\nSelecting gain for channel %d to %s...\n", channel + 1, gain_label);
	tecella_chan_set_gain(h, channel, gain);

	tecella_chan_set_gain(h, 0, 1);
	tecella_chan_set_gain(h, 0, 2);
	tecella_chan_set_gain(h, 0, 3);
	tecella_chan_set_gain(h, 0, 2);

	//wprintf(L"\nNevermind, setting all channels to lowest gain...\n");
	//tecella_chan_set_gain(h, TECELLA_ALLCHAN, TECELLA_GAIN_A);
}


/******************************************************************************
* Auto Compensation
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

	// バグけし．Offsetはうまくいかないことが多いので，5mV位に手動で補正する
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
* Per-channel settings
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

	/*
	//Set a register to a random valid value (with proper precision.)
	//This method should result in a value that doesn't get rounded
	// to a different value when it's programmed in hardware.
	tecella_get_reg_props(h, TECELLA_REG_JP, &reg_props);
	if (reg_props.supported)
	{
		wprintf(L"Setting %s to a random valid value WITH proper precision:\n", reg_props.label);
		for (int channel = 0; channel < hw_props.nchans; ++channel)
		{
			//uses lsb:
			dvalue = reg_props.v_min +
				reg_props.v_lsb * (rand() % reg_props.v_divisions);
			tecella_chan_set(h, TECELLA_REG_JP, channel, dvalue);
			wprintf(L"\tWrite %s of channel %d --> %lf\n", reg_props.label, channel, dvalue);

			tecella_chan_get(h, TECELLA_REG_JP, channel, &dvalue);
			wprintf(L"\tRead  %s of channel %d --> %lf\n", reg_props.label, channel, dvalue);
		}
	}

	//Set a register to a random valid value (without proper precision.)
	//The actual value may vary once it's programmed in the hardware
	tecella_get_reg_props(h, TECELLA_REG_JP, &reg_props);
	if (reg_props.supported)
	{
		wprintf(L"Setting %s to a random valid value WITHOUT proper precision:\n", reg_props.label);
		for (int channel = 0; channel < hw_props.nchans; ++channel)
		{
			//doesn't use lsb:
			dvalue = reg_props.v_min +
				(reg_props.v_max - reg_props.v_min) * rand() / RAND_MAX;
			tecella_chan_set(h, TECELLA_REG_JP, channel, dvalue);
			wprintf(L"\tWrite %s of channel %d --> %lf\n", reg_props.label, channel, dvalue);

			tecella_chan_get(h, TECELLA_REG_JP, channel, &dvalue);
			wprintf(L"\tRead  %s of channel %d --> %lf\n", reg_props.label, channel, dvalue);
		}
	}
	*/
}


/******************************************************************************
* Stimulus
******************************************************************************/
// This function shows how to program the stimulus for both
// single stimulus systems and multi stimulus systems.
void setup_stimulus(TECELLA_HNDL h)
{
	TECELLA_HW_PROPS hw_props;
	tecella_get_hw_props(h, &hw_props);

	wprintf(L"\nSetting up the stimuli...\n");

	// vcmdを有効にしないと強制的に0Vになってしまうらしい
	int channel = 0; 
	//bool isenable;
	//tecella_chan_get_vcmd_enable(h, channel, &isenable);
	//if(!isenable) tecella_chan_set_vcmd_enable(h, channel, true);

	// 50mVでホールドするだけでよい．
	double voltage = 50e-3;
	tecella_stimulus_set_hold(h, voltage);
	

	//wprintf(L"\nSetting up the software filter...\n");
	//あと，sampling周期を5kHzにしたい (multiplierをx8にする) ので，以下のコマンドが必要？
	//必要そうでなければ消すこと．
	//tecella_sw_filter_enable(h, true);
	//tecella_sw_filter_auto_downsample(h, true);
	
	
	// 旧TecellaAmpExample00.cppと同じにするため，再度復帰
	//Create a stimulus
	const int SEGMENT_COUNT = 3;
	TECELLA_STIMULUS_SEGMENT stimulus[SEGMENT_COUNT] = {
		{TECELLA_STIMULUS_SEGMENT_SET, 50e-3, 0, 250e-3, 0},   //0 milliVolts, 250ms (.25 seconds)
		{TECELLA_STIMULUS_SEGMENT_SET, 50e-3, 0, 500e-3, 0},   //0 milliVolts, 500ms (.5 seconds)
		{TECELLA_STIMULUS_SEGMENT_SET, 50e-3, 0, 250e-3, 0},   //0 milliVolts, 250ms (.25 seconds)
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
* Acquire start - Modified
******************************************************************************/
void acquire_start(TECELLA_HNDL h) {
	//以下， tecella_acquire_start()についてTecellaAmp.hからコピー
	/** Sets up an acquisition and returns once the acquisition has begun.
	When the function returns, an internal thread runs asynchronously that constantly pulls data from the device and pushes it into the internal queue.
	@param h A handle to an initialized device.
	@param sample_period_multiplier The actual sample period used will be sample_period_min times sample_peiriod_multiplier.
	@param continuous If true, acquisition continues until tecella_acquire_stop() is called.  If false, acquisition continues for as long as the longest programmed stimulus or until tecella_acquire_stop() is called.
	@param start_stimuli If true, all stimuli start playing at the start of acquisition and also capture continuously if continuous_stimuli is ture.  To use different options for different stimuli set both continuous_stimuli and start_stimuli to false and call tecella_acquire_start_stimulus() for each stimulus before calling tecella_acquire_start().
	@param continuous_stimuli Valid only if continuous is true.  If true, all stimuli will capture continuously and also start playing if start_stimuli is true.  To use different options for different stimuli set both continuous_stimuli and start_stimuli to false and call tecella_acquire_start_stimulus() for each stimulus before calling tecella_acquire_start().
	@param start_on_trigger If true, acquisition will not start until an external hardware trigger is detected.
	*/

}


/******************************************************************************
* Acquire stop - Modified
******************************************************************************/
void acquire_stop(TECELLA_HNDL h) {
	// acquireを止める．スレッドが終了する．
	tecella_acquire_stop(h);
	wprintf(L"\tAcquisition stopped.\n");
}


/******************************************************************************
* Acquire WITHOUT Callback - Modified
******************************************************************************/
void acquire_without_callback(TECELLA_HNDL h, double* timestamp_arg, double* destination_arg)
{
	TECELLA_HW_PROPS hw_props;
	tecella_get_hw_props(h, &hw_props);

	//Sets the API's internal buffer to hold up to 2 seconds worth of data per channel.
	tecella_acquire_set_buffer_size(h, 20000 * 2);

	//Unset the callback functions
	tecella_stimulus_set_callback(h, 0);
	tecella_acquire_set_callback(h, 0);

	//Open a text file to ouput the samples to
	//FILE* fp = 0;
	//fp = fopen("capture.txt", "w");
	//fprintf(fp, "timestamp, pA\n");

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
		//stop the acquisition if the 5000> samples are collected.
		//the loop will continue until there are no more samples.
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

		//output samples
		//output samples to a file if not in continuous mode
		//  and it's the first channel
		//if (fp && channel == 0)
		//{
		if (idx <= 4) {
			double scale;
			tecella_acquire_i2d_scale(h, channel, &scale);
			scale *= 1e12;   //convert scale from amps to picoamps   .... ...  ... .. . . . .  pAって10^-12では！！！！？？？？
			for (unsigned int i = 0; i < samples_returned; ++i) {
				//fprintf(fp, "%llu, %lf\n", timestamp, scale * samples[i]);
				// destination = samples に代入する処理をここに書く
				timestamp_arg[(idx - 1) * buffer_size + i] = double(timestamp + i);
				destination_arg[(idx - 1) * buffer_size + i] = scale * samples[i];
			}
		}
		//}
			
		wprintf(L"\tAcquired %d samples from all channels. \n", samples_returned);

	}
		
	tecella_acquire_stop(h);

	//if (fp) { fclose(fp); }

	

	wprintf(L"\tAcquisition complete.\n");
}



/******************************************************************************
* The Stimulus Callback Function
******************************************************************************/
//the stimulus callback function runs in a separate thread
// independent of the acquisition thread and the main thread
// but in the same thread as the acquire callback
void stimulus_callback_function(TECELLA_HNDL h, int stimulus_index, unsigned long long timestamp)
{
	//wprintf(L"\n\tWe have been notified that stimulus #%d started at timestamp %d", stimulus_index, timestamp);
}

/******************************************************************************
* The Acquisition Callback Function
******************************************************************************/
//the acquisition callback function runs in a separate thread
// independent of the acquisition thread and the main thread
// but in the same thread as the stimulus callback
void acquire_callback_function(TECELLA_HNDL h, int channel, unsigned int sample_count)
{
	const unsigned int buffer_size = 1024;
	short samples[buffer_size];
	unsigned int samples_requested = 0;
	unsigned int samples_returned = 0;

	if (channel == 0) {
		wprintf(L"\n\tAcquiring %d samples from channel %d", sample_count, channel);
	}
	else {
		wprintf(L",%d", channel);
	}

	while (sample_count > 0)
	{
		if (buffer_size < sample_count) {
			samples_requested = buffer_size;
		}
		else {
			samples_requested = sample_count;
		}

		unsigned long long timestamp;
		bool last_sample_flag;
		tecella_acquire_read_i(h, channel, samples_requested, samples, &samples_returned, &timestamp, &last_sample_flag);

		//wprintf(L"(samples_left=%d,t=%d,len=%d)", sample_count, timestamp, samples_returned);
		if (last_sample_flag) {
			wprintf(L"\nLast sample received. (Means stimulus or acquisition has ended.)");
			//do any cleanup for the current stimulus
			// and any setup for the next stimulus (if any)
		}

		//store samples somewhere in a thread-safe way
		double scale;
		tecella_acquire_i2d_scale(h, channel, &scale);
		double first_sample_in_amps = scale * samples[0];


		FILE* fp = 0;
		fopen_s(&fp, "capture.txt", "w"); // upgrade
		if (fp) {
			for (unsigned int i = 0; i < samples_returned; ++i) {
				fprintf(fp, "%llu, %lf\n", timestamp, scale * samples[i]);
			}
			fclose(fp);
		}
		

		if (samples_returned == 0) {
			sample_count = 0;
			wprintf(L"\nSomething is wrong.  This should never happen.");
		}
		else {
			sample_count -= samples_returned;
		}
	}
}

/******************************************************************************
* Acquire WITH Callback
******************************************************************************/
void acquire_with_callback(TECELLA_HNDL h, bool continuous)
{
	wprintf(L"\nRunning acquisition WITH a callback in %s mode.\n", continuous ? L"Continuous" : L"Single Shot");

	//Sets the API's internal buffer to hold up to 2 seconds worth of data per channel.
	tecella_acquire_set_buffer_size(h, 20000 * 2);

	//Set the callback functions
	tecella_stimulus_set_callback(h, stimulus_callback_function);
	tecella_acquire_set_callback(h, acquire_callback_function);

	//start acquisition
	wprintf(L"\tStarting acquisition.\n");
	int sample_period_multiplier = 1;
	tecella_acquire_start(h, sample_period_multiplier, continuous);
	wprintf(L"\tAcquisition thread started and callback thread started.\n");
	wprintf(L"\tMain thread can do whatever it wants now, but be careful when sharing data with the callback thread.\n");

	//let main thread do GUI stuff or whatever.
	//in this case we'll just sleep.
	int i = 0;
	while (i < 20 || continuous)
	{
		Sleep(63);
		wprintf(L"\nMain thread slept for 63ms.  Press any key to stop acquire.");

		//if a button is pressed stop the acquisition.
		if (_kbhit()) {
			_getch();
			tecella_acquire_stop(h);
			wprintf(L"\nStopping acquisition.\n");
			Sleep(63);
			break;
		}

		++i;
	}
	
}