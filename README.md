# RealtimeQuantitativeBilayer
This repository contains a software for the research article of [Real-time quantitative characterization of ion channel activities for automated control of a lipid bilayer system](https://www.journals.elsevier.com/biosensors-and-bioelectronics/forthcoming-special-issues/crossing-the-laboratory-borders-volume-i-health-biosensors-and-diagnostic-tools-from-the-lab-to-the-fab).

## What is this?
This is a toolkit for lipid bilayer researchers, providing them with the in-situ current acquisition and the subsequent analysis & response. Therefore, the researchers can introduce the notion of "automated control" into lipid bilayer experiments. 

## Currently applicable to:
- [x]  Real-time and in-situ estimation of the applied physiological stimuli (e.g. membrane potential) from the gating current and the corresponding open probability of ion channels (e.g. big-potassium channel), for the real-time sensing applications.
- [x]  Immediate recovery from measurement failures (e.g. rupture or failure of lipid bilayers) without human supervision or intervention.
- [x]  Automatic acquisition of a large amount of data on the electrical conductance of biological nanopores (e.g. alpha-hemolysin) to create a histogram without manual labor, for the efficient research and investigation of membrane proteins.

## Currently dealing with:
- [ ] Application of the platform to drug discovery industry,like promotor/inhibitor investigation for ion channels.
- [ ] Measurement of membrane proteins with low S/N ratio (i.e. with too small conductances).
- [ ] Improvement in algorithms for determining the state/number of membrane proteins.


# Installation procedure

## Preinstall the necessary software & hardware
The following software/libraries/packages and hardware are required for "RealtimeQuantitativeBilayer" to properly function. If you don't have any of these, install them in advance.

### Microsoft Visual Studio
* Download and install the IDE from [the official homepage](https://visualstudio.microsoft.com/ja/vs/).
  * The software is developed by Visual Studio Community 2019.

### Qt
* Download and install the package from [the official homepage](https://www.qt.io/ja-jp/download-open-source).
  * The software uses Qt 5.15.2 (MSVC 2019 32-bit) in the open-source edition.

### Connecting Qt to Visual Studio
* Open "Extensions > Manage Extensions" in Visual Studio.
  * Search "Qt Visual Studio Tools" and install.
  * Open "Extensions > Qt VS Tools > Qt versions".
  * Add qmake paths to the displayed panel.
  * Restart Visual Studio.

### Tecella amplifier and TecellaAmp API/DLLs
Although you can futuristically incorporate any lipid bilayer amplifiers from any manufacturers, the software currently integrated with only Tecella amplifiers and corresponding public API/DLLs.

[NOTE] The purpose to use API/DLLs is to acquire the digitized current in-situ and on-line, not after experiments. Therefore, **if you find other manufacturer's API/DLLs more suitable or discover other way to acquire the digitized current in-situ and on-line, you can replace them with Tecella amplifier and API/DLLs in this paper**. 

* Obtain a Tecella amplifier.
  * The software uses [PICO](http://www.tecella.com/pico.html).
  * Install necessary drivers by the manufacturer's instructions.

### Arduino and hardware peripherals
You can use your preferable microcomputers for driving peripheral devices (such as syringe pumps or stepper motors). 

* Obtain a microcomputer and hardware equipment.
  * Just for a reference, the software uses Arduino Mega 2560 R3 and LEGATO® 180, respectively.

## Download this repository
* Download this repository to your preferable working directory.


# Usage

## Setup the hardware

### Amplifier
* Connect the amplifier with your PC.

### Arduino and hardware peripherals
* Connect the Arduino MEGA with your PC.
* Connect the hardware to their appropriate drivers.

### Lipid bilayer environment
Lipid bilayers can be created in many ways. In this paper, we used [Droplet Contact Method](https://www.nature.com/articles/srep01995), which utilizes a perforated separator to stabilize the lipid bilayer while maintaining ease of operation.
* Construct the lipid bilayer chamber with an appropriate surface coating and electrical wirings.
* Infuse lipid-oil mixture into the chamber.
* Infuse electrolyte solution with membrane proteins into the chamber.

## Run the application
Here is the procedure to run the application.

### Setup
* Build and run the application in Visual Studio.
* Check the functionality of re-formation system.  If you use the syringe pump, press the button on the upper right and check that an air bubble is infused in the lipid bilayer chamber.
* Cover the whole system with Faraday Cage.
* Select "Amplifier" as the data source. 
  * [Note] You can reload the recorded local data by selecting "ATF" or "CSV" columns. For detail of data loader, please refer to the source code.
* Select the appropriate protein as the protein type.
* Select the appropriate postprocessing method.
* Press "Setup" button and wait until the connection and calibration is finished.
* Enter the appropriate conductance and bias membrane voltage.

### Acquire
* Press "Acquire" button to start the acquisition.
  * The graph is automatically scrolls.
  * Every second, the raw current value, the idealized data, the post processed data (open probability, estimated stimuli, etc.) are exported to CSV files in "log" directory.

### Stop
* If you want to terminate the software, press "Stop" button before killing the process for graceful termination.

## Analysis
* The postprocessed data is already exported in "log" folder.
* You can use them to easily create your own reports.


# Deploy
If you want to use the software without Visual Studio (e.g. other user's PC), deployment is possible by using the files in Bila-kit/deploy/.
* First, you need to make a Release build.
* From Release/, copy the exe file into Bila-kit/deploy/, which contains the necessary DLLs.
* Now check that the file can be executed.


# About a licence
This software is open-source except manufacturer's API/DLLs, so you can freely modify this application to meet your demands. Although not obligatory, we would really appreciate if you cite the following paper.

[Real-time quantitative characterization of ion channel activities for automated control of a lipid bilayer system](https://www.journals.elsevier.com/biosensors-and-bioelectronics/forthcoming-special-issues/crossing-the-laboratory-borders-volume-i-health-biosensors-and-diagnostic-tools-from-the-lab-to-the-fab)

