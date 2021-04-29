
/*
 *  Hamlib sdrangel_rtlsdr backend - main file
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
 * For information about the SDRAngel_plutocontrols see
 * https://github.com/f4exb/sdrangel_rtlsdr/tree/master/swagger
 *
 */

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
#include "sdrangel_rtlsdr.h"


// struct sdrangel_rtlsdr_state *state;

int rx_position_rtlsdr = 0;
int tx_position_rtlsdr = 1;

// CMD * RESET
int sdrangel_rtlsdr_reset(RIG *rig, reset_t reset) {
	int ret=0;
	freq_t freq=0.0;

	// reset can be 0=none  1=software reset 2=VFO reset 4=memory reset
	if(reset & 2) {
		rig->state.cache.freqMainA = freq;

		// Current VFO in use
		rig->state.current_vfo = RIG_VFO_A;
		sdrangel_set_rit(rig, 0, 0);
	}
	rig_debug(RIG_DEBUG_TRACE, "%d %s:  Frequency Read = %lf\n", __LINE__, __func__, freq);
	if(reset & 8)
		ret = sdrangel_rtlsdr_init(rig);
	if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   ret=%d\n", __LINE__, __func__, ret );
		return -1;   // error condition
	}
	return RIG_OK;
}

// CMD (none)    INIT 
// Initilize all radio functions
int sdrangel_rtlsdr_init (RIG * rig) {
	int i=0;
	int ret;
	int buf_cnt=0;
	static char buf[HUGEBUFSIZE];
	char *p_buf=&buf[0];
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
					if(strcmp(p_object, "rx_position_rtlsdr") == 0) 
							rx_position_rtlsdr = atoi(p_value);
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

	rig_debug(RIG_DEBUG_ERR, "%d  %s: \nValues taken from configure file:\nrx_position_rtlsdr=%d\ntx_position_rtlsdr=%d\n\n",__LINE__,  __func__, rx_position_rtlsdr, tx_position_rtlsdr);

	// Print the version of this code. Update in sdrangle.h if needed.
	printf("SDRangel_pluto Hamlab Interface Version %s\n",rig->caps->version);
	ret=sdrangel_open (rig);


	// Make sure we have a SDRAngel_pluto active with Rx 
	sprintf(p_buf,"http://127.0.0.1:8091/sdrangel/devicesets");
	ret=HTTP_action_GET(p_buf);
        if(ret != 0){
                printf("Make sure you have SDRAngel_pluto setup and working! %d\n",ret);
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   ret=%d\n",__LINE__,  __func__, ret );
                return -1;      //Failed
        }
	buf_cnt = strlen (p_buf);
	strcpy (hunt_value, "devicesetcount");
	extract_value(hunt_value, buf_cnt, p_buf);
	if(atoi(hunt_value) < 1) {
		printf("Must have SDRAngel operating with Rx device setup on SDRAngel_pluto with SSB in R0. Check this and try again.\n");
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   hunt_value=%s\n",__LINE__,  __func__, hunt_value );
		return -1;
	}

	// Put Rig in receive State
	sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/run",rx_position_rtlsdr);
        strcpy(p_buf,"");
        ret =  HTTP_action_POST(p_URL, p_buf);
        if(ret != 0) {
                rig_debug(RIG_DEBUG_ERR, "%d %s: \n", __LINE__, __func__);
                return -1;      //Failed
        }
        sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/focus",rx_position_rtlsdr);
        strcpy(p_buf,"");
        ret=HTTP_action_PATCH(p_URL,p_buf);
        rig_debug(RIG_DEBUG_TRACE, "%d %s: url = %s\n", __LINE__, __func__, URL);
        if(ret != 0) {
                rig_debug(RIG_DEBUG_ERR, "%d %s: \n", __LINE__, __func__);
                return -1;      //Failed
        }

	rig_debug(RIG_DEBUG_TRACE, "%d %s:  Partial Init  channel_count(hunt_value) = %s\n", __LINE__, __func__, hunt_value);
	if(ret != 0){
		printf("Problem initilizing SDRangel_pluto radio interface with receive mode.\n");
		printf("Make sure SDRangel_pluto radio is running on your computer.\n");
		rig_debug(RIG_DEBUG_ERR, "%d %s: Initialazation Function Failed!   ret=%d\n", __LINE__, __func__, ret );
		return -1;
	}

	// Start with RIT 
	ret=sdrangel_set_rit(rig, 0, 0);		// Zero the RIT
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set RIT failed! ret=%d\n", __LINE__, __func__, ret );
                return -1;      //Failed
        }

	// Set the initial sample rate
	strcpy(p_parameter_name,"devSampleRate");
	strcpy(p_value,"6500000");
	ret = sdrangel_set_device_Parameter(rig, 0, p_parameter_name, p_value);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set sample rate failed! ret=%d\n", __LINE__, __func__, ret );
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

	//VFOA
	rig->state.cache.freqMainA = freq;
	
	// Current VFO in use
	rig->state.current_vfo = RIG_VFO_A;
	rig_debug(RIG_DEBUG_TRACE, "%d %s: Completed Init  Frequency Read = %lf\n", __LINE__, __func__, freq);

	return RIG_OK;
}

