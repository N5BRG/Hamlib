
/*
 *  Hamlib SDRANGEL backend - main header
 *  Copyright (c) 2004-2012 by Stephane Fillod
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

#ifndef _SDRANGEL_H
#define _SDRANGEL_H 1

#include "hamlib/rig.h"
#include <curl/curl.h>

#define MAX_DEVICES 2
#define MAX_CHANNELS 2
#define HUGEBUFSIZE 4000
#define READBUFSIZE 2000
#define CHANNELBUFSIZE 1200
#define MIDBUFSIZE 900
#define sdrangel_TX_VFOS (RIG_VFO_A|RIG_VFO_B|RIG_VFO_MAIN|RIG_VFO_MAIN_A|RIG_VFO_MAIN_B)
#define sdrangel_RX_VFOS (RIG_VFO_A|RIG_VFO_MAIN|RIG_VFO_MAIN_A)
#define sdrangel_ALL_TX_MODES (RIG_MODE_FM|RIG_MODE_AM|RIG_MODE_CW|RIG_MODE_SSB)
#define sdrangel_ALL_RX_MODES (RIG_MODE_FM|RIG_MODE_AM|RIG_MODE_CW|RIG_MODE_SSB)
//Audio Input devices for modAFInput setting
#define MIC 3
#define KEY 4
#define TONE 1

typedef struct MemoryStruct{
  char *memory;
  size_t size;
} *p_MemoryStruct;

extern const struct rig_caps sdrangel_pluto_caps;
extern const struct rig_caps sdrangel_hackrf_caps;
extern const struct rig_caps sdrangel_rtlsdr_caps;

/* Function Prototypes  */
int sdrangel_close(RIG *rig);
int sdrangel_cleanup(RIG *rig);
int set_modAFInput(int audio_input);
int set_modAFInput(int audio_input);
void extract_value(char *hunt_value, int buf_cnt, char *buffer);
int sdrangel_open (RIG * rig);
int sdrangel_set_ptt(RIG *rig, vfo_t vfo, ptt_t ptt);
char *replace_value(char *p_buf, char *name, char *new_value);
char *replace_value2(char *p_buf, char *name1, char *name2, char *new_value1, char *new_value2);
int sdrangel_set_ptt (RIG * rig, vfo_t vfo, ptt_t ptt);
extern int sdrangel_get_ptt(RIG * rig, vfo_t vfo, ptt_t *ptt);
int sdrangel_set_freq (RIG * rig, vfo_t vfo, freq_t freq);
int sdrangel_set_vfo_freq (RIG * rig, vfo_t vfo, freq_t freq);
int sdrangel_set_rit(RIG *rig, vfo_t vfo, shortfreq_t rit);
int sdrangel_get_rit(RIG *rig, vfo_t vfo, shortfreq_t *rit);
int sdrangel_get_xit(RIG *rig, vfo_t vfo, shortfreq_t *xit);
int sdrangel_set_xit(RIG *rig, vfo_t vfo, shortfreq_t xit);
int sdrangel_get_mode(RIG * rig, vfo_t vfo, rmode_t * mode, pbwidth_t * width);
int sdrangel_set_mode(RIG * rig, vfo_t vfo, rmode_t mode, pbwidth_t width);
int sdrangel_get_vfo(RIG * rig, vfo_t * vfo);
int sdrangel_set_vfo(RIG *rig,vfo_t vfo);
int sdrangel_get_freq(RIG * rig, vfo_t vfo, freq_t * freq);
int sdrangel_set_split_vfo(RIG * rig, vfo_t vfo, split_t split, vfo_t tx_vfo);

int HTTP_action_PATCH(char *URL, char *buffer);
int HTTP_action_POST(char *URL, char *buffer);
int HTTP_action_DELETE(char *buffer);
int HTTP_action_GET(char *buffer);
int locate_position(char *p_buf, char *name);
void zero_split_mode_status();               // Not Split=0   Split=1
void zero_current_TX_VFO();                  // VFO_A
extern int ptt_relay(unsigned short relay_number, int state); 
/*
extern int set_freq_hackrf (RIG * rig, vfo_t vfo, freq_t freq);
extern int get_freq_hackrf (RIG * rig, vfo_t vfo, freq_t * freq);
extern int set_split_freq_hackrf (RIG * rig, vfo_t vfo, freq_t freq);
extern int get_split_freq_hackrf (RIG * rig, vfo_t vfo, freq_t *freq);
extern int set_split_freq_mode_hackrf(RIG *rig, vfo_t vfo, freq_t freq, rmode_t mode, pbwidth_t width);
extern int get_split_freq_mode_hackrf(RIG *rig, vfo_t vfo, freq_t *freq, rmode_t *mode, pbwidth_t *width);

*/

#endif /* _SDRANGEL_H */
