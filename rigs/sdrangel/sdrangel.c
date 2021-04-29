/*
 *  Hamlib SDRAngel backend - main file
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <unistd.h>             /* UNIX standard function definitions */
#include <fcntl.h>              /* File control definitions */
#include <errno.h>              /* Error number definitions */
#include <math.h>
#include <time.h>

#include "hamlib/rig.h"
#include "iofunc.h"
#include "misc.h"
#include "token.h"
#include "network.h"

#include "register.h"

#include "sdrangel.h"

#define RIG_OP_XCHG 0;

// struct sdrangel_state *state;
vfo_t sdrangel_tx_vfo=0;
int split_mode_status=0;        // Not Split=0   Split=1
vfo_t current_TX_VFO=0;         // VFO_A=0    VFO_B=1

int rx_mode = 4;
int rx_freq = 14417400;    // May need to move this up above 400 MHz to support Pluto's that have not been expanded in Frequency
int rx_radio = 1;
int tx_mode = 4;
int tx_freq = 14417400;
int tx_radio = 1;
freq_t vfoa = 0;
freq_t vfob = 0;

/*
 * initrigs_sdrangel is called by rig_backend_load
 */
DECLARE_INITRIG_BACKEND(sdrangel)
{
    rig_debug(RIG_DEBUG_VERBOSE, "%s: _init called\n", __func__);

    rig_register(&sdrangel_pluto_caps);
    rig_register(&sdrangel_hackrf_caps);
    rig_register(&sdrangel_rtlsdr_caps);

    return RIG_OK;
}

/*
 * initrots_sdrangel is called by rot_backend_load      DO NOT SEEM TO NEED THIS
*/
/*
DECLARE_INITROT_BACKEND(sdrangel)
{
    rig_debug(RIG_DEBUG_VERBOSE, "%s: _init called\n", __func__);

    rot_register(&pcrotor_caps);

    return RIG_OK;
}

*/

void zero_split_mode_status() {
	split_mode_status = 0;
}		              

void zero_current_TX_VFO() {
	current_TX_VFO = 0;
}		               

// Support Function
// Find the value of interest in a REST buffer and return with current value in val_buf.
void extract_value(char *hunt_value, int buf_cnt, char *buffer) {
        char val_buf[25];
        int i = 0;
        int j = 0;
        int v_count = strlen (hunt_value) + 3;
        while (buffer[i] != 0) {
                if ((strncmp(buffer + i, hunt_value, strlen (hunt_value)) == 0)) {
                        j = 0;
                        while ((j < 20) && (buffer[i + j + v_count]) != ',') {
                                val_buf[j] = buffer[i + j + strlen (hunt_value) + 3];
                                j++;
                        }
                }
                if (j > 0)
                        break;
                i++;
        }
        val_buf[j] = '\0';
        strcpy (hunt_value, val_buf);
        rig_debug(RIG_DEBUG_TRACE, "%d %s: hunt_value = %s\n", __LINE__, __func__, hunt_value);
        return;
}

// Support Function
// Open the socket to SDRAngel_pluto for communications
int sdrangel_open (RIG * rig) {
	hamlib_port_t *p = &rig->state.rigport;
	int sdrangel_default_port = 8091;
	int ret = 0;
	ret = network_open (p, sdrangel_default_port);
	rig_debug(RIG_DEBUG_TRACE, "%d %s: ret error count = %d\n", __LINE__, __func__, ret);
	if (ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: ret=%d\n", __LINE__, __func__, ret );
		return -1;
	}
	return RIG_OK;
}

// CMD None    CLOSE FUNCTION
int sdrangel_close(RIG *rig) {
	hamlib_port_t *p = &rig->state.rigport;
        int fd = p->fd;
	int ret = 0;
	int len = 0;
	int nbytes = 0;
	char ptt_off_buf1[193]="DELETE /sdrangel/deviceset/1/device/run HTTP/1.1\r\n\r\n";

	//free(state);  // free up memory we were using for cache

	ret = network_close (p);
	if (ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: ret=%d\n", __LINE__, __func__, ret );
		return -1;
	}
	// Leave SDRAngel_pluto in a receive state
	ret=sdrangel_set_ptt(rig,1,0);
	if (ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: ret=%d\n", __LINE__, __func__, ret );
		return -1;
	}

	ret=sdrangel_set_ptt(rig,1,0);
	if (ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: ret=%d\n", __LINE__, __func__, ret );
		return -1;
	}
	len=strlen(&ptt_off_buf1[0]);
	nbytes = write (fd, &ptt_off_buf1[0],len);
	rig_debug(RIG_DEBUG_TRACE, "%d %s: nbytes count = %d\n", __LINE__, __func__, nbytes);
	if (nbytes < 1) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: nbytes=%d\n", __LINE__, __func__, nbytes );
		return -1;
	}
	printf("CLOSING SDRAngel_pluto\n");
	return RIG_OK;
}

int sdrangel_cleanup(RIG *rig) {
	printf("CLEANUP SDRAngel_pluto computer space and set SDRAngel_pluto to receive.\n");
	return RIG_OK;
}

// CMD (Device_Parameter_Change)
// Uses CURL to write to REST interface and change a device parameter 
int sdrangel_set_device_Parameter(RIG * rig, vfo_t vfo, char *p_parameter_name, char *p_value) {
        int ret=0;
        char *p_buf = (char *) calloc(HUGEBUFSIZE, sizeof(char));
        char buffer2[HUGEBUFSIZE]=" ";
        char *p_buf2 = &buffer2[0];
        // Array only needs to be 798 but 900 used for safty. Could reduce if memory issues.
        char URL[100];
        char *p_URL = &URL[0];

        if(!(vfo & 1)){
                sprintf(p_buf,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",0);
                ret=HTTP_action_GET(p_buf);
                if(ret != 0){
                        printf("Get mode  Failed! %d\n",ret);
                        rig_debug(RIG_DEBUG_ERR,"%d %s: buffer:\n\%s\n", __LINE__, __func__, p_buf);
                        return -1;      //Failed
                }
                p_buf2=replace_value(p_buf, p_parameter_name,  p_value);
                sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",0);
                ret =  HTTP_action_PATCH(p_URL, p_buf2);
                if(ret != 0){
                        free(p_buf);
                        free(p_buf2);
                        rig_debug(RIG_DEBUG_ERR, "%d %s: p_URL: %s\n", __LINE__, __func__, p_URL);
                        return -1;      //Failed
                }
        }
        if(vfo & 1) {
                sprintf(p_buf,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",1);
		ret=HTTP_action_GET(p_buf);
                if(ret != 0){
                        printf("Get mode  Failed! %d\n",ret);
                        rig_debug(RIG_DEBUG_ERR,"%d  %s: buffer:\n\%s\n", __LINE__, __func__, p_buf);
                        return -1;      //Failed
                }
                p_buf2=replace_value(p_buf, p_parameter_name, p_value);
                sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",1);
                ret =  HTTP_action_PATCH(p_URL, p_buf2);
                if(ret != 0){
                        free(p_buf);
                        free(p_buf2);
                        rig_debug(RIG_DEBUG_ERR, "%d %s: p_URL: %s\n", __LINE__, __func__, p_URL);
                        return -1;      //Failed
                }
        }
        free(p_buf);
        free(p_buf2);
        rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);
        return RIG_OK;
}

// CMD M
// TODO Add more modes of modulation
int sdrangel_set_mode (RIG * rig, vfo_t vfo, rmode_t new_mode, pbwidth_t width) {
        char buf[READBUFSIZE]=" ";
        char *buffer=&buf[0];
        char buf1[READBUFSIZE]=" ";
        char *buffer1=&buf1[0];
	char URL[100];
	char *p_URL=&URL[0];
        char value[20]=" ";
	char *p_value=&value[0];
	char name[25]=" ";
	char *p_name=&name[0];
        char value1[20]=" ";
	char *p_value1=&value1[0];
	char name1[25]=" ";
	char *p_name1=&name1[0];
	rmode_t current_mode=0;
	pbwidth_t current_width=0;
	vfo_t current_vfo=0;
	int ret=1;

	char ssb_mode_demod[676] = "\{  \"SSBDemodSettings\": \{    \"agc\": 0,    \"agcClamping\": 0,    \"agcPowerThreshold\": -100,    \"agcThresholdGate\": 4,    \"agcTimeLog2\": 7,    \"audioBinaural\": 0,    \"audioDeviceName\":    \"System default device\",    \"audioFlipChannels\": 0,    \"audioMute\": 0,    \"dsb\": 0,    \"inputFrequencyOffset\": 0,    \"lowCutoff\": 300,    \"reverseAPIAddress\": \"127.0.0.1\",    \"reverseAPIChannelIndex\": 0,    \"reverseAPIDeviceIndex\": 0,    \"reverseAPIPort\": 8888,    \"rfBandwidth\": 3000,    \"rgbColor\": -16711936,    \"spanLog2\": 3,    \"streamIndex\": 0,    \"title\": \"SSB Demodulator\",    \"useReverseAPI\": 0,    \"volume\": 1   },   \"channelType\": \"SSBDemod\",   \"direction\": 0} ";
//printf("ssb demod %ld\n",strlen(ssb_mode_demod)+20);

	char ssb_mode_mod[912] = "\{ \"SSBModSettings\": \{ \"agc\": 0,     \"audioBinaural\": 0,     \"audioDeviceName\": \"System default device\",     \"audioFlipChannels\": 0,     \"audioMute\": 0,     \"bandwidth\": 2333,     \"cmpPreGainDB\": -10,     \"cmpThresholdDB\": -60,     \"cwKeyer\": \{ \"dashKey\": 45,       \"dashKeyModifiers\": 0,       \"dotKey\": 46,       \"dotKeyModifiers\": 0,       \"keyboardIambic\": 1,       \"loop\": 0,       \"mode\": 0,       \"sampleRate\": 48000,       \"wpm\": 13 },     \"dsb\": 0,     \"inputFrequencyOffset\": 0,     \"lowCutoff\": 300,     \"modAFInput\": 0,     \"playLoop\": 0,     \"reverseAPIAddress\": \"127.0.0.1\",     \"reverseAPIChannelIndex\": 0,     \"reverseAPIDeviceIndex\": 0,     \"reverseAPIPort\": 8888,     \"rgbColor\": -16711936,     \"spanLog2\": 3,     \"title\": \"SSB Modulator\",     \"toneFrequency\": 1000,     \"usb\": 1,     \"useReverseAPI\": 0,     \"volumeFactor\": 1 },   \"channelType\": \"SSBMod\",   \"direction\": 1 }";
//printf("ssb mod %ld\n",strlen(ssb_mode_mod)+20);

	char  am_mode_demod[555] = "\{\"channelType\": \"AMDemod\",  \"direction\": 0,  \"originatorDeviceSetIndex\": 0,  \"originatorChannelIndex\": 0,  \"AMDemodSettings\": {    \"inputFrequencyOffset\": 0,    \"rfBandwidth\": 0,    \"squelch\": 0,    \"volume\": 0,    \"audioMute\": 0,    \"bandpassEnable\": 0,    \"rgbColor\": -256,    \"title\": \"string\",    \"audioDeviceName\": \"string\",    \"pll\": 0,    \"syncAMOperation\": 0,    \"streamIndex\": 0,    \"useReverseAPI\": 0,    \"reverseAPIAddress\": \"string\",    \"reverseAPIPort\": 0,    \"reverseAPIDeviceIndex\": 0,    \"reverseAPIChannelIndex\": 0  }}";
//printf("am demod %ld\n",strlen(am_mode_demod+20));

	char am_mode_mod[746] = "\{  \"AMModSettings\": \{    \"audioDeviceName\": \"System default device\",    \"channelMute\": 0,    \"cwKeyer\": \{      \"dashKey\": 45,      \"dashKeyModifiers\": 0,      \"dotKey\": 46,      \"dotKeyModifiers\": 0,      \"keyboardIambic\": 1,      \"loop\": 0,      \"mode\": 0,      \"sampleRate\": 48000,      \"wpm\": 13    },    \"inputFrequencyOffset\": 0,    \"modAFInput\": 0,    \"modFactor\": 0.20000000298023224,    \"playLoop\": 0,    \"reverseAPIAddress\": \"127.0.0.1\",    \"reverseAPIChannelIndex\": 0,    \"reverseAPIDeviceIndex\": 0,    \"reverseAPIPort\": 8888,    \"rfBandwidth\": 12500,    \"rgbColor\": -256,    \"title\": \"AM Modulator\",    \"toneFrequency\": 1000,    \"useReverseAPI\": 0,    \"volumeFactor\": 1  },  \"channelType\": \"AMMod\",  \"direction\": 1}";
//printf("am mod %ld\n",strlen(am_mode_mod)+20);

        ret=sdrangel_get_mode(rig, current_vfo , &current_mode, &current_width);
        if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: new_mode=%ld\n", __LINE__, __func__, new_mode );
		return -1;
	}

	switch (new_mode)
	{
		case RIG_MODE_AM:
			strcpy(buffer,am_mode_demod);
			strcpy(buffer1,am_mode_mod);
			strcpy(p_name,"id");
			strcpy(p_name1,"title");
			strcpy(p_value,"\"AMDemod\"");
			strcpy(p_value1,"\"AM Demodulation\"");
			buffer=replace_value2(buffer, p_name, p_name1, p_value, p_value1);
			strcpy(p_value,"\"AMMod\"");
			strcpy(p_value1,"\"AM Modulator\"");
			buffer1=replace_value2(buffer1, p_name, p_name1, p_value, p_value1);
			if(width != -1){
				if(width == 0) width = 3000;
				strcpy(p_name,"rfBandwidth");
				sprintf(p_value," %ld",width);
				buffer=replace_value(buffer, p_name, p_value);
				strcpy(p_name,"bandwidth");
				buffer1=replace_value(buffer1, p_name, p_value);
			}
			break;
		case RIG_MODE_LSB:
			strcpy(buffer,ssb_mode_demod);
			strcpy(buffer1,ssb_mode_mod);
			strcpy(p_name,"id");
			strcpy(p_name1,"title");
			strcpy(p_value,"\"SSBDemod\"");
			strcpy(p_value1,"\"LSB Demodulation\"");
			buffer=replace_value2(buffer, p_name, p_name1, p_value, p_value1);
			strcpy(p_value,"\"SSBMod\"");
			strcpy(p_value1,"\"LSB Modulator\"");
			buffer1=replace_value2(buffer1, p_name, p_name1, p_value, p_value1);

			if(width != -1){
				if(width == 0) width = 3000;
				strcpy(p_name,"rfBandwidth");
				sprintf(p_value," %d",((int) width * (-1))); // * -1 makes it LSB
				buffer=replace_value(buffer, p_name, p_value);
				strcpy(p_name,"bandwidth");
				buffer1=replace_value(buffer1, p_name, p_value);
			}
			break;
		case RIG_MODE_USB:
			strcpy(buffer,ssb_mode_demod);
			strcpy(buffer1,ssb_mode_mod);
			strcpy(p_name,"id");
			strcpy(p_name1,"title");
			strcpy(p_value,"\"SSBDemod\"");
			strcpy(p_value1,"\"USB Demodulation\"");
			buffer=replace_value2(buffer, p_name, p_name1, p_value, p_value1);
			strcpy(p_value,"\"SSBMod\"");
			strcpy(p_value1,"\"USB Modulator\"");
			buffer1=replace_value2(buffer1, p_name, p_name1, p_value, p_value1);

			if(width != -1){
				if(width == 0) width = 3000;
				strcpy(p_name,"rfBandwidth");
				sprintf(p_value," %d",(int) width);
				buffer=replace_value(buffer, p_name, p_value);
				strcpy(p_name,"bandwidth");
				buffer1=replace_value(buffer1, p_name, p_value);
			}
			break;
		case RIG_MODE_DSB:
			strcpy(buffer,ssb_mode_demod);
			strcpy(buffer1,ssb_mode_mod);
			strcpy(p_name,"id");
			strcpy(p_name1,"title");
			strcpy(p_value,"\"SSBDemod\"");
			strcpy(p_value1,"\"DSB Demodulation\"");
			buffer=replace_value2(buffer, p_name, p_name1, p_value, p_value1);
			strcpy(p_value,"\"SSBMod\"");
			strcpy(p_value1,"\"DSB Modulator\"");
			buffer1=replace_value2(buffer1, p_name, p_name1, p_value, p_value1);
			
			if(width != -1){
				if(width == 0) width = 3000;
				strcpy(p_name,"rfBandwidth");
				sprintf(p_value," %d",(int) width);
				buffer=replace_value(buffer, p_name, p_value);
				strcpy(p_name,"bandwidth");
				buffer1=replace_value(buffer1, p_name, p_value);
				strcpy(p_name,"dsd");
				sprintf(p_value," %d",1);     // Setting dsd=1 makes mode DSB
				buffer=replace_value(buffer, p_name, p_value);
				buffer1=replace_value(buffer1, p_name, p_value);
			}
			break;
/*
		case RIG_MODE_FM:
			strcpy(buffer,fm_mode_demod);
			strcpy(buffer1,fm_mode_mod);
			strcpy(p_name,"id");
			strcpy(p_name1,"title");
			strcpy(p_value,"\"FMDemod\"");
			strcpy(p_value1,"\"FM Demodulation\"");
			buffer=replace_value2(buffer, p_name, p_name1, p_value, p_value1);
			strcpy(p_value,"\"FMMod\"");
			strcpy(p_value1,"\"FM Modulator\"");
			buffer1=replace_value2(buffer1, p_name, p_name1, p_value, p_value1);

			if(width != -1){
				if(width == 0) width = 3000;
			strcpy(p_name,"rfBandwidth");
			sprintf(p_value," %d",(int) width);
			buffer=replace_value(buffer, p_name, p_value);
			strcpy(p_name,"bandwidth");
			buffer1=replace_value(buffer1, p_name, p_value);
			}
			break;
*/
		case RIG_MODE_CW:
			strcpy(buffer,ssb_mode_demod);
			strcpy(buffer1,ssb_mode_mod);
			strcpy(p_name,"id");
			strcpy(p_name1,"title");
			strcpy(p_value,"\"SSBDemod\"");
			strcpy(p_value1,"\"CW Demodulation\"");
			buffer=replace_value2(buffer, p_name, p_name1, p_value, p_value1);
			strcpy(p_value,"\"SSBMod\"");
			strcpy(p_value1,"\"CW Modulator\"");
			buffer1=replace_value2(buffer1, p_name, p_name1, p_value, p_value1);
			strcpy(p_name,"rfBandwidth");
			sprintf(p_value," %d",(int) width);
			buffer=replace_value(buffer, p_name, p_value);

			if(width != -1){
				if(width == 0) width = 3000;
				strcpy(p_name,"bandwidth");
				buffer1=replace_value(buffer1, p_name, p_value);
				strcpy(p_name,"dsd");
				sprintf(p_value," %d",1);     // Setting dsd=1 makes mode DSB
				buffer=replace_value(buffer, p_name, p_value);
				buffer1=replace_value(buffer1, p_name, p_value);
			}
			break;
		default:
			strcpy(buffer,ssb_mode_demod);
                        strcpy(buffer1,ssb_mode_mod);
                        strcpy(p_name,"id");
                        strcpy(p_name1,"title");
                        strcpy(p_value,"\"SSBDemod\"");
                        strcpy(p_value1,"\"USB Demodulation\"");
                        buffer=replace_value2(buffer, p_name, p_name1, p_value, p_value1);
                        strcpy(p_value,"\"SSBMod\"");
                        strcpy(p_value1,"\"USB Modulator\"");
                        buffer1=replace_value2(buffer1, p_name, p_name1, p_value, p_value1);
			if(width != -1){
				if(width == 0) width =3000;
				strcpy(p_name,"rfBandwidth");
				sprintf(p_value," %d",(int) width);
				buffer=replace_value(buffer, p_name, p_value);
				strcpy(p_name,"bandwidth");
				buffer1=replace_value(buffer1, p_name, p_value);
			}
	}

	// Turn off both channels
        sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/run",0);
        ret =  HTTP_action_DELETE(p_URL);
	if(rig->caps->rig_type != RIG_TYPE_RECEIVER){
		sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/run",1);
		ret =  HTTP_action_DELETE(p_URL);
	}
	if((vfo & 3) != 1 ){     // Not VFOB so VFOA
		// Inset new mode of modulation device 0 channel 0 as an added channel will delete old one next.
		sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel",0);
		ret =  HTTP_action_POST(p_URL, buffer);
		if(ret != RIG_OK){
			free(buffer);
			free(buffer1);
			rig_debug(RIG_DEBUG_ERR, "%d %s: POST Error mode=%s\n", __LINE__, __func__, p_value );
			return -1;	//Failed
		}
		if(rig->caps->rig_type != RIG_TYPE_RECEIVER){
			sprintf(URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d/settings",0,1);
			ret =  HTTP_action_PATCH(p_URL, buffer);
			if(ret != RIG_OK){
				free(buffer);
				free(buffer1);
				rig_debug(RIG_DEBUG_ERR, "%d %s: PATCH Error mode=%s\n", __LINE__, __func__, p_value );
				return -1;	//Failed
			}
		}
		// delete the current mode of modulation
		sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d",0,0);
		//  This operation caused a crash in SDRAngel_pluto on earlier versions of SDRAngel_pluto.
		ret =  HTTP_action_DELETE(p_URL);
		if(ret != RIG_OK){
			free(buffer);
			free(buffer1);
			rig_debug(RIG_DEBUG_ERR, "%d %s: Delete Channel Error mode=%s\n", __LINE__, __func__, p_value );
			return -1;	//Failed
		}
	}

	if(((vfo & 3) == 1) && (rig->caps->rig_type != RIG_TYPE_RECEIVER)){
		// Inset new mode of modulation device 1 channel 0
		sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel",1);
		ret =  HTTP_action_POST(URL, buffer1);
		if(ret != RIG_OK){
			free(buffer);
			free(buffer1);
			rig_debug(RIG_DEBUG_ERR, "%d %s: POST Error mode=%s\n", __LINE__, __func__, p_value );
			return -1;	//Failed
		}
		sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d/settings",1,1);
		ret =  HTTP_action_PATCH(p_URL, buffer1);
		if(ret != 0){
			free(buffer);
			free(buffer1);
			rig_debug(RIG_DEBUG_ERR, "%d %s: PATCH Error mode=%s\n", __LINE__, __func__, p_value );
			return -1;	//Failed
		}
		// delete the current mode of modulation
		sprintf(buffer,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d",1,0);
		//  This operation caused a crash in SDRAngel_pluto on earlier versions of SDRAngel_pluto.
		ret =  HTTP_action_DELETE(buffer);
		if (ret != RIG_OK) {
			rig_debug(RIG_DEBUG_ERR, "%d %s: Delete Channel Error ret=%d\n", __LINE__, __func__, ret);
			return -1;
		}
	}

	free(buffer);
	free(buffer1);
	if(rig->caps->rig_type != RIG_TYPE_RECEIVER){
		ret=sdrangel_set_ptt(rig,1,0);
		if(ret != 0) {
			rig_debug(RIG_DEBUG_ERR, "%d %s: ret=%d\n", __LINE__, __func__, ret);
			return -1;	//Failed
		}
	}
	rig_debug(RIG_DEBUG_TRACE, "%d %s: buffer = %s\n", __LINE__, __func__, buffer);
	return RIG_OK;
}

// CMD m
// Get the current operating mode of SDRAngel_pluto channel by vfo but default is to go with first channel found.
// TODO:  Do more work to make this search for a particular channel mode.
// TODO:  Do more work on types of modes
int sdrangel_get_mode(RIG * rig, vfo_t vfo, rmode_t * mode, pbwidth_t * width) {
	int ret=0;
	char buf[HUGEBUFSIZE] = " ";
	char *buffer = &buf[0];
	char value[20];
	char *hunt_value = &value[0];
	char current_mode[30]=" ";
	char *p_current_mode=&current_mode[0];
	int buf_cnt;
	int device=0;

	if(vfo == 1)
		device=1;
	else
		device=0;

	//sprintf(buffer,"http://127.0.0.1:8091/sdrangel/devicesets");
	sprintf(buffer,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d/settings",device,0);
	ret=HTTP_action_GET(buffer);
        if(ret != 0){
                printf("Get mode  Failed! %d\n",ret);
		rig_debug(RIG_DEBUG_ERR, "%d %s: buffer:\n\%s\n", __LINE__, __func__, buffer);
                return -1;      //Failed
        }
	*mode = RIG_MODE_NONE;	// Initilize with no mode
	buf_cnt = strlen (buffer);
	strcpy (hunt_value, "channelType");
	extract_value (hunt_value, buf_cnt, buffer);
	p_current_mode = hunt_value + 1;
	if (strncmp (p_current_mode, "LSB", 3) == 0)
		*mode = RIG_MODE_LSB;
	if (strncmp (p_current_mode, "SSB", 3) == 0)
		*mode = RIG_MODE_USB;
	if (strncmp (p_current_mode, "CW", 2) == 0)
		*mode = RIG_MODE_CW;
	if (strncmp (p_current_mode, "NFM", 3) == 0)
		*mode = RIG_MODE_FM;
	if (strncmp (p_current_mode, "WFM", 3) == 0)
		*mode = RIG_MODE_WFM;
	if(strlen(p_current_mode) > 11){ 
		if (strncmp (p_current_mode, "\"Broadcast FM", 12) == 0)
			*mode = RIG_MODE_WFM;
	}
	if (strncmp (p_current_mode, "AM", 2) == 0)
	*mode = RIG_MODE_AM;
/*
	if(strlen(p_current_mode) > 5){ 
		if (strncmp (p_current_mode, "FreeDV", 6) == 0)
			*mode = RIG_MODE_ATV;
	}
	if (strncmp (p_current_mode, "DATV", 4) == 0)
		*mode = RIG_MODE_ATV;
	if (strncmp (p_current_mode, "ATV", 3) == 0)
		*mode = RIG_MODE_ATV;
*/
	if(vfo == 1)
		strcpy (hunt_value, "bandwidth");
	else
		strcpy (hunt_value, "rfBandwidth");
	extract_value (hunt_value, buf_cnt, buffer);
	*width = (pbwidth_t) atoi (hunt_value);
	if(*width < 0){
		*width=*width * (-1);
		*mode = RIG_MODE_LSB;
	}
	rig_debug(RIG_DEBUG_TRACE, "%d %s: p_current_mode = %s\n", __LINE__, __func__, p_current_mode);
	if (*width > 0)
		return RIG_OK;
	else {
		rig_debug(RIG_DEBUG_ERR, "%d %s: p_current_mode=\%s\n", __LINE__, __func__, p_current_mode);
		return -1;     // Error condition
	}
}

// CMD X
int sdrangel_set_split_mode(RIG *rig, vfo_t vfo, rmode_t tx_mode, pbwidth_t tx_width){
	int ret=0;
	split_mode_status=1;
	current_TX_VFO=1;   // 0=VFO_A   1=VFO_B
	ret=sdrangel_set_mode(rig, current_TX_VFO, tx_mode, tx_width); 
	rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);
	if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: tx_mode=%ld\n", __LINE__, __func__, tx_mode);
		return -1;
	}
	ret= set_modAFInput(MIC);
	if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: modAFInput=%d\n", __LINE__, __func__, MIC);
		return -1;
	}
	return RIG_OK;
}

// CMD x
int sdrangel_get_split_mode(RIG *rig, vfo_t vfo, rmode_t *tx_mode, pbwidth_t *tx_width){
	int ret=0;
	ret=sdrangel_get_mode(rig, 1,  tx_mode, tx_width);   // VFO at 1 get TX mode
	rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);
	if(ret < 0) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: tx_mode=%ld\n", __LINE__, __func__, *tx_mode);
		return -1;	// Error Condition
	}
	return RIG_OK;
}

// CMD T
int sdrangel_set_ptt(RIG * rig, vfo_t vfo, ptt_t ptt) {
	int off=0;
        int ret=0;
        char url[100];
        char *URL=&url[0];
        //char buf[READBUFSIZE] = "DELETE /sdrangel/deviceset/0/device/run HTTP/1.1\r\nHost: 127.0.0.1:8091\r\n\r\n";
        char buf[READBUFSIZE] = " ";
        char *buffer = &buf[0];

// Following section operates a computer USB based PTT relay board made by SainSmart
	if(ptt == 1) {
		if(rig->state.relay_info)         // Has to be set to 1 or more in CONFIGURE file
			ptt_relay(rig->state.relay_info, 1);
		off = 0;
	}
	else {
		if(rig->state.relay_info)         // Has to be set to 1 or more in CONFIGURE file
			ptt_relay(rig->state.relay_info, 0);
		off = 1;
	}


        sprintf(buffer,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/run",off);
        ret =  HTTP_action_DELETE(buffer);

        sprintf(URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/run",ptt);
        strcpy(buffer,"");
        ret =  HTTP_action_POST(URL, buffer);

        sprintf(URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/focus",ptt);
        strcpy(buffer,"");
        ret=HTTP_action_PATCH(URL,buffer);
        rig_debug(RIG_DEBUG_TRACE, "%d %s: url = %s\n", __LINE__, __func__, URL);
        if(ret != 0) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: buffer:\n\%s\n", __LINE__, __func__, buffer);
                return -1;      //Failed
	}
	// set_modAFInput(MIC); 
        return RIG_OK;
}

// CMD t
int sdrangel_get_ptt(RIG * rig, vfo_t vfo, ptt_t *ptt) {
	int ret=0;
	int buf_cnt=0;
	char buf[2000]=" ";
	char *buffer=&buf[0];
	char hunt_value[25];
	char *p_hunt_value=&hunt_value[0];

	strcpy(buffer,"http://127.0.0.1:8091/sdrangel/deviceset/1");
	ret=HTTP_action_GET(buffer);
	if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Set Get PTT Failed! buffer:\n\%s\n", __LINE__, __func__, buffer);
		return -1;      //Failed
	}
	buf_cnt = strlen (buffer);
        strcpy (p_hunt_value, "state");
        extract_value (hunt_value, buf_cnt, buffer);
	if(strncmp((p_hunt_value+1),"running",4) == 0)
		*ptt=1;
	else
		*ptt=0;
        rig_debug(RIG_DEBUG_TRACE, "%d %s: hunt_value = %s\n", __LINE__, __func__, p_hunt_value);
	return RIG_OK;
}

// Support Function
// Function to replace a value in a REST interface data set.
char *replace_value2(char *p_buf, char *name1, char *name2, char *new_value1, char *new_value2) {
        int j=0;
        int i=0;
        int o=0;
        int found=0;
        int len1=strlen(name1);
        int len_name1 = strlen(new_value1);
        int len2=strlen(name2);
        int len_name2 = strlen(new_value2);
        int p_buf_len = strlen(p_buf);
        int safty=0;
        char *p_buf_out=calloc(4000, sizeof(char));

        while((p_buf[i] != '\0') && (safty++ < (p_buf_len+100))  ){
                if( (i > 2) && (p_buf[i]=='\r') && (p_buf[i-1] == '}'))
                        break;
                p_buf_out[o]=p_buf[i];
                o++;
                i++;
		if(strncmp(&p_buf[i],name1,len1) == 0){
                        while(p_buf[i] != ':') {
                                p_buf_out[o]=p_buf[i];
                                o++;
                                i++;
                        }
                        p_buf_out[o]=p_buf[i];
                        o++;      // get past :
                        found=1;
                        while(p_buf[i] != ',') // Walking over old value
                                i++;
                        for(j=0;j<len_name1;j++){
                                p_buf_out[o]=(new_value1[j]);
                                o++;
                        }
                        p_buf_out[o]=',';
                        o++;
                        i++;
                }

                if(strncmp(&p_buf[i],name2,len2) == 0){
                        while(p_buf[i] != ':') {
                                p_buf_out[o]=p_buf[i];
                                o++;
                                i++;
                        }
                        p_buf_out[o]=p_buf[i];
                        o++;      // get past :
                        found=1;
                        while(p_buf[i] != ',') // Walking over old value
                                i++;
                        for(j=0;j<len_name2;j++){
                                p_buf_out[o]=(new_value2[j]);
                                o++;
                        }
                        p_buf_out[o]=',';
                        o++;
                        i++;
                }
        }
        p_buf_out[o]='\0';
        rig_debug(RIG_DEBUG_TRACE, "%d %s:  value found true = %d\n", __LINE__, __func__, found);
        if(found)
                return &p_buf_out[0]; // Found replacement value and fixed
        else {
		rig_debug(RIG_DEBUG_ERR, "%d %s: Did not find |%s|\n", __LINE__, __func__, name2);
                return NULL;
	}

}

// Support Function
// Used to locate the position for the new value in a buffer. Runs in the background.
int locate_position(char *p_buf, char *p_name) {
        int i=0;
        int len=strlen(p_name);
        int p_buf_len = strlen(p_buf);
        int safty=0;


        while((p_buf[i] != '\0') && (safty++ < (p_buf_len+100))  ){
                if( (i > 2) && (p_buf[i]=='\r') && (p_buf[i-1] == '}'))
                        break;
                i++;
                if(strncmp(&p_buf[i],p_name,len) == 0){     //Looking for the item to replace
                        while(p_buf[i] != ':') {          // Move up past the name
                                i++;
                        }
                        i++;    // get past :          // Move up 1 more blank space
			rig_debug(RIG_DEBUG_TRACE, "%d %s:  p_name= %s location = %d\n", __LINE__, __func__, p_name, i);
			return i;	// Position to start writing new value in buffer
		}
	}
	rig_debug(RIG_DEBUG_ERR, "%d %s: Did not find |%s|\n", __LINE__, __func__, p_name);
	return -1;  	      // Error Condition  Did not find name	
}


// Support function
// Function to replace a value in a REST interface data set.
char *replace_value(char *p_buf, char *name, char *new_value) {
        int j=0;
	int i=0;
	int o=0;
        int found=0;
        int len=strlen(name);
        int len_name = strlen(new_value);
        int p_buf_len = strlen(p_buf);
	int safty=0;
	int len_buf = strlen(p_buf);

	char *p_buf_out=calloc((len_buf+20), sizeof(char));
        while((p_buf[i] != '\0') && (safty++ < (p_buf_len+100))  ){
		if( (i > 2) && (p_buf[i]=='\r') && (p_buf[i-1] == '}'))
			break;
                p_buf_out[o]=p_buf[i];
                o++;
                i++;
                if(strncmp(&p_buf[i],name,len) == 0){     //Looking for the item to replace
                        while(p_buf[i] != ':') {          // Move up past the name
				p_buf_out[o]=p_buf[i];
                                o++;
                                i++;
                        }
			p_buf_out[o]=p_buf[i];		// Move up 1 more blank space
			o++;      // get past :
                        found=1;
                        while(p_buf[i] != ',') // Walking over old value // Move past old value
                                i++;
                        for(j=0;j<len_name;j++){	// Put in new reading
                                p_buf_out[o]=(new_value[j]);
                                o++;
                        }
                        p_buf_out[o]=',';		//Put comma in at end
                        o++;
                        i++;
                }
        }						// Loop though all othe buffer dataOB
        p_buf_out[o]='\0';
        rig_debug(RIG_DEBUG_TRACE, "%d %s: value found true = %d\n", __LINE__, __func__, found);
        if(found)
                return &p_buf_out[0]; // Found replacement value and fixed
        else {
		rig_debug(RIG_DEBUG_ERR, "%d %s: buffer count position=%d name=%s  value=%s\n", __LINE__, __func__, o, name, new_value);
                return NULL;
	}

}

// CMD V
// SDRAngel_pluto only has one vfo but we may use this to switch between channels later.
int sdrangel_set_vfo(RIG *rig,vfo_t vfo) {
	int ret=0;
	vfo_t TxbitandVFOcurr = (3 << 29);
	freq_t freq;
	 switch (vfo & (RIG_VFO_A + RIG_VFO_B + RIG_VFO_MAIN + RIG_VFO_MAIN_A + RIG_VFO_MAIN_B + TxbitandVFOcurr)) {   //TX VFO has to be one of these
			case RIG_VFO_A:
			case RIG_VFO_MAIN:
			case RIG_VFO_MAIN_A:
	//                      printf("VFO_A selection\n");
				split_mode_status = 0;	// TX VFOA so force split to none.
				current_TX_VFO = 0;   // VFO_A
				ret=sdrangel_get_freq(rig, 0, &freq);  // Get RX VFO freq
				if(ret != RIG_OK){
					rig_debug(RIG_DEBUG_ERR, "%d %s: Error Reading Rx VFO ret=%d\n", __LINE__, __func__, ret );
					return -1;
				}
				ret=sdrangel_set_freq(rig, 1, freq);  // Set TX VFO freq with RX read
				if(ret != RIG_OK){
					rig_debug(RIG_DEBUG_ERR, "%d %s: Error Reading Tx VFO ret=%d\n", __LINE__, __func__, ret );
					return -1;
				}
				break;

			case RIG_VFO_B:
			case RIG_VFO_MAIN_B:
			case (3 << 29):   // current VFO and bit to indicate the TX_VFO
		//              printf("VFO_B selection\n");
				split_mode_status = 1;	// Different VFO for TX so force split mode.
				current_TX_VFO = 1;    // VFO_B
				ret=sdrangel_get_freq(rig, 0, &freq);  // Get RX VFOA freq so we can force a match in VFOB
				if(ret != 0){
					rig_debug(RIG_DEBUG_ERR, "%d %s: Error getting VFO_B\n", __LINE__, __func__);
					return -1;      //Failed
				}
				ret = sdrangel_set_freq(rig, 1,freq);
				if(ret != 0){
					rig_debug(RIG_DEBUG_ERR, "%d %s: Error setting VFO_B\n", __LINE__, __func__);
					return -1;      //Failed
				}
				break;

			default:
				printf("vfo = %u\n",(vfo & (RIG_VFO_A + RIG_VFO_B + RIG_VFO_MAIN + RIG_VFO_MAIN_A + RIG_VFO_MAIN_B)));
				printf("Default selection\n");
				rig_debug(RIG_DEBUG_ERR, "%d %s: Error seting VFO_A\n", __LINE__, __func__);
				return -1;      //Failed
        }

	rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);
        if(ret != 0) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: vfo=%d\n", __LINE__, __func__, vfo);
		return -1;
	}
	return RIG_OK;
}

// CMD v
int sdrangel_get_vfo(RIG *rig, vfo_t *vfo) {
	if(current_TX_VFO == 0)
		*vfo = RIG_VFO_A;
	else
		*vfo = RIG_VFO_B;
	rig_debug(RIG_DEBUG_TRACE, "%d %s:  vfo= %d\n", __LINE__, __func__, *vfo);
	return RIG_OK;

}

// CMD F
// Uses CURL to write to REST interface and set frequency.
int sdrangel_set_freq(RIG * rig, vfo_t vfo, freq_t freq) {
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
        rounded_freq = ((long) (freq/1000))*1000;
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
		ret=sdrangel_set_rit(rig, vfo, kHz_portion);
		if(ret != 0){
			rig_debug(RIG_DEBUG_ERR,"%d %s: buffer:\n\%s\n", __LINE__, __func__, p_buf);
			return -1;      //Failed
		}
	}
	if(split_mode_status == 1 || ((vfo & 1) == 1)){
		vfob = freq;
		sprintf(p_buf,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",1); 
		ret=HTTP_action_GET(p_buf);
		if(ret != 0){
			printf("Get mode  Failed! %d\n",ret);
			rig_debug(RIG_DEBUG_ERR,"%d %s: buffer:\n\%s\n", __LINE__, __func__, p_buf);
			return -1;      //Failed
		}
		strcpy(p_name,"centerFrequency");
		p_buf2=replace_value(p_buf, p_name, p_value);
		sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",1); 
		ret =  HTTP_action_PATCH(p_URL, p_buf2);
		if(ret != 0){
			free(p_buf);
			free(p_buf2);
			rig_debug(RIG_DEBUG_ERR, "%d %s: p_URL: %s\n", __LINE__, __func__, p_URL);
			return -1;      //Failed
		}
		ret=sdrangel_set_rit(rig, vfo, kHz_portion);
		if(ret != 0){
			rig_debug(RIG_DEBUG_ERR,"%d %s: buffer:\n\%s\n", __LINE__, __func__, p_buf);
			return -1;      //Failed
		}
	}
	free(p_buf);
	free(p_buf2);
	rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);
	return RIG_OK;
}

// CMD f      Get Frequency of LO pointed at by VFO
int sdrangel_get_freq(RIG * rig, vfo_t vfo, freq_t * freq) {
        int ret=0;
        char buf[MIDBUFSIZE] = " ";
        char *buffer = &buf[0];
        char value[20];
        char *hunt_value = &value[0];
        int buf_cnt;
        int current_vfo=(vfo & 3) -1;    // Limit this to VFO 0 ro 1
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
        if(current_vfo) {
                ret=sdrangel_get_xit(rig,vfo,p_rit);
                if(*p_rit < -9999000 || *p_rit > 9999000){
                        *p_xit = 0;
                        printf("RIT currently set to a bad value and the reading will be ignored!\n");
                }
        }
        else {
                ret=sdrangel_get_rit(rig,vfo,p_rit);
                if(*p_rit < -9999000 || *p_rit > 9999000){
                        *p_rit = 0;
                        printf("RIT currently set to a bad value and the reading will be ignored!\n");
                }
        }
        *freq = *freq + (freq_t) (*p_rit + *p_xit);
        rig_debug(RIG_DEBUG_TRACE, "%d %s: frequency = %lf\n", __LINE__, __func__, *freq);
        if (*freq > 0)
                return RIG_OK;
        else {
                rig_debug(RIG_DEBUG_ERR, "%d %s: hunt_value=%s\n", __LINE__, __func__, hunt_value );
                return -1;      // Error condition
        }
}

// CMD I         **** This function does not meet the timing requirements of WSJTX
int sdrangel_set_split_freq(RIG * rig, vfo_t vfo, freq_t freq){
	int ret=0;
	long rounded_freq=0;
        shortfreq_t kHz_portion=0;
	split_mode_status = 1;
	vfob=freq;
	//printf("VFO_B selection\n");
	current_TX_VFO = 1;    // VFO_B
        rounded_freq = ((long) (freq/1000))*1000;
        kHz_portion = (shortfreq_t) (freq - rounded_freq);
        freq =(freq_t) rounded_freq;
	ret = sdrangel_set_freq(rig, 1,freq);
	if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Error setting VFO_B\n", __LINE__, __func__);
		return -1;      //Failed
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
int sdrangel_get_split_freq(RIG * rig, vfo_t vfo, freq_t *freq){
	int ret=0;
	int this_vfo=0;
	shortfreq_t xit=0;
	shortfreq_t *p_xit=&xit;
	vfo_t TxbitandVFOcurr = (3 << 29);
	switch (vfo & (RIG_VFO_A + RIG_VFO_B + RIG_VFO_MAIN + RIG_VFO_MAIN_A + RIG_VFO_MAIN_B + TxbitandVFOcurr)) {   //TX VFO has to be one of these
		case RIG_VFO_A:
		case RIG_VFO_MAIN:
		case RIG_VFO_MAIN_A:
			this_vfo = 1;	
			break;

		case RIG_VFO_B:
		case RIG_VFO_MAIN_B:
		case (3 << 29):   // current VFO and bit to ndicate the TX_VFO
			this_vfo = 2;
			break;

		default:
			rig_debug(RIG_DEBUG_ERR, "%d %s: Error seting VFO_A\n", __LINE__, __func__);
			return -1;      //Failed
	}
	ret = sdrangel_get_freq(rig, this_vfo,freq);
	if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: Error seting VFO_A\n", __LINE__, __func__);
		return -1;      //Failed
	}
	ret = sdrangel_get_xit(rig, 1, p_xit);
	*freq = *freq + *p_xit;
	rig_debug(RIG_DEBUG_TRACE, "%d %s: vfo=%d  freq=%lf\n", __LINE__, __func__,vfo,*freq);
        return RIG_OK;
}

// CMD K
int sdrangel_set_split_freq_mode(RIG *rig, vfo_t vfo, freq_t freq, rmode_t mode, pbwidth_t width){
	int ret=0;
	split_mode_status = 1;		// Split mode
	current_TX_VFO = 1;		// Split mode VFO
	ret=sdrangel_set_mode (rig, current_TX_VFO, mode, width);
	if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: mode=%lu\n", __LINE__, __func__, mode);
		return -1;
	}
	ret=sdrangel_set_split_freq(rig, 1, freq);    // Force vco to Tx VFOB
	if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: frequency=%lf\n", __LINE__, __func__, freq);
		return -1;
	}
	ret= set_modAFInput(MIC);

	if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: modAFInput=%d\n", __LINE__, __func__, MIC);
		return -1;
	}

	rig_debug(RIG_DEBUG_TRACE, "%d %s: \n", __LINE__, __func__);
	return RIG_OK;
}

// CMD k
int sdrangel_get_split_freq_mode(RIG *rig, vfo_t vfo, freq_t *freq, rmode_t *mode, pbwidth_t *width){
	int ret=0;
	ret = sdrangel_get_mode (rig, 2, mode, width); 
	if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: mode=%ld\n", __LINE__, __func__, *mode);
		return -1;
	}
	ret=sdrangel_get_freq(rig, 2,freq);
	if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: mode=%lf\n", __LINE__, __func__, *freq);
		return -1;
	}
	rig_debug(RIG_DEBUG_TRACE, "%d %s: split = %d\n", __LINE__, __func__, split_mode_status);
	return RIG_OK;
}

// CMD (Not defined)
// Uses CURL to write to REST interface and set Primary VFO, Tx or Rx.
int sdrangel_set_vfo_freq (RIG * rig, vfo_t vfo, freq_t freq) {
	int ret=0;
	// Array only needs to be 798 but 900 used for safty. Could reduce if memory issues.
        char buf[MIDBUFSIZE]=" ";
        char *p_buf=&buf[0];
        char buf1[MIDBUFSIZE]=" ";
        char *p_buf1=&buf1[0];
	char URL[100];
        char *p_URL = &URL[0];
	char value[25]=" ";
	char *p_value=&value[0];
	sprintf(p_value,"%11.0lf",freq);
	char name[25]="centerFrequency";
	char *p_name=&name[0];
	sprintf(p_buf,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",vfo);
	ret=HTTP_action_GET(p_buf);
        if(ret != 0){
                printf("Set freq Failed! %d\n",ret);
                return -1;      //Failed
        }
	p_buf1=replace_value(p_buf, p_name, p_value);
	if(p_buf == NULL){
		free(p_buf1);
		printf("Error working with SDRangel_pluto. Be sure both R0 and T1 setup properly.");
		rig_debug(RIG_DEBUG_ERR, "%d %s: p_value=%s\n", __LINE__, __func__, p_value);
		return -1;
	}
	sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/device/settings",vfo);
	ret=HTTP_action_PATCH(p_URL,p_buf1);
	free(p_buf1);
	rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);
	if(ret != 0) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: p_buf:%s\n", __LINE__, __func__, p_buf);
		return -1;
	}
	return RIG_OK;
}

// CMD S
// Sets the vfo for TX
int sdrangel_set_split_vfo (RIG * rig, vfo_t vfo, split_t split, vfo_t tx_vfo){
	if(split == 0 || split == 1  || split == 2)
		split_mode_status = split;
		//rig->state.cache.split = split;
	else {
		rig_debug(RIG_DEBUG_ERR, "%d %s: Split value not valid at, tx_vfo=%d\n", __LINE__, __func__, tx_vfo);
		return -1;
	}
	vfo_t TxbitandVFOcurr = (3 << 29);
	switch (tx_vfo & (RIG_VFO_A + RIG_VFO_B + RIG_VFO_MAIN + RIG_VFO_MAIN_A + RIG_VFO_MAIN_B + TxbitandVFOcurr)) {   //TX VFO has to be one of these
		case RIG_VFO_A: 
			current_TX_VFO = 0; // RIG_VFO_A, Will not work if in split mode so error out
			break;

		case RIG_VFO_B: 
			current_TX_VFO = 1;  // RIG_VFO_B, If in split mode then VFO_B for TX and VFO_A will be for RX
			break;                       // When Not in split mode then VFO_A and VFOB will be locked and changes in either will track

		case RIG_VFO_MAIN: 	         		
			current_TX_VFO = 0; // RIG_VFO_A, Will only work if not in split. VFO_A and VFO_B will be locked and changes in either will track
			break;

		case RIG_VFO_MAIN_A: 
			current_TX_VFO = 0; //RIG_VFO_A, Will only work if not in split. VFO_A and VFO_B will be locked and changes in either will track
			break;

		case RIG_VFO_MAIN_B:                // If in split mode then VFO_B for TX and VFO_A will be for RX 
		case (3 << 29):   // current VFO and bit to ndicate the TX_VFO
			current_TX_VFO = RIG_VFO_B;    // When Not in split mode then VFO_A and VFOB will be locked and changes in either will track
			break;
		default:
			rig_debug(RIG_DEBUG_ERR, "%d %s: tx_vfo=%d\n", __LINE__, __func__, tx_vfo);
			return -1;
	}
	rig_debug(RIG_DEBUG_TRACE, "%d %s:  tx_vfo= %d\n", __LINE__, __func__, tx_vfo);
	return RIG_OK;
}

// CMD s
int sdrangel_get_split_vfo (RIG * rig, vfo_t vfo, split_t *split, vfo_t *tx_vfo){
	if(split_mode_status == 1) {
		*tx_vfo=current_TX_VFO+1;		//Hamlib expects 1=VFO_A   2=VFO_B locally values are 1 less
		*split=1;
	}
	else {
		*tx_vfo=1;                             // Sets VFOA
		*split=0;
		split_mode_status=0;
		current_TX_VFO=0;   // 0=VFO_A   1=VFO_B
	}
	rig_debug(RIG_DEBUG_TRACE, "%d %s:  split=%d  tx_vfo= %d\n", __LINE__, __func__, *split, *tx_vfo);
        return RIG_OK;
}


// CMD (not defined yet)
int sdrangel_get_vfo_info(RIG *rig, vfo_t vfo, freq_t *freq, rmode_t *mode, pbwidth_t *width, split_t *split) {
	int ret = 0;
	vfo_t TxbitandVFOcurr = (3 << 29);
	switch (vfo & (RIG_VFO_A + RIG_VFO_B + RIG_VFO_MAIN + RIG_VFO_MAIN_A + RIG_VFO_MAIN_B + TxbitandVFOcurr)) {   //TX VFO has to be one of these
		case RIG_VFO_A:
		case RIG_VFO_MAIN:
		case RIG_VFO_MAIN_A:
			ret = sdrangel_get_freq(rig, 0,freq);
			if(ret != 0){
				rig_debug(RIG_DEBUG_ERR, "%d %s: reading VFO_A\n", __LINE__, __func__);
				return -1;      //Failed
			}
			ret = sdrangel_get_mode (rig, 0, mode, width); 
			if(ret != 0){
				rig_debug(RIG_DEBUG_ERR, "%d %s: reading VFO_A\n", __LINE__, __func__);
				return -1;      //Failed
			}
			*split = split_mode_status;
			break;
		case RIG_VFO_B:
		case RIG_VFO_MAIN_B:
		case (3 << 29):   // current VFO and bit to ndicate the TX_VFO
			ret = sdrangel_get_freq(rig, 1,freq);
			if(ret != 0){
				rig_debug(RIG_DEBUG_ERR, "%d %s: reading VFO_A\n", __LINE__, __func__);
				return -1;      //Failed
			}
			ret = sdrangel_get_mode (rig, 1, mode, width); 
			if(ret != 0){
				rig_debug(RIG_DEBUG_ERR, "%d %s: reading VFO_A\n", __LINE__, __func__);
				return -1;      //Failed
			}
			*split = split_mode_status;
			break;                       // When Not in split mode then VFO_A and VFOB will be locked and changes in either will track

		default:
			rig_debug(RIG_DEBUG_ERR, "%d %s: tx_vfo=%d\n", __LINE__, __func__, current_TX_VFO);
			return -1;
	}

	return RIG_OK;
}

// CMD Z
// Uses CURL to write to REST interface and set XIT.
int sdrangel_set_xit(RIG *rig, vfo_t vfo, shortfreq_t xit) {
	int ret=0;
        char *buffer = (char *) calloc(HUGEBUFSIZE, sizeof(char));
	char *buffer2; 
	char URL[100];
        char *p_URL = &URL[0];
	char value[20]=" ";
	char *p_value=&value[0];
	char name[25]=" ";
	char *p_name=&name[0];

        sprintf(buffer,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d/settings",1,0);
	ret=HTTP_action_GET(buffer);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: buffer:\n%s\n", __LINE__, __func__, buffer);
                return -1;      //Failed
        }
        // Update XIT value
        strcpy(p_name,"inputFrequencyOffset");
	sprintf(p_value," %ld",xit);
        buffer2=replace_value(buffer, p_name, p_value);
	free(buffer);
        sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d/settings",1,0);
        ret =  HTTP_action_PATCH(p_URL, buffer2);
	free(buffer2);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: p_URL:Set XIT Failed!\np_URL= %s\n", __LINE__, __func__, p_URL);
                return -1;      //Failed
        }
	rig_debug(RIG_DEBUG_TRACE, "%d %s:  p_value= %s\n", __LINE__, __func__, p_value);
	return RIG_OK;
}


// CMD z
// Uses CURL to write to REST interface and get XIT.
int sdrangel_get_xit(RIG *rig, vfo_t vfo, shortfreq_t *xit) {
	int ret=0;
	int buf_cnt=0;
        char *buffer=(char *) calloc(4000, sizeof(char));
        char value[30]=" ";
        char *hunt_value=&value[0];

        sprintf(buffer,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d/settings",1,0);
        ret=HTTP_action_GET(buffer);
	if(ret != RIG_OK) {
		rig_debug(RIG_DEBUG_ERR, "%d %s: buffer:\n%s\n", __LINE__, __func__, buffer);
		return -1;   //Error condition
	}
	buf_cnt = strlen (buffer);
	strcpy (value, "inputFrequencyOffset");
	extract_value(hunt_value, buf_cnt, buffer);
	*xit = (shortfreq_t) strtoul(hunt_value,NULL,10);
	free(buffer);
	rig_debug(RIG_DEBUG_TRACE, "%d %s:  hunt_value= %s\n", __LINE__, __func__, hunt_value);
        return RIG_OK;
}


// CMD J
// Uses CURL to write to REST interface and set RIT.
int sdrangel_set_rit(RIG *rig, vfo_t vfo, shortfreq_t rit) {
	int ret=0;
	char buf[4000]=" ";
	char *buffer=&buf[0];
        char *buffer2;
	//buffer2 = calloc(4000, sizeof(char));
	char URL[100];
        char *p_URL = &URL[0];
	char value[30]=" ";
	char *p_value=&value[0];
	char name[30]=" ";
	char *p_name=&name[0];

        sprintf(buffer,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d/settings",0,0);
	ret=HTTP_action_GET(buffer);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR , "%d %s: buffer:\n%s\n", __LINE__, __func__, buffer);
                return -1;      //Failed
        }
        // Update RIT value
        strcpy(p_name,"inputFrequencyOffset");
	sprintf(p_value,"%ld",rit);
        buffer2=replace_value(buffer, p_name, p_value);
        sprintf(p_URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d/settings",0,0);
        ret =  HTTP_action_PATCH(p_URL, buffer2);
	free(buffer2);
        if(ret != 0){
                printf("Set RIT Failed! %d\n",ret);
		rig_debug(RIG_DEBUG_ERR , "%d %s: p_URL: %s\n", __LINE__, __func__, p_URL);
                return -1;      //Failed
        }
	rig_debug(RIG_DEBUG_TRACE, "%d %s:  p_value= %s\n", __LINE__, __func__, p_value);
	return RIG_OK;

}

// CMD j
// Uses CURL to write to REST interface and get RIT.
int sdrangel_get_rit(RIG *rig, vfo_t vfo, shortfreq_t *rit) {
	int ret=0 ;
	int buf_cnt=0;
        char buf[MIDBUFSIZE]=" ";
        char *buffer=&buf[0];
        char value[30]="1234";
        char *hunt_value=&value[0];

        sprintf(buffer,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d/settings",0,0);
	ret=HTTP_action_GET(buffer);
        if(ret != 0){
		rig_debug(RIG_DEBUG_ERR, "%d %s: buffer:\n%s\n", __LINE__, __func__, buffer);
                return -1;      //Failed
        }
	buf_cnt = strlen (buffer);
	strcpy (value, "inputFrequencyOffset");
	extract_value(hunt_value, buf_cnt, buffer);
        *rit=(shortfreq_t) atol(hunt_value);
	rig_debug(RIG_DEBUG_TRACE, "%d %s:  hunt_value= %s\n", __LINE__, __func__, hunt_value);
        return RIG_OK;
}


//  REST Interface support code Below here


static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        size_t realsize = size * nmemb;
        struct MemoryStruct *mem = (struct MemoryStruct *)userp;

        char *ptr = realloc(mem->memory, mem->size + realsize + 1);
        if(ptr == NULL) {
                /* out of memory! */
                printf("not enough memory (realloc returned NULL)\n");
                return 0;
        }

        mem->memory = ptr;
        memcpy(&(mem->memory[mem->size]), contents, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;

        return realsize;
}

int HTTP_action_GET(char *buffer) {
        CURLcode ret;
        CURL *hnd;
        struct MemoryStruct chunk;

        chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
        chunk.size = 0;    /* no data at this point */

        curl_global_init(CURL_GLOBAL_ALL);
        hnd = curl_easy_init();
        if(hnd == NULL)
                return 390;     // Unknown error

        // specify URL to get
        curl_easy_setopt(hnd, CURLOPT_URL, buffer);

        // send all data to this function
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        // we pass our 'chunk' struct to the callback function
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);

        // some servers don't like requests that are made without a user-agent field, so we provide one  
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "libcurl-agent/1.1");

        // get it!
        ret = curl_easy_perform(hnd);

        // check for errors
        if(ret != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
        }

        // cleanup curl stuff
        curl_easy_cleanup(hnd);
        strcpy(buffer,chunk.memory);
	int position=(int) strlen(buffer) +1;

	buffer[position]='\0';

        // we're done with libcurl, so clean it up
        curl_global_cleanup();

        free(chunk.memory);

        hnd = NULL;
        rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);

        return RIG_OK;
}


int HTTP_action_DELETE(char *buffer) {
	CURLcode ret;
	CURL *hnd;
	struct MemoryStruct chunk;

	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
        chunk.size = 0;    /* no data at this point */

	hnd = curl_easy_init();
		if(hnd == NULL)
			return 390;     // Unknown error


	ret = curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 2048L);
		if(ret != CURLE_OK) {
			return 400;     // Unknown error
		}

	ret = curl_easy_setopt(hnd, CURLOPT_URL, buffer );
		if(ret != CURLE_OK) {
			if(ret == CURLE_OUT_OF_MEMORY)
				return 202;
			return 410;     // Unknown error
		}

	ret = curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
		if(ret != CURLE_OK) {
			return 420;     // Unknown error
		}

	ret = curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "DELETE");
		if(ret != CURLE_OK) {
			if(ret == CURLE_UNKNOWN_OPTION)
				return 430;
			if(ret == CURLE_OUT_OF_MEMORY)
				return 440;
			return 450;     // Unknown error
		}


        // send all data to this function  
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        // we pass our 'chunk' struct to the callback function 
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);

        // some servers don't like requests that are made without a user-agent field, so we provide one 
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	ret = curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
        if(ret != CURLE_OK) {
                if(ret == CURLE_UNKNOWN_OPTION)
			return 460;
                if(ret == CURLE_OUT_OF_MEMORY)
                        return 470;
                return 480;     // Unknown error
        }

	ret = curl_easy_perform(hnd);
        if(ret != CURLE_OK) {
                if(ret == CURLE_UNKNOWN_OPTION)
                        return 490;
                if(ret == CURLE_OUT_OF_MEMORY)
                        return 500;
                curl_easy_cleanup(hnd);
                hnd = NULL;
                return 510;     // Unknown error
                }
        curl_easy_cleanup(hnd);
        free(chunk.memory);
        hnd = NULL;
        //rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);

        return RIG_OK;
}

int HTTP_action_POST(char *URL, char *buffer) {
	CURLcode ret;
	CURL *hnd;
	struct MemoryStruct chunk;

	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
        chunk.size = 0;    /* no data at this point */

	hnd = curl_easy_init();
        if(hnd == NULL)
                return 390;     // Unknown error

	ret = curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 2048L);
        if(ret != CURLE_OK) {
                return 400;     // Unknown error
        }

	ret = curl_easy_setopt(hnd, CURLOPT_URL, URL);
        if(ret != CURLE_OK) {
                if(ret == CURLE_OUT_OF_MEMORY)
                        return 520;
                return 530;     // Unknown error
        }

	if(strlen(buffer) > 3){
		curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, buffer);
		curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE, (long)strlen(buffer));
	}
	ret = curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
        if(ret != CURLE_OK) {
                return 540;     // Unknown error
        }
	ret = curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
        if(ret != CURLE_OK) {
                if(ret == CURLE_UNKNOWN_OPTION)
                        return 550;
                if(ret == CURLE_OUT_OF_MEMORY)
                        return 560;
                return 570;     // Unknown error
        }

/*
        ret = curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
                if(ret != CURLE_OK) {
                        if(ret == CURLE_UNKNOWN_OPTION)
                        return 580;
                        if(ret == CURLE_OUT_OF_MEMORY)
                                return 590;
                        return 600;     // Unknown error
                }
*/

        // send all data to this function  
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        // we pass our 'chunk' struct to the callback function 
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);

        ret = curl_easy_perform(hnd);
                if(ret != CURLE_OK) {
                        if(ret == CURLE_UNKNOWN_OPTION)
                                return 610;
                        if(ret == CURLE_OUT_OF_MEMORY)
                                return 620;

                      return 630;     // Unknown error
                }
        curl_easy_cleanup(hnd);
        free(chunk.memory);

        hnd = NULL;
        //rig_debug(RIG_DEBUG_TRACE, "%d %s: ret = %d\n", __LINE__, __func__, ret);

        return RIG_OK;
}

int HTTP_action_PATCH(char *URL, char *buffer) {
	CURLcode ret;
	CURL *hnd;
	long buffer_len=0;
	struct MemoryStruct chunk;

	chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
        chunk.size = 0;    /* no data at this point */

	hnd = curl_easy_init();
        if(hnd == NULL){
                return 390;     // Unknown error
	}

	ret = curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 8192L);
        if(ret != CURLE_OK) {
                return 400;     // Unknown error
        }

        ret = curl_easy_setopt(hnd, CURLOPT_URL, URL);
                if(ret != CURLE_OK) {
                        if(ret == CURLE_OUT_OF_MEMORY)
                                return 640;
                        return 650;     // Unknown error
                }
	ret = curl_easy_setopt(hnd, CURLOPT_USERAGENT, "libcurl-agent/1.1");

        ret = curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "PATCH");
                if(ret != CURLE_OK) {
                        if(ret == CURLE_UNKNOWN_OPTION)
                                return 670;
                        if(ret == CURLE_OUT_OF_MEMORY)
                                return 680;
                        return 690;     // Unknown error
                }

	buffer_len = (long)strlen(buffer);
        if(buffer_len > 1){
		ret = curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, buffer);

		ret = curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE, buffer_len);
        ret = curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
                if(ret != CURLE_OK) {
                        return 660;     // Unknown error
                }
        ret = curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "PATCH");
                if(ret != CURLE_OK) {
                        if(ret == CURLE_UNKNOWN_OPTION)
                                return 670;
                        if(ret == CURLE_OUT_OF_MEMORY)
                                return 680;
                        return 690;     // Unknown error
                }

	}

	//curl_reset($hnd)            // If a reset is reuired

        // send all data to this function  
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        // we pass our 'chunk' struct to the callback function 
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);

        ret = curl_easy_perform(hnd);
                if(ret != CURLE_OK) {

	if(ret == CURLE_UNKNOWN_OPTION)
                                return 760;
                        if(ret == CURLE_OUT_OF_MEMORY)
                                return 770;
                        return 780;     // Unknown error
	}
        free(chunk.memory);
        curl_easy_cleanup(hnd);
        hnd = NULL;
        rig_debug(RIG_DEBUG_TRACE,"%d %s: ret = %d\n", __LINE__, __func__, ret);

//      stop = clock();
//      printf("StopWatch=%ld\n",stop-start);
        return RIG_OK;
}

	//This block of code is to set the mic as the input device and may be required.
	//The MIC already may be set but we are not going to check and just force it to save time.
int set_modAFInput(int audio_input) {
        //int ret=0;
        char url[100];
        char *URL=&url[0];
        char buf[READBUFSIZE] = "\{\r\n  \"SSBModSettings\": \{\r\n    \"modAFInput\": 3\r\n  },\r\n  \"channelType\": \"SSBMod\",\r\n  \"direction\": 1\r\n}\r\n";
        char *buffer = &buf[0];
	// Make sure the MIC  Audio Input selection is made. 
        sprintf(URL,"http://127.0.0.1:8091/sdrangel/deviceset/%d/channel/%d/settings",1,0);
        (void)  HTTP_action_PATCH(URL, buffer);

/* This works but it returns an error so I have commented out error handling.
        if(ret != RIG_OK){
		rig_debug(RIG_DEBUG_ERR, "%d %s: ret=%d\nbuffer:\n%s\n", __LINE__, __func__, ret, buffer);
                return -1;      //Failed
	}
*/

	return RIG_OK;
}

// End of sdrangel.c


