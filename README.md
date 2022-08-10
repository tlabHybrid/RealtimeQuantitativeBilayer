# Bila-kit
This repository contains the software for an article [HOGEHOGE](https://www.kikagaku.ai/).

## What is Bila-kit?
Bila-kit is a toolkit for lipid bilayer researchers, providing them with the in-situ current acquisition and the following analysis & feedback system. Therefore, the researchers can automate lipid bilayer experiments using Bila-kit. 

## Currently applicable to:
- [x]  Detection & recovery from measurement failures (e.g. rupture or failure of lipid bilayers) without human supervision.
- [x]  Automatic measurement of electrical conductance of biological nanopores (e.g. alpha-hemolysin) to create a histogram.
- [x]  Real-time estimation of the concentration of chemical substances (e.g. beta-cyclodextrin) using corresponding membrane proteins (e.g. alpha-hemolysin) for real-time sensing applications.
- [x]  In-situ calculation of open probability of ion channels (e.g. big-pottasium channel) under their promotors/inhibitors (e.g. verapamil) to effciently investigate drug candidates. 

## Currently dealing with:
- [ ] Measurement of membrane proteins with low S/N ratio (i.e. with too small conductances).
- [ ] Introduction of object-oriented programming to the source codes (poor structuring for now).
- [ ] Improvement in algorithms for determining the state/number of membrane proteins.


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

[NOTE] The purpose to use API/DLLs is to acquire the digitized current in-situ and on-line, not after experiments. Therefore, if you find other manufacturer's API/DLLs more suitable or discover other way to acquire the digitized current in-situ and on-line, you can replace them with Tecella amplifier and API/DLLs in this paper. 

* Obtain a Tecella amplifier.
  * Bila-kit uses [PICO](http://www.tecella.com/pico.html).
  * Install necessary drivers by the manufacturer's instructions.

### Arduino and stepper motors
You can use your preferable microcomputers for driving peripheral devices (such as stepper motors). Just for example, we use Arduino MEGA. 

* Obtain Arduino MEGA.
  * Bila-kit uses Arduino Mega 2560 R3. 

## Download this repository
* Download this repository to your preferable working directory.


# Usage

## Setup the hardware connection
![The image](/Assets/capture.PNG)

## Setup the lipid bilayer environment
* Rotating device is on the paper

## Run the application
* Setup
* Acquire
* Stop

## Analysis
* already in the log/ folder.
* open CSV and create histogram.


# Modification
Bila-kit is open-source except some API/DLLs, so you can freely modify this application to meet your demands. Although not obligatory, we would really appreciate if you cite the following paper.

[HOGE]


