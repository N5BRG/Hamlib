
/*
 *  Hamlib KIT backend - SDRAngel_rtlsdr software for SDR interface.
 *  Copyright (c) 2021 by Bob Stricklin N5BRG
 *
 *
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

#ifndef _SDRAngel_rtlsdr_H
#define _SDRAngel_rtlsdr_H 1
#define MAX_DEVICES 2
#define MAX_CHANNELS 2
#define HUGEBUFSIZE 4000
#define READBUFSIZE 2000
#define CHANNELBUFSIZE 1200
#define MIDBUFSIZE 900
#define sdrangel_rtlsdr_TX_VFOS (RIG_VFO_A|RIG_VFO_B|RIG_VFO_MAIN|RIG_VFO_MAIN_A|RIG_VFO_MAIN_B)
#define sdrangel_rtlsdr_RX_VFOS (RIG_VFO_A|RIG_VFO_MAIN|RIG_VFO_MAIN_A)
#define sdrangel_rtlsdr_ALL_TX_MODES (RIG_MODE_FM|RIG_MODE_AM|RIG_MODE_CW|RIG_MODE_SSB)
#define sdrangel_rtlsdr_ALL_RX_MODES (RIG_MODE_FM|RIG_MODE_AM|RIG_MODE_CW|RIG_MODE_SSB)

/* Function Prototypes  */

#include "hamlib/rig.h"
#include <curl/curl.h>
int sdrangel_rtlsdr_init();
int sdrangel_rtlsdr_reset(RIG *rig, reset_t reset);
extern int sdrangel_close(RIG *rig);
extern int sdrangel_cleanup(RIG *rig);
extern void extract_value(char *hunt_value, int buf_cnt, char *buffer);
extern int sdrangel_open (RIG * rig);
extern int sdrangel_set_ptt(RIG *rig, vfo_t vfo, ptt_t ptt);
extern char *replace_value(char *p_buf, char *name, char *new_value);
extern char *replace_value2(char *p_buf, char *name1, char *name2, char *new_value1, char *new_value2);
extern int sdrangel_set_freq (RIG * rig, vfo_t vfo, freq_t freq);
extern int sdrangel_set_rit(RIG *rig, vfo_t vfo, shortfreq_t rit); 
extern int sdrangel_get_rit(RIG *rig, vfo_t vfo, shortfreq_t *rit); 
extern int sdrangel_get_mode (RIG * rig, vfo_t vfo, rmode_t * mode, pbwidth_t * width);
extern int sdrangel_set_mode (RIG * rig, vfo_t vfo, rmode_t mode, pbwidth_t width);
extern int sdrangel_get_freq (RIG * rig, vfo_t vfo, freq_t * freq);
extern int HTTP_action_PATCH(char *URL, char *buffer);
extern int HTTP_action_POST(char *URL, char *buffer);
extern int HTTP_action_DELETE(char *buffer);
extern int HTTP_action_GET(char *buffer);
extern int locate_position(char *p_buf, char *name);
extern int sdrangel_set_device_Parameter(RIG * rig, vfo_t vfo, char *p_parameter_name, char *p_value);

const struct rig_caps sdrangel_rtlsdr_caps = {
  RIG_MODEL (RIG_MODEL_SDRANGEL_RTLSDR),
  .model_name = "sdrangel_rtlsdr",
  .mfg_name = "Open Source",
  .version = "20210323.1",
  .copyright = "RS",
  .status = RIG_STATUS_BETA,
  .rig_type = RIG_TYPE_RECEIVER,
  .port_type = RIG_PORT_NETWORK,
  .ip_address_port = "127.0.0.1:8091",
  .rig_init = sdrangel_rtlsdr_init,
  .post_write_delay = 1,
  .timeout = 1000,

  .retry = 3,

    .rx_range_list1 =   { // USA Version -- United States
	{ kHz(500), MHz(1750), sdrangel_rtlsdr_ALL_TX_MODES, W(0.1), W(1), sdrangel_rtlsdr_RX_VFOS, RIG_ANT_1, "USA" },
        RIG_FRNG_END,
    },
    .tx_range_list1 =  {
        RIG_FRNG_END,
    },


    .rx_range_list2 =   { // EUR Version -- Europe
	{kHz(500), MHz(1750), sdrangel_rtlsdr_ALL_RX_MODES, -1, -1, sdrangel_rtlsdr_RX_VFOS, RIG_ANT_1, "EUR"},
        RIG_FRNG_END,
    },
    .tx_range_list2 =   {
        RIG_FRNG_END,
    },

    .rx_range_list3 =   { // ITR Version -- Italy
        {MHz(500), MHz(1750), sdrangel_rtlsdr_ALL_RX_MODES, -1, -1, sdrangel_rtlsdr_RX_VFOS, RIG_ANT_CURR, "ITR"},
        RIG_FRNG_END,
    },
    .tx_range_list3 =  {
        RIG_FRNG_END,
    },

    .rx_range_list4 =   { // TPE Version -- Taiwan ??
        {kHz(500), MHz(1750), sdrangel_rtlsdr_ALL_RX_MODES, -1, -1, sdrangel_rtlsdr_RX_VFOS, RIG_ANT_CURR, "TPE"},
        RIG_FRNG_END,
    },
    .tx_range_list4 =  {
        RIG_FRNG_END,
    },

    .rx_range_list5 =   { // KOR Version -- Republic of Korea
        {kHz(500), MHz(1500), sdrangel_rtlsdr_ALL_RX_MODES, -1, -1, sdrangel_rtlsdr_RX_VFOS, RIG_ANT_CURR, "KOR"},
        RIG_FRNG_END,
    },
    .tx_range_list5 =  {
        RIG_FRNG_END,
    },

	.tuning_steps = {{sdrangel_rtlsdr_ALL_RX_MODES, Hz (1)}, RIG_TS_END,},

	.rig_open = sdrangel_open,
	.get_freq = sdrangel_get_freq,
	.set_freq = sdrangel_set_freq,
        .set_rit = sdrangel_set_rit,
        .get_rit = sdrangel_get_rit,
        .set_mode = sdrangel_set_mode,
        .get_mode = sdrangel_get_mode,
	.reset = sdrangel_rtlsdr_reset,
	.rig_cleanup = sdrangel_cleanup
}; 

#endif   /* _SDRAngel_rtlsdr_H */


