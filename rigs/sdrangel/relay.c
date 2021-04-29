#include <stdio.h>
#include <stdlib.h>
#include <ftdi.h>
#include "hamlib/rig.h"

#define MAX_NUM_RELAYS 4

	static struct ftdi_context *ftdi;
	static int g_num_relays=MAX_NUM_RELAYS;


int ptt_relay(unsigned short relay_number, int state) {
	unsigned char buf[1];
	int ret=0;
	unsigned int chipid;
	char port[50]=" ";
	char *portname=&port[0];
	int number=1;
	int *num_relays=&number;
	int data=0;
	int *relay_data=&data;

	if ((ftdi = ftdi_new()) == 0)
	{
		printf("ftdi_new failed\n");
		return EXIT_FAILURE;
	}

	/* Open FTDI USB device */
	ret=ftdi_usb_open(ftdi, 0x0403, 0x6001);
	if (ret < 0)
	{
		rig_debug(RIG_DEBUG_ERR, "34  %s: Failed to open FTDI device, SainSmart relay board!\n", __func__);
		ftdi_free(ftdi);
		return -2;
	}
/*
	ftdi_set_interface(ftdi, INTERFACE_A);  // Or try INTERFACE_B
	f = ftdi_usb_open(ftdi, 0x0403, 0x6001);
	if (f < 0 && f != -5)
	{
		fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(ftdi));
		ftdi_free(ftdi);
		exit(-1);
	}
*/
	// Put ftdi into Bit Bang Mode
	if(ftdi_set_bitmode(ftdi, 0xFF, BITMODE_BITBANG) < 0) 
	{
		rig_debug(RIG_DEBUG_ERR, "50  %s: Failed to open FTDI device in bitbang mode!\n", __func__);
		ftdi_free(ftdi);
		return -1;
	}


	/* Check if this is an R type chip
	* Type 245RL = 5000
	*/
	//printf("relay type:%d\n",ftdi->type);
	if (ftdi->type != 5000 && ftdi->type != TYPE_R )
	{
		rig_debug(RIG_DEBUG_ERR, "62  %s: Wrong tyoe of chip used for FTDI on relay board!\n", __func__);
		ftdi_free(ftdi);
		return -1;
	}

	/* Read out FTDI Chip-ID of R type chips */
	ftdi_read_chipid(ftdi, &chipid);
	/* Return parameters */
	if (num_relays!=NULL) * num_relays = g_num_relays;
	sprintf(portname, "FTDI chipid %X", chipid);
	//printf("DBG: portname %s\n", portname);



	/* Get relay state from the card */
	if (ftdi_read_pins(ftdi, buf) < 0)
	{
		rig_debug(RIG_DEBUG_ERR, "79  %s: Problem getting the current state of relay board!\n", __func__);
		return -1;
	}
	*relay_data = buf[0];


	/* Set the new relay state bit */
	relay_number = relay_number-1;

	if (state == 0)
	{
		/* Clear the relay bit in mask */
		buf[0] &= ~(1 << relay_number);
	}
	else
	{
		/* Set the relay bit in mask */
		buf[0] |= 0xf0 | (1 << relay_number);
	}
  
	//printf("DBG: Writing GPIO bits %02X\n", buf[0]);
  
	/* Set relay on the card */

	if (ftdi_write_data(ftdi,buf, 1) < 0)
	{
		rig_debug(RIG_DEBUG_ERR, "105  %s: Problem writing data to FTDI chip on relay board!\n", __func__);
		ftdi_free(ftdi);
		return -1;
	}

	/* Get relay state from the card */
	if (ftdi_read_pins(ftdi, &buf[0]) < 0)
	{
		rig_debug(RIG_DEBUG_ERR, "113  %s: Problem getting the current state of relay board!\n", __func__);
		return -3;
	}

	ftdi_usb_close(ftdi);

	ftdi_free(ftdi);

	return 0;

}


