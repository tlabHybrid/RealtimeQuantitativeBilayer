/******************************************************************************
// ActuationSerial.cpp
//
// This code sends instructions to the stepper motor based on the processed signal.

******************************************************************************/

#include "MyHelper.h"

void conductActuationSerial(bool rupture_flag, bool recovery_flag, int number_of_channel) {
    if (!rupture_flag) {
        if (number_of_channel >= 2) {
            // If more than 2 molecules are on the lipid bilayer, the processing will be much difficult. Reform it.
            sendSerial("r\n");
        }
    }
    else {
        if (!recovery_flag) {
            // If the bilayer is ruptured, reform it.
            // However, if the bilayer is already in process of reformation, do not send the signal.
            sendSerial("r\n");
        }
    }
}