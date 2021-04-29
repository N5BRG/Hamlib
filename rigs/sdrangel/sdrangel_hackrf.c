/*
 *  Hamlib sdrangel_hackrf backend - main file
 *  Copyright (c) 2021 by Robert Stricklin N5BRG
 *
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  Credit is given to Volker Schroer as some of this code was taken from another package in the Hamlib colection
 */
/*
 *
 * For information about the SDRAngel_hackrfcontrols see
 * https://github.com/f4exb/sdrangel_hackrf/tree/master/swagger
 *
 */

#define MIC3 3

//#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>		/* String function definitions */
#include <unistd.h>		/* UNIX standard function definitions */
#include <fcntl.h>		/* File control definitions */
#include <errno.h>		/* Error number definitions */
#include <math.h>
#include <time.h>

#include "hamlib/rig.h"
#include "iofunc.h"
#include "misc.h"
#include "token.h"
#include "network.h"
#include "sdrangel_hackrf.h"


// struct sdrangel_hackrf_state *state;

int rx_position_hackrf = 0;
int tx_position_hackrf = 1;
extern freq_t vfoa;
extern freq_t vfob;

// CMD * RESET
int sdrangel_hackrf_reset(RIG *rig, reset_t reset) {
	int ret=0;
	freq_t freq=0.0;
	freq_t *p_freq=&freq;
	long rounded_freq=0;
	shortfreq_t kHz_portion=0;


	// reset can be 0=none  1=software reset 2=VFO reset 4=memory reset
	if(reset & 2) {
		zero_split_mode_status();               // Not Split=0   Split=1
		zero_current_TX_VFO();                  // VFO_A
		//rig->state.cache.split=0; // Force split vfo mode to locked VFOA with VFOB 
		//rig->state.vfo_list = (rig->state.vfo_list | 0x03);  // Force split vfo mode to locked VFOA with VFOB 
		ret=sdrangel_set_ptt(rig,1,0);
		//VFOA
		rig->state.cache.freqMainA = freq;
		//VFOB
		rig->state.cache.freqMainB = freq;

		// Current VFO in use
		rig->state.current_vfo = RIG_VFO_A;
		sdrangel_get_freq(rig, 0, p_freq);
		//Round off the frequency to the nearest kHz
		rounded_freq = ((long) (freq/1000)) * 1000;
		freq =(freq_t) rounded_freq;
		kHz_portion = (shortfreq_t) (freq - rounded_freq);
		set_freq_hackrf(rig, 0, freq);  // Update freq with rounded freq
		set_freq_hackrf(rig, 1, freq);
		vfoa=freq + (freq_t) kHz_portion;
		vfob=freq + (freq_t) kHz_portion;
		sdrangel_set_rit(rig, 0, kHz_portion);
		sdrangel_set_xit(rig, 1, kHz_portion);
	}
	rig_debug(RIG_DEBUG_TRACE, "%d %s:  Frequency Read = %lf\n", __LINE__, __func__, freq);
	if(reset & 8)
		ret = sdrangel_hackrf_init(rig);
	if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   ret=%d\n", __LINE__, __func__, ret );
		return -1;   // error condition
	}
	return RIG_OK;
}

// CMD (none)    INIT 
// Initilize all radio functions
int sdrangel_hackrf_init (RIG * rig) {
	int i=0;
	int ret;
	int buf_cnt=0;
	long rounded_freq=0;
	shortfreq_t kHz_portion=0;
	static char buf[HUGEBUFSIZE];
	char *p_buf=&buf[0];
	static char buf2[HUGEBUFSIZE];
	char *p_buf2=&buf2[0];
	char URL[100];
	char *p_URL=&URL[0];
	char parameter_name[25]=" ";
	char *p_parameter_name=&parameter_name[0];
	char value[25];
	char *p_value=&value[0];
	freq_t freq=0.0;
	char new_hunt_value[25]=" ";
	char *hunt_value=&new_hunt_value[0];
	char filename[100] = " ";
	char *p_filename = &filename[0];
	char line[200]=" ";
	char *p_line=&line[0];
	char object[20]=" ";
	char *p_object=&object[0];

	p_filename = getenv("HAMLIB_SDRANGEL_PATH");
	if(p_filename) {
		strcat(p_filename,"/CONFIGURE_HAMLIB_SDRANGEL_PLUTO");
		//printf("%s\n",p_filename);
		FILE *f1 = fopen(p_filename,"r");
		if(f1 == NULL) {
			printf("Unable to open file CONFIGURE_HAMLIB_SDRANGEL_PLUTO so defaults will be used.\n");
			rig_debug(RIG_DEBUG_ERR, "%d  %s: Failed to read file %s\n",__LINE__, __func__, p_filename );
		}
		else {
			while(fgets(p_line,200,f1) != NULL) {
				if(p_line[0] != '#') {
					sscanf(p_line,"%s %s",p_object,p_value);
					i=0;
					while (p_object[i] != '\0') {
						if (p_object[i] >= 'A' && p_object[i] <= 'Z') {
							p_object[i] = p_object[i] + 32;
						}
						i++;
					}
					if(strcmp(p_object, "rx_position_hackrf") == 0) 
							rx_position_hackrf = atoi(p_value);
					else if(strcmp(p_object, "tx_position_hackrf") == 0) 
							tx_position_hackrf = atoi(p_value);
					else if(strcmp(p_object, "relay_info") == 0) 
							rig->state.relay_info =(int) atoi(p_value);
					else if(strcmp(p_object, "audioinput") == 0) 
							rig->state.audioInput =(int) atoi(p_value);
					else
						printf("From the file CONFIGURE_HAMLIB_SDRANGEL_PLUTO  \"%s\" Has no effect.\n",p_object);
				}
			}
			fclose(f1);
		}
	}
	else
		printf("Enviornment varable can not be found.\n");

	rig_debug(RIG_DEBUG_ERR, "%d  %s: \nValues taken from configure file:\nrx_position_hackrf=%d\ntx_position_hackrf=%d\n\n",__LINE__,  __func__, rx_position_hackrf, tx_position_hackrf);

	zero_split_mode_status();		// Not Split=0   Split=1
	zero_current_TX_VFO();			// VFO_A

	// Print the version of this code. Update in sdrangle.h if needed.
	printf("SDRangel_hackrf Hamlab Interface Version %s\n",rig->caps->version);
	ret=sdrangel_open (rig);


	// Make sure we have a SDRAngel_hackrf active with Rx and Tx
	sprintf(p_buf,"http://127.0.0.1:8091/sdrangel/devicesets");
	ret=HTTP_action_GET(p_buf);
        if(ret != 0){
                printf("Make sure you have SDRAngel_hackrf setup and working! %d\n",ret);
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   ret=%d\n",__LINE__,  __func__, ret );
                return -1;      //Failed
        }
	buf_cnt = strlen (p_buf);
	strcpy (hunt_value, "devicesetcount");
	extract_value(hunt_value, buf_cnt, p_buf);
	if(atoi(hunt_value) < 2) {
		printf("Must have SDRAngel operating with both a Rx and Tx device setup on SDRAngel_hackrf with SSB in R0 and T0. Check this and try again.\n");
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   hunt_value=%s\n",__LINE__,  __func__, hunt_value );
		return -1;
	}

	// Make sure we have two channels 
	// Check for a min of 1 channel on device 0
        sprintf(p_buf,"http://127.0.0.1:8091/sdrangel/deviceset/0/channels/report");
        ret=HTTP_action_GET(p_buf);
        if(ret != 0){
                printf("Make sure you have SDRAngel_hackrf setup and working! %d\n",ret);
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
        }
        buf_cnt = strlen (p_buf);
        strcpy (hunt_value, "channelcount");
        extract_value(hunt_value, buf_cnt, p_buf);
        if(atoi(hunt_value) < 1) {
                printf("Must one channel of Demod/Mod active.\n");
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   hunt_value=%s\n", __LINE__, __func__, hunt_value );
                return -1;
        }
	// Check for a min of 1 channel on device 1
	sprintf(p_buf,"http://127.0.0.1:8091/sdrangel/deviceset/1/channels/report");
        ret=HTTP_action_GET(p_buf);
        if(ret != 0){
                printf("Make sure you have SDRAngel_hackrf setup and working! %d\n",ret);
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
        }
        buf_cnt = strlen (p_buf);
        strcpy (hunt_value, "channelcount");
        extract_value(hunt_value, buf_cnt, p_buf);
        if(atoi(hunt_value) < 1) {
                printf("Must have at least one channel of Demod/Mod active on each device.\n");
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   ret=%d\n", __LINE__, __func__, ret );
                return -1;
        }

	// Put Rig in receive State
	ret=sdrangel_set_ptt(rig,1,0);
	rig_debug(RIG_DEBUG_TRACE, "%d %s:  Partial Init  channel_count(hunt_value) = %s\n", __LINE__, __func__, hunt_value);
	if(ret != 0){
		printf("Problem initilizing SDRangel_hackrf radio interface with receive mode.\n");
		printf("Make sure SDRangel_hackrf radio is running on your computer.\n");
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   ret=%d\n", __LINE__, __func__, ret );
		return -1;
	}

	// Set fcPOS or Frequency Control Position to 1 in channel Rx

	sprintf(p_buf,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",rx_position_hackrf);
        ret=HTTP_action_GET(p_buf);
	strcpy(p_parameter_name,"fcPos");
	strcpy(p_value," 2");
	p_buf2=replace_value(p_buf, p_parameter_name, p_value);
	sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",rx_position_hackrf);
	ret=HTTP_action_PATCH(p_URL, p_buf2);
        free(p_buf2);
        rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);
        if(ret != 0) {
                rig_debug(RIG_DEBUG_ERR, "%d %s: p_buf:%s\n", __LINE__, __func__, p_buf);
                return -1;
        }

	// Start with RIT and XIT at zero
	ret=sdrangel_set_rit(rig, 0, 0);		// Zero the RIT
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set RIT failed! ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
        }
	ret=sdrangel_set_xit(rig, 1, 0);		// Zero the XIT
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set XIT failed! ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
        }

	// Get current frequency info from SDRAngel_hackrf VFOA and place it in VFOB and Hamlib cache
	ret=sdrangel_get_freq(rig, 0, &freq);  // Get RX VFO freq
	if(ret != RIG_OK){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Error Reading Rx VFO ret=%d\n", __LINE__, __func__, ret );
		return -1;
	}
	//Round off the frequency to the nearest kHz
	vfoa=freq;
	rounded_freq = ((long) (freq/1000)*1000);
	kHz_portion = (shortfreq_t) (freq - rounded_freq);
	freq =(freq_t) rounded_freq;
	ret=set_freq_hackrf(rig, 0, freq);  // Set rounded VFO Rx freq
	ret=set_freq_hackrf(rig, 1, freq);  // Set TX VFO freq with RX read
	if(ret != RIG_OK){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Error Reading Tx VFO ret=%d\n", __LINE__, __func__, ret );
		return -1;
	}
	// Start with RIT and XIT at zero or kHz_portion
	ret=sdrangel_set_rit(rig, 0, kHz_portion);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set RIT failed! ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
        }
	ret=sdrangel_set_xit(rig, 1, kHz_portion);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set XIT failed! ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
        }
	vfoa=freq + (freq_t) kHz_portion;
	vfob=freq + (freq_t) kHz_portion;

	// Set the initial sample rate
	strcpy(p_parameter_name,"devSampleRate");
	strcpy(p_value,"6500000");
	ret = sdrangel_set_device_Parameter(rig, 0, p_parameter_name, p_value);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set sample rate failed! ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
        }
	ret = sdrangel_set_device_Parameter(rig, 1, p_parameter_name, p_value);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set XIT failed! ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
        }
	// Set the initial decimation/interpulation rate
	strcpy(p_parameter_name,"log2Decim");
	strcpy(p_value,"4");
	ret = sdrangel_set_device_Parameter(rig, 0, p_parameter_name, p_value);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set sample rate failed! ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
        }
	strcpy(p_parameter_name,"log2Interp");
        strcpy(p_value,"4");
	ret = sdrangel_set_device_Parameter(rig, 1, p_parameter_name, p_value);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set XIT failed! ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
	}

	//VFOA
	rig->state.cache.freqMainA = freq;
	//VFOB
	rig->state.cache.freqMainB = freq;
	
	// Current VFO in use
	rig->state.current_vfo = RIG_VFO_A;
	rig_debug(RIG_DEBUG_TRACE, "%d %s: Completed Init  Frequency Read = %lf\n", __LINE__, __func__, freq);

	return RIG_OK;
}

// CMD I
int set_split_freq_hackrf(RIG * rig, vfo_t vfo, freq_t freq) {
        int ret=0;
	long rounded_freq=0;
        shortfreq_t kHz_portion=0;
	shortfreq_t delta_freq=0;
        split_mode_status = 1;
        //printf("VFO_B selection\n");
        current_TX_VFO = 1;    // VFO_B
	delta_freq = (shortfreq_t) (freq - (freq_t) (((long) (vfoa/1000))*1000));
	//delta_freq = (shortfreq_t) (freq-vfoa);
	vfob = freq;
        //printf("VFO_B selection\n");
        current_TX_VFO = 1;    // VFO_B
        rounded_freq = ((long) (freq/1000))*1000;
        kHz_portion = (shortfreq_t) (freq - rounded_freq);
        freq =(freq_t) rounded_freq;
	ret = sdrangel_set_xit(rig, 0, delta_freq);
        if(ret != RIG_OK) {
                rig_debug(RIG_DEBUG_ERR, "%d %s: delta_frequency=%ld\n", __LINE__, __func__, delta_freq);
                return -1;
        }
	ret=sdrangel_set_xit(rig, 1, kHz_portion);
        if(ret != 0){
                rig_debug(RIG_DEBUG_ERR, "%d %s: Error setting VFO_B XIT\n", __LINE__, __func__);
                return -1;      //Failed
        }
        // This is inserted to force Hamlib to switch to VFOB --  Takes too much time but not the only time issue.
        ret=sdrangel_set_split_vfo (rig, 536870912, 2, 2);
        rig_debug(RIG_DEBUG_TRACE, "%d %s: vfo=%d  freq=%lf\n", __LINE__, __func__,vfo,freq);
        return RIG_OK;
}

// CMD i
int get_split_freq_hackrf(RIG * rig, vfo_t vfo, freq_t *freq){
        int ret=0;
	shortfreq_t xit=0;
	shortfreq_t *p_xit=&xit;
	vfo_t this_vfo=0;
        vfo_t TxbitandVFOcurr = (3 << 29);
        switch (vfo & (RIG_VFO_A + RIG_VFO_B + RIG_VFO_MAIN + RIG_VFO_MAIN_A + RIG_VFO_MAIN_B + TxbitandVFOcurr)) {   //TX VFO has to be one of these
                case RIG_VFO_A:
                case RIG_VFO_MAIN:
                case RIG_VFO_MAIN_A:
                        this_vfo = 1;
                        break;

                case RIG_VFO_B:
                case RIG_VFO_MAIN_B:
                case (3 << 29):   // current VFO and bit to indicate the TX_VFO
                        this_vfo = 2;
                        break;

                default:
                        rig_debug(RIG_DEBUG_ERR, "%d %s: Error seting VFO_A\n", __LINE__, __func__);
                        return -1;      //Failed
        }
        ret = sdrangel_get_xit(rig, this_vfo, &xit);
        if(ret != 0){
                rig_debug(RIG_DEBUG_ERR, "%d %s: Error seting VFO_A\n", __LINE__, __func__);
                return -1;      //Failed
        }
	ret = sdrangel_get_xit(rig, 1, p_xit);
        *freq = *freq + *p_xit;
        rig_debug(RIG_DEBUG_TRACE, "%d %s: vfo=%d  freq=%lf\n", __LINE__, __func__,vfo,*freq);
	*freq = (freq_t) (vfoa + xit);
        return RIG_OK;
}

// CMD K
int set_split_freq_mode_hackrf(RIG *rig, vfo_t vfo, freq_t tx_freq, rmode_t mode, pbwidth_t width){
        int ret=0;
	shortfreq_t delta_freq = 0;
        split_mode_status = 1;          // Split mode
        current_TX_VFO = 1;             // Split mode VFO
        ret=sdrangel_set_mode (rig, current_TX_VFO, mode, width);
        if(ret != RIG_OK) {
                rig_debug(RIG_DEBUG_ERR, "%d %s: mode=%lu\n", __LINE__, __func__, mode);
                return -1;
        }
	delta_freq = (shortfreq_t) (tx_freq - vfoa);
	ret = sdrangel_set_xit(rig, 1, delta_freq);
        if(ret != RIG_OK) {
                rig_debug(RIG_DEBUG_ERR, "%d %s: delta_frequency=%ld\n", __LINE__, __func__, delta_freq);
                return -1;
        }
        ret= set_modAFInput(MIC3);
        if(ret != RIG_OK) {
                rig_debug(RIG_DEBUG_ERR, "%d %s: modAFInput=%d\n", __LINE__, __func__, MIC3);
                return -1;
        }
        return RIG_OK;
}

// CMD k
int get_split_freq_mode_hackrf(RIG *rig, vfo_t vfo, freq_t *freq, rmode_t *mode, pbwidth_t *width){
        int ret=0;
	//shortfreq_t xit = 0;
        ret = sdrangel_get_mode (rig, 2, mode, width);
        if(ret != RIG_OK) {
                rig_debug(RIG_DEBUG_ERR, "%d %s: mode=%ld\n", __LINE__, __func__, *mode);
                return -1;
        }
        ret=get_freq_hackrf(rig, 2,freq);
        if(ret != RIG_OK) {
                rig_debug(RIG_DEBUG_ERR, "%d %s: mode=%lf\n", __LINE__, __func__, *freq);
                return -1;
        }
        rig_debug(RIG_DEBUG_TRACE, "%d %s: split = %d\n", __LINE__, __func__, split_mode_status);
        return RIG_OK;
}

// CMD F
// Uses CURL to write to REST interface and set frequency.
int set_freq_hackrf(RIG * rig, vfo_t vfo, freq_t freq) {
        int ret=0;
	long rounded_freq=0;
	shortfreq_t kHz_portion=0;
        char *p_buf = (char *) calloc(HUGEBUFSIZE, sizeof(char));
        char buffer2[HUGEBUFSIZE]=" ";
        char *p_buf2 = &buffer2[0];
        // Array only needs to be 798 but 900 used for safty. Could reduce if memory issues.
        char URL[100];
        char *p_URL = &URL[0];
        char name[25]=" ";
        char *p_name=&name[0];
        char value[25]=" ";
        char *p_value=&value[0];
        vfoa = freq;
	rounded_freq = ((long) (freq/1000)*1000);
	kHz_portion = (shortfreq_t) (freq - rounded_freq);
	freq =(freq_t) rounded_freq;
        sprintf(p_value," %f",freq);
        if(split_mode_status == 0 || ((vfo & 1) != 1)){
		vfoa = freq;
                sprintf(p_buf,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",0);
                ret=HTTP_action_GET(p_buf);
                if(ret != 0){
                        printf("Get mode  Failed! %d\n",ret);
                        rig_debug(RIG_DEBUG_ERR,"%d %s: buffer:\n\%s\n", __LINE__, __func__, p_buf);
                        return -1;      //Failed
                }
                strcpy(p_name,"centerFrequency");
                p_buf2=replace_value(p_buf, p_name, p_value);
                sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",0);
                ret =  HTTP_action_PATCH(p_URL, p_buf2);
                if(ret != 0){
                        free(p_buf);
                        free(p_buf2);
                        rig_debug(RIG_DEBUG_ERR, "%d %s: p_URL: %s\n", __LINE__, __func__, p_URL);
                        return -1;      //Failed
                }
		free(p_buf);
		free(p_buf2);
		ret=sdrangel_set_rit(rig, 0, kHz_portion);
		if(ret != 0){
			rig_debug(RIG_DEBUG_ERR, "%d %s: Set RIT failed! ret=%d\n", __LINE__, __func__, ret );
			return -1;      //Failed
		}
		ret=sdrangel_set_xit(rig, 0, kHz_portion);
		if(ret != 0){
			rig_debug(RIG_DEBUG_ERR, "%d %s: Set RIT failed! ret=%d\n", __LINE__, __func__, ret );
			return -1;      //Failed
		}
        }
        if(split_mode_status == 1 || ((vfo & 1) == 1)){
                vfob = freq;
		ret = sdrangel_set_xit(rig, 1, (vfoa-vfob));
                if(ret != 0){
                        rig_debug(RIG_DEBUG_ERR,"%d %s: vfoa-vfob=%lf\n", __LINE__, __func__, (vfoa-vfob));
                        return -1;      //Failed
                }
        }
        rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);
        return RIG_OK;
}

// CMD f
int get_freq_hackrf(RIG * rig, vfo_t vfo, freq_t * freq) {
        int ret=0;
        char buf[MIDBUFSIZE] = " ";
        char *buffer = &buf[0];
        char value[20];
        char *hunt_value = &value[0];
        int buf_cnt;
	int current_vfo=(vfo & 3)-1;
printf("current_vfo=%d\n",current_vfo);
        if(current_vfo < 0) current_vfo = 0;
        shortfreq_t rit=0;
        shortfreq_t *p_rit=&rit;
        shortfreq_t xit=0;
        shortfreq_t *p_xit=&xit;
	sprintf(buffer,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",current_vfo);
        ret=HTTP_action_GET(buffer);
        if(ret != 0){
                rig_debug(RIG_DEBUG_ERR, "%d %s: buffer:\n=%s\n", __LINE__, __func__, buffer );
                return -1;      //Failed
        }
        buf_cnt = strlen (buffer);
        strcpy (value, "centerFrequency");
        extract_value (hunt_value, buf_cnt, buffer);
        sscanf(hunt_value,"%lf",freq);
	//Adjust for rit or xit
	//if(vfo == RIG_VFO_CURR && split_mode_status!=1){     // RIG_VFO_N(29) or 536870912 Decimal  2^29 binary
	if(current_vfo){     // RIG_VFO_N(29) or 536870912 Decimal  2^29 binary
		ret=sdrangel_get_xit(rig,0,p_xit);
		if(*p_xit < -99990000 || *p_xit > 9999000){
			*p_xit = 0;
			printf("XIT currently set to a bad value and the reading will be ignored!\n");
		}
	}
	else{
		ret=sdrangel_get_rit(rig,1,p_rit);
		if(*p_rit < -9999000 || *p_rit > 9999000){
			*p_rit = 0;
			printf("RIT currently set to a bad value and the reading will be ignored!\n");
		}
	}
	// May need to add rounding of freq here becasue the Hz value can not be seen on display.
	*freq = *freq + (freq_t) (*p_rit + *p_xit);
        rig_debug(RIG_DEBUG_TRACE, "%d %s: frequency = %lf\n", __LINE__, __func__, *freq);
        if (*freq > 0)
                return RIG_OK;
        else {
                rig_debug(RIG_DEBUG_ERR, "%d %s: hunt_value=%s\n", __LINE__, __func__, hunt_value );
                return -1;      // Error condition
        }
}

