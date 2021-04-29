
/*
 *  Hamlib KIT backend - SDRAngel_pluto software for SDR interface.
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

#ifndef _SDRAngel_pluto_H
#define _SDRAngel_pluto_H 1
#define MAX_DEVICES 2
#define MAX_CHANNELS 2
#define HUGEBUFSIZE 4000
#define READBUFSIZE 2000
#define CHANNELBUFSIZE 1200
#define MIDBUFSIZE 900
#define sdrangel_pluto_TX_VFOS (RIG_VFO_A|RIG_VFO_B|RIG_VFO_MAIN|RIG_VFO_MAIN_A|RIG_VFO_MAIN_B)
#define sdrangel_pluto_RX_VFOS (RIG_VFO_A|RIG_VFO_MAIN|RIG_VFO_MAIN_A)
#define sdrangel_pluto_ALL_TX_MODES (RIG_MODE_FM|RIG_MODE_AM|RIG_MODE_CW|RIG_MODE_SSB)
#define sdrangel_pluto_ALL_RX_MODES (RIG_MODE_FM|RIG_MODE_AM|RIG_MODE_CW|RIG_MODE_SSB)
#define PLUTO_EXTENDED_FREQUENCY 1	// 0=standard 325 to 3800 MHz  1=extended 70 to 6000 MHz

/* Function Prototypes  */

#include "hamlib/rig.h"
#include <curl/curl.h>
int sdrangel_pluto_init();
int sdrangel_pluto_reset(RIG *rig, reset_t reset);
extern int sdrangel_close(RIG *rig);
extern int sdrangel_cleanup(RIG *rig);
extern void extract_value(char *hunt_value, int buf_cnt, char *buffer);
extern int sdrangel_open (RIG * rig);
extern int sdrangel_set_ptt(RIG *rig, vfo_t vfo, ptt_t ptt);
extern char *replace_value(char *p_buf, char *name, char *new_value);
extern char *replace_value2(char *p_buf, char *name1, char *name2, char *new_value1, char *new_value2);
extern int sdrangel_set_ptt (RIG * rig, vfo_t vfo, ptt_t ptt);
extern int sdrangel_get_ptt(RIG * rig, vfo_t vfo, ptt_t *ptt);
extern int sdrangel_set_freq (RIG * rig, vfo_t vfo, freq_t freq);
extern int sdrangel_set_vfo_freq (RIG * rig, vfo_t vfo, freq_t freq); 
extern int sdrangel_set_rit(RIG *rig, vfo_t vfo, shortfreq_t rit); 
extern int sdrangel_get_rit(RIG *rig, vfo_t vfo, shortfreq_t *rit); 
extern int sdrangel_get_xit(RIG *rig, vfo_t vfo, shortfreq_t *xit); 
extern int sdrangel_set_xit(RIG *rig, vfo_t vfo, shortfreq_t xit); 
extern int sdrangel_get_mode (RIG * rig, vfo_t vfo, rmode_t * mode, pbwidth_t * width);
extern int sdrangel_set_mode (RIG * rig, vfo_t vfo, rmode_t mode, pbwidth_t width);
extern int sdrangel_get_vfo (RIG * rig, vfo_t * vfo);
extern int sdrangel_set_vfo(RIG *rig,vfo_t vfo);
extern int sdrangel_get_freq (RIG * rig, vfo_t vfo, freq_t * freq);
extern int sdrangel_set_split_vfo (RIG * rig, vfo_t vfo, split_t split, vfo_t tx_vfo);
extern int sdrangel_get_split_vfo (RIG * rig, vfo_t vfo, split_t *split, vfo_t *tx_vfo);
extern int sdrangel_set_split_freq (RIG * rig, vfo_t vfo, freq_t freq);
extern int sdrangel_get_split_freq (RIG * rig, vfo_t vfo, freq_t *freq);
extern int sdrangel_set_split_mode(RIG *rig, vfo_t vfo, rmode_t tx_mode, pbwidth_t tx_width);
extern int sdrangel_get_split_mode(RIG *rig, vfo_t vfo, rmode_t *tx_mode, pbwidth_t *tx_width);
extern int sdrangel_set_split_freq_mode(RIG *rig, vfo_t vfo, freq_t freq, rmode_t mode, pbwidth_t width);
extern int sdrangel_get_split_freq_mode(RIG *rig, vfo_t vfo, freq_t *freq, rmode_t *mode, pbwidth_t *width);
extern int sdrangel_get_vfo_info(RIG *rig, vfo_t vfo, freq_t *freq, rmode_t *mode, pbwidth_t *width, split_t *split);
extern int HTTP_action_PATCH(char *URL, char *buffer);
extern int HTTP_action_POST(char *URL, char *buffer);
extern int HTTP_action_DELETE(char *buffer);
extern int HTTP_action_GET(char *buffer);
extern int locate_position(char *p_buf, char *name);
extern int sdrangel_set_device_Parameter(RIG * rig, vfo_t vfo, char *p_parameter_name, char *p_value);
extern int ptt_relay(unsigned short relay_number, int state);
extern void  zero_split_mode_status();               // Not Split=0   Split=1
extern void  zero_current_TX_VFO();                  // VFO_A

const struct rig_caps sdrangel_pluto_caps = {
  RIG_MODEL (RIG_MODEL_SDRANGEL_PLUTO),
  .model_name = "sdrangel_pluto",
  .mfg_name = "Open Source",
  .version = "20210323.1",
  .copyright = "RS",
  .status = RIG_STATUS_BETA,
  .rig_type = RIG_TYPE_TRANSCEIVER,
  .ptt_type = RIG_PTT_RIG,
  .port_type = RIG_PORT_NETWORK,
  .ip_address_port = "127.0.0.1:8091",
  .rig_init = sdrangel_pluto_init,
  .post_write_delay = 1,
  .timeout = 1000,

  .retry = 3,

    .rx_range_list1 =   { // USA Version -- United States
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(144), MHz(148), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "USA"},
#endif
        {MHz(430), MHz(450), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(1240), MHz(1300), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(2300), MHz(2310), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(2390), MHz(2450), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(3300), MHz(3500), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(325), MHz(3800), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "USA"},
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(5650), MHz(5650), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(70), MHz(6000), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "USA"},
#endif
        RIG_FRNG_END,
    },
    .tx_range_list1 =  {
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(144), MHz(148), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(144), MHz(148), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "USA"},
#endif
        {MHz(430), MHz(450), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(1240), MHz(1300), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(2300), MHz(2310), sdrangel_pluto_ALL_TX_MODES, -1, -1, sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(2390), MHz(2450), sdrangel_pluto_ALL_TX_MODES, -1, -1, sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(3300), MHz(3500), sdrangel_pluto_ALL_TX_MODES, -1, -1, sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "USA"},
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(5650), MHz(5650), sdrangel_pluto_ALL_TX_MODES, -1, -1, sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(430), MHz(450), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "USA"},
        {MHz(1240), MHz(1300), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "USA"},
#endif
        RIG_FRNG_END,
    },


    .rx_range_list2 =   { // EUR Version -- Europe
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(144), MHz(146), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "EUR"},
#endif
        {MHz(400), MHz(470), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_1, "EUR"},
        {MHz(430), MHz(440), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "EUR"},
        {MHz(1240), MHz(1300), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "EUR"},
        RIG_FRNG_END,
    },
    .tx_range_list2 =   {
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(144), MHz(146), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "EUR"},
#endif
        {MHz(430), MHz(440), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "EUR"},
        {MHz(1240), MHz(1300), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "EUR"},
        {MHz(144), MHz(146), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "EUR"},
        {MHz(430), MHz(440), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "EUR"},
        {MHz(1240), MHz(1300), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "EUR"},
        RIG_FRNG_END,
    },

    .rx_range_list3 =   { // ITR Version -- Italy
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(144), MHz(146), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "ITR"},
#endif
        {MHz(430), MHz(434), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(435), MHz(438), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(1240), MHz(1245), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(1270), MHz(1298), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "ITR"},
        RIG_FRNG_END,
    },
    .tx_range_list3 =  {
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(144), MHz(146), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "ITR"},
#endif
        {MHz(430), MHz(434), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(435), MHz(438), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(1240), MHz(1245), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(1270), MHz(1298), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(144), MHz(146), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(430), MHz(434), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(435), MHz(438), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(1240), MHz(1245), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "ITR"},
        {MHz(1270), MHz(1298), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "ITR"},
        RIG_FRNG_END,
    },

    .rx_range_list4 =   { // TPE Version -- Taiwan ??
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(144), MHz(146), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "TPE"},
#endif
        {MHz(430), MHz(432), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "TPE"},
        {MHz(1260), MHz(1265), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "TPE"},
        RIG_FRNG_END,
    },
    .tx_range_list4 =  {
#ifndef PLUTO_EXTENDED_FREQUENCY
       {MHz(144), MHz(146), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "TPE"},
#endif
        {MHz(430), MHz(432), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "TPE"},
        {MHz(1260), MHz(1265), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "TPE"},
        {MHz(144), MHz(146), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "TPE"},
        {MHz(430), MHz(432), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "TPE"},
        {MHz(1260), MHz(1265), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "TPE"},
        RIG_FRNG_END,
    },

    .rx_range_list5 =   { // KOR Version -- Republic of Korea
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(144), MHz(146), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "KOR"},
#endif
        {MHz(430), MHz(440), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "KOR"},
        {MHz(1260), MHz(1300), sdrangel_pluto_ALL_RX_MODES, -1, -1, sdrangel_pluto_RX_VFOS, RIG_ANT_CURR, "KOR"},
        RIG_FRNG_END,
    },
    .tx_range_list5 =  {
#ifndef PLUTO_EXTENDED_FREQUENCY
        {MHz(144), MHz(146), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "KOR"},
#endif
        {MHz(430), MHz(440), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.5), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "KOR"},
        {MHz(1260), MHz(1300), sdrangel_pluto_ALL_TX_MODES ^ RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "KOR"},
        {MHz(144), MHz(146), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "KOR"},
        {MHz(430), MHz(440), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "KOR"},
        {MHz(1260), MHz(1300), RIG_MODE_AM, W(0.1), W(1), sdrangel_pluto_TX_VFOS, RIG_ANT_CURR, "KOR"},
        RIG_FRNG_END,
    },

	.tuning_steps = {{sdrangel_pluto_ALL_RX_MODES, Hz (1)}, RIG_TS_END,},

	.rig_open = sdrangel_open,
	.get_freq = sdrangel_get_freq,
	.set_freq = sdrangel_set_freq,
        .set_rit = sdrangel_set_rit,
        .get_rit = sdrangel_get_rit,
        .set_xit = sdrangel_set_xit,
        .get_xit = sdrangel_get_xit,
        .set_ptt = sdrangel_set_ptt,
        .get_ptt = sdrangel_get_ptt,
        .get_vfo = sdrangel_get_vfo,
        .set_vfo = sdrangel_set_vfo,
        .set_split_vfo = sdrangel_set_split_vfo,
        .get_split_vfo = sdrangel_get_split_vfo,
        .set_split_freq = sdrangel_set_split_freq,
        .get_split_freq = sdrangel_get_split_freq,
        .set_split_freq_mode = sdrangel_set_split_freq_mode,
        .get_split_freq_mode = sdrangel_get_split_freq_mode,
        .set_split_mode = sdrangel_set_split_mode,
        .get_split_mode = sdrangel_get_split_mode,
        .set_mode = sdrangel_set_mode,
        .get_mode = sdrangel_get_mode,
	.reset = sdrangel_pluto_reset,
//	.rig_close = sdrangel_close,
	.rig_cleanup = sdrangel_cleanup
}; 

#endif   /* _SDRAngel_pluto_H */


