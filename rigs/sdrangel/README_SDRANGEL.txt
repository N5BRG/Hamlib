Installation of Hamlib with SDRAngel support:

Follow the same installation procedures outlined for Hamlib using this forked package. The build is done the same way.  The details can be found at: https://github.com/Hamlib/Hamlib/releases.

Before building Hamlib you must add one additional library dependency, the CURL package. You can do this by downloading CURL from:

https://curl.se/libcurl/ 

Follow the instructions provided with CURL for your computer.

After installing CURL then go through the Hamlib build process. Running the following commands from the Hamlib folder will build and install the program.

./bootstrap
./configuration
make
sudo make install
sudo ldconfig



Comments:

This is a collection of files used to support SDRAngel with Hamlib.

The software was developed using Ubuntu 20.04 and Hamlib 4.2~git. 

All work was done with C programing language in hopes the software will be more portable to other operating systems. There may be a need to convert to other languages in some areas to speed up the interface. This is work in progress.

There is an issue with speed. Your ability to work with this program will be impacted by the capability of the computer you use. The code was developed and tested using an Intel i7. I wish it were faster. If you are using a low-cost ARM based computer, you may not be happy with the performance. A key goal was to make this work with WSJTX. At this time this package will not work due to speed limitations. More work is being done to try and correct these issues.

Communications with SDRAngel is done using a REST interface which is included in SDRAngel. This operates like a web page interface. You send SDRAngel a HTTP type request called a GET, POST, PATCH, or DELETE and wait for a reply. Each request requires moving approximately 1000 bytes of data and sometimes two directions. This is why the process is slow.  Typically, all communication takes place on the computer SDRAngel is running on. It is possible to run the Hamlib interface on another computer on the internet, but this set is more complex and not intended for the typical user.

The REST interface used in this package requires a CURL library package. Using CURL makes the interface more robust. The instructions for obtaining CURL are included below.

 There are many commands available in Hamlib. Only a portion of these commands have been implements at this time. The commands needed to operate a radio and make contacts including the WSJTX mode are currently included. More commands may be added later. The RTL-sdr type receiver only devices have a more limited command interface and may only support Frequency and Mode control.

The advantage of SDRAngel is the many types of SDR radios supported and the modes which may be used on these radios. More radios and modes are being added as well. The SDRAngel package has been continuously improved mostly due to the efforts of Edouard Griffiths, F4EXB. Others are supporting the effort as well. You can find the details of the SDRAngel package at: https://github.com/f4exb/sdrangel. The software can be installed on Linux, Windows, or  Apple Mac. The development was done with Linux and additional steps will be required to support Windows. These have not been addressed but hopfully they will be in the near future.

This Hamlib module has also been tested with CQRLOG using Linux and this Hamlib interface works with the CQRLOG package as well. This logging program was originally developed by Peter, OK2CQR and Martin, OK1RR and continues to enjoy much support from others. This package is located at cqrlog.com.

One nice feather of CQRLOG is the ability to open a widow of active DX stations using a DX cluster. Then you can double click a station in this list and the required frequency and mode will be sent via Hamlib to your rig. I have tested this with this Hamlib package and seen it working. More testing needs to be done though.

Finally, WSJTX v2.4.0-rc3 functions with this package. Getting all the details to work with this package took some time and needs more testing. Timing is key in some of the modes supported by WSJTX and you may find they cannot be used. Again, more testing with some feedback will be needed to learn the true limitations.

My testing has been with SDRAngel using a Analog Devices Pluto, RTL-sdr  and Hackrf One.


Relay Operation:

To help with T/R switching the code is included to operate a USB relay board sold under the name SainSmart Relay. Search for “SainSmart USB 4 Channel Relay Automation (5V)SainSmart USB 4 Channel Relay Automation (5V)” on Amazon to view this relay. Other relays using a FTDI chip will work but code modifications beyond the config file may be needed.

The FTDI chip controls one of four on board relays. You can select the particular relay being used in the config file.

Including this capability required the addition of an FTDI library and other files for support.


Debugging Problems:

If you append a -vvv to the startup command you will get a printout on your terminal of any errors that occur during processing. Each error line in the message will include a number which is roughly the line of code with the error, the function name, and in some cases a variable value to help understand the root cause of the issue. If you cannot find the issue yourself then reporting this information using the GITHUB issue logging system may help resolve the problem. Be sure to thoroughly check your setup and consider any changes you have made. The typical issue reported is: This was all working fine last night when I turned it off and went to bed. Then this morning I turned it on and things are just not working. Later, it is determined some change was made in the interim but the operator. A lot of things have to be setup correctly, there radio, the computer, Hamlib, logging program, and possibly WSJTX.

If you enter a -vvvvv then you will get a message for every executed function with information similar to the -vvv option above. This may also help by tracking through the execution of the program and telling you how far the program is running before it has some issue or crashes.

Config File:

In order to make this code more versatile a config file is used. The name of the configure file is CONFIGURE_HAMLIB_SDRANGEL_(RIGTYPE) This can be a help and it may be a hindrance because the file is more which may go wrong for the user who does not understand. To include a configure file you must define a location for the file which will work across all types of operating systems. The approach here is to define an environment variable which is the PATH to the location of the config file. If this variable is present on your computer Hamlib will look for the config file using this PATH. If config is not found the program will look for the file in the “./” directory. If it does not find the config there Hamlib for SDRAngel will use default values. You can place the config file in any folder you prefer but be sure to setup the environment variable. Please review the config file and make any needed changes in the values provided for your equipment. Initially this is only relays but hopefully more options will be added later.

In Linux add the environment value HAMLIB_SDRANGEL_PATH to your .bashrc file by appending line like the one shown to the end of the file. Then log off and log back on to your computer or reboot.

Edit the file in your root directory by adding one line to bottom:    /home/user/.bashrc
export HAMLIB_SDRANGEL_PATH = "/home/your_user_name/Hamlib"



General Comment:

In many cases Hamlib is used with a physical radio manufactured by a vendor. Once the radio is in production the interface is fixed until upgrades are made. These upgrades to the radio firmware are rare. In this case the radio is in Software and continuing to evolve, mostly by adding more capability.  SDRAngel has been available for many years now though and the package is fairly stable.

The Hamlib interface is sensitive to small changes. Computers and programs have to have everything exact. If you have problems carefully think through any changes which have occurred in your setup. If you upgrade a software package you will need to recheck all the functions you typically use or at least be on alert for issues.


Operation:
After building Hamlib you should be able to run the program like this:

First run SDRAngel and create a “R0” receiver for your SDR and a “T1” transmitter. Apply a SSB demodulator and a modulator channel to each of these respectively.

Now start Hamlib in manual mode:

rigctl -m34001        (34001=Pluto   34002=HackRF   34003=RTL-SDR)

rigctl starts Hamlib and the -m34001 tells Hamlib to use the interface for SDRAngel.

After starting you can start using the available commands. Type a ? to see a listing of possible commands. Not all are implemented but the most important ones are there.

You can start with a ‘f’ and read the frequency of “R0”. 

The next important command to try is PTT for transceiver. This will cause your radio to start to transmit so be sure you are OK with this. Type in the command T 1 (Capital T space 1) will cause the SDRAngel to switch focus to T1 and activate the transmitter. This is the PTT ON mode. T 0 (Capital T space Zero) puts you back in receive state, R0 in focus and active T1 idle.

In addition to the basic Hamlib commands available in Hamlib these interface rig commands are available for SDRAngel at this time:

SET		GET
F		f		Frequency
J		j		RIT
Z		z		XIT
T		t		PTT
K		k		Split, Frequency, Mode, Bandwidth
V		v		VFO
S		s		Split VFO
I		I		Tx Split Frequency
X		x		Split Tx Operation
M		m		Modulation Mode, Bandwidth 
*				Reset	


More information on the commands:

*** Frequency
	This is working on the R0 receive frequency. This has been assigned to what Hamlib knows as VFOA.

*** RIT
	Use this command to adjust the receive frequency. The RIT is set to zero at startup and at reset. The RIT is also zeroed after a new frequency is set.

*** XIT
	Use this command to adjust the transmit frequency. The XIT is set to zero at startup and at reset The RIT is also set to zero when a new frequency is set. WSJTX uses XIT to make minor adjustments to the Tx frequency.

	Actual commands:
		Z – Reads current XIT setting in hertz.
		Z xxxxxx – Sets the current XIT to you value xxxxx in hertz. 9
*** PTT
	PTT will place in transmit mode. The interface to SDRAngel is the REST interface which is like passing data using HTTP commands. This interface is a bit slow, and three commands are needed to switch to transmit. There is some variability in the length of time this can take. Much work was done to try to speed this up but in the end the code was written to be more accurate and less concerned with speed.

*** VFO
	The VFO settings in Hamlib are challenging. In the case of the SDRAngel we essentially have VFOA for R0 and a VFOB for Tx. The Hamlib interface provided here locks these together until you place the radio in SPLIT operation. The Tx VFOB is set to the same frequency as the Rx VFOA at startup. SPLIT is used by WSJTX. If you request info on the current VFO Hamlib may respond with currVFO. This is typically VFOA (R0). If you are in SPLIT then currVFO is Tx VFOB. 

*** Split VFO
	Tells Hamlib you are operating with a split VFO and both Tx and Rx VFO will need to be maintained.

Actual Commands:
	s - reads current Split condition and TX VFO 	0=Not Split TX VFO none means lock to VFOA.
	S 1  VFOB    Sets split mode and TX VFO to VFOB
	S 2  VFOA	Sets split mode to not split and TX VFO to none (VFOA).


*** Tx Split Frequency
	This command sets the frequency in Tx VFOB and reads the current frequency. When a new frequency is placed in VFOB the XIT is also set to zero. This is because the frequency passed is in hertz and that covers both the device setting in SDRAngel and the XIT.

*** Split Tx Operation
	This tells the Hamlib interface we will be using both Rx and Tx VFO. The mode is also included here to support cases where a different mode is used for Tx.

*** Modulation Mode
	This sets the mode to be used for modulation and demodulation. When you use this command both Rx and Tx will be set to the same mode.

*** Reset	
	Puts SDRAngel in the same state as a power up. Tx VFO will be set to match the current Rx VFO and both RIT and XIT will be set to zero. SPLIT will be set to NO	

The normal way to use Hamlib is to run it  in the background and interface with other programs like WSJTX or CQRLOG type the command:

rigctld -m34001

You can actually start a background version of Hamlib and then run a second instance in the foreground with rigctl if you want to monitor things manually.

This should be enough to get you started. SDRAngel and Hamlib have more capability and in the future more commands may be added to take advantage of this.


For developers:

There is a script file included here called 'n'. To rebuild the projcect starting from the sdrangel folder type ./n enter. The script will take you back to ~/Hamlib rebuild the project and then put you back in the sdrangel folder.

There is a lot of work left to do with this but I wanted to get the posting out to allow others to look at and coment on the code. Also hope to reserve the #34 rig type in Hamlib.


Bob Stricklin   N5BRG

Document last changed March 23 2021   Rev   1.0
Please report any spelling or grammar issues with this document. Feel free to submit a corrected version. If you feel the wording is not clear or needs to be expanded, please submit this as well. Your help will be appreciated as many people will continue to work with these instructions after you hopefully.
