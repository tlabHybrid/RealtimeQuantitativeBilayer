# Bila-kit
This repository contains the software for an article [HOGEHOGE](https://www.kikagaku.ai/).

## What is Bila-kit?
Bila-kit is a toolkit for lipid bilayer researchers, providing them with the in-situ current acquisition and the following analysis & feedback system. Therefore, the researchers can automate lipid bilayer experiments using Bila-kit. 

## Currently applicable to:
- [x]  Immediate recovery from measurement failures (e.g. rupture or failure of lipid bilayers) without human supervision or intervention.
- [x]  Automatic acquisition of a large amount of data on the electrical conductance of biological nanopores (e.g. alpha-hemolysin) to create a histogram without manual labor, for the efficient research and investigation of membrane proteins.
- [x]  Real-time and in-situ estimation of the applied physiological stimuli (e.g. membrane potential) from the gating current and the corresponding open probability of ion channels (e.g. big-potassium channel), for the real-time sensing applications.

## Currently dealing with:
- [ ] Application of the platform to drug discovery industry,like promotor/inhibitor investigation for ion channels.
- [ ] Measurement of membrane proteins with low S/N ratio (i.e. with too small conductances).
- [ ] Introduction of object-oriented programming to the source codes (poor structuring for now).
- [ ] Improvement in algorithms for determining the state/number of membrane proteins.
- [ ] Introducing a better peripherals to actuation system for better experimental results.


# Installation procedure

## Preinstall the necessary software & hardware
The following software/libraries/packages and hardware are required for Bila-kit to properly function. If you don't have any of these, install them before using Bila-kit.

### Microsoft Visual Studio
* Download and install the IDE from [the official homepage](https://visualstudio.microsoft.com/ja/vs/).
  * Bila-kit is developed by Visual Studio Community 2019.

### Qt
* Download and install the package from [the official homepage](https://www.qt.io/ja-jp/download-open-source).
  * Bila-kit uses Qt 5.15.2 (MSVC 2019 32-bit) in the open-source edition.

### Connecting Qt to Visual Studio
* Open "Extensions > Manage Extensions" in Visual Studio.
  * Search "Qt Visual Studio Tools" and install.
  * Open "Extensions > Qt VS Tools > Qt versions".
  * Add qmake paths to the displayed panel.
  * Restart Visual Studio.

### Tecella amplifier and TecellaAmp API/DLLs
Although you can futuristically incorporate any lipid bilayer amplifiers from any manufacturers to Bila-kit, it currently integrated with only Tecella amplifiers and corresponding public API/DLLs.

[NOTE] The purpose to use API/DLLs is to acquire the digitized current in-situ and on-line, not after experiments. Therefore, **if you find other manufacturer's API/DLLs more suitable or discover other way to acquire the digitized current in-situ and on-line, you can replace them with Tecella amplifier and API/DLLs in this paper**. 

* Obtain a Tecella amplifier.
  * Bila-kit uses [PICO](http://www.tecella.com/pico.html).
  * Install necessary drivers by the manufacturer's instructions.

### Arduino and stepper motors
You can use your preferable microcomputers for driving peripheral devices (such as stepper motors). Just for example, we use Arduino MEGA. 

* Obtain a microcomputer and a stepper motor.
  * Bila-kit uses Arduino Mega 2560 R3 and 28BYJ-48 (with ULN2003 motor driver), respectively.

## Download this repository
* Download this repository to your preferable working directory.


# Usage

## Setup the hardware
![Device image](/Bila-kit/images/device.PNG)

### Amplifier
* Connect the amplifier with your PC.

### Arduino and a stepper motor
* Connect the Arduino MEGA with your PC.
* Connect the stepper motor to the driver.
* Connect the motor driver to Arduino MEGA.
  * In Bila-kit, 9 ~ 12 pins of Arduino are connected to the motor driver.
* Power the motor driver with a 9V battery.
  * We recommend not to use a socket to power the motor to reduce the effect of electromagnetical noise.
* If needed, you can use a housing of a stepper motor.
  * In Bila-kit, we used [a publicly available housing for 28BYJ-48 & ULN2003](https://www.thingiverse.com/thing:5145361)

### Lipid bilayer environment
Lipid bilayers can be created in many ways. In this paper, we used [Droplet Contact Method](https://www.nature.com/articles/srep01995), which utilizes a perforated separator to stabilize the lipid bilayer while maintaining ease of operation.

The rotatable device in Bila-kit is already described in the paper. Here, we just overview the experimental procedure.

* Construct the lipid bilayer chamber with an appropriate surface coating and electrical wirings.
* Install the rotation table onto a shaft of the stepper motor.
* Infuse lipid-oil mixture into the chamber.
* Infuse electrolyte solution with membrane proteins into the chamber.

## Run the application
Here is the procedure to run the Bila-kit application.

### Setup
* Build and run the application in Visual Studio.
* Rotate the lipid bilayer chamber with the "rotation" button to the appropriate position.
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
If you want to use Bila-kit without Visual Studio (e.g. other user's PC), deployment is possible by using the files in Bila-kit/deploy/.
* First, you need to make a Release build.
* From Release/, copy the exe file into Bila-kit/deploy/.
* Now check that the file can be executed.


# About a licence
Bila-kit is open-source except manufacturer's API/DLLs, so you can freely modify this application to meet your demands. Although not obligatory, we would really appreciate if you cite the following paper.

[HOGE]


