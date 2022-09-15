#pragma once

/*******************************************
* TAmpExample_00.cpp
* Copyright 2008 Tecella LLC
*******************************************/

#include "TecellaAmp.h"
// conio.h is included for the kbhit and getch functions()
// you may need to use a different header+functions for different compilers.
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>


/******************************************************************************
* Function Declarations - Modified
******************************************************************************/
void setup_gui(TECELLA_HNDL h);
void setup_source_and_gain(TECELLA_HNDL h);
void setup_auto_compensation(TECELLA_HNDL h);
void setup_per_channel_settings(TECELLA_HNDL h);
void setup_stimulus(TECELLA_HNDL h, double voltage = 0.050);

void acquire_without_callback(TECELLA_HNDL h, double* timestamp, double* destination);  // Acquire current by blocking manner. (Tecella specific function)
void acquire_stop(TECELLA_HNDL h);		// Stop acquireing.