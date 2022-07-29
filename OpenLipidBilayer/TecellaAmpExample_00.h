#pragma once

/*******************************************
* TAmpExample_00.cpp
* Copyright 2008 Tecella LLC
*******************************************/

#include "TecellaAmp.h"
//conio.h is included for the kbhit and getch functions()
//windows.h is only needed for the Sleep() function
// you may need to use a different header+functions for different compilers.
#include <conio.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


/******************************************************************************
* Function Declarations
******************************************************************************/
//void test_utility_set_stimulus(TECELLA_HNDL h);
void setup_gui(TECELLA_HNDL h);
void setup_source_and_gain(TECELLA_HNDL h);
void setup_auto_compensation(TECELLA_HNDL h);
void setup_per_channel_settings(TECELLA_HNDL h);
void setup_stimulus(TECELLA_HNDL h);


void acquire_start(TECELLA_HNDL h);		// [obsolete] should be deleted.
void acquire_stop(TECELLA_HNDL h);		// Stop acquireing.
void acquire_without_callback(TECELLA_HNDL h, double* timestamp, double* destination);  // Acquire current by blocking manner. (Tecella specific function)
void acquire_with_callback(TECELLA_HNDL h, bool continuous);  // [obsolete] do not use.
