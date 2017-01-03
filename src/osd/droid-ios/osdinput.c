//============================================================
//
//  droidinput.c - Implementation of MAME input routines
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  MAME4DROID MAME4iOS by David Valdeita (Seleuco)
//
//============================================================

#ifdef ANDROID
#include <android/log.h>
#endif

#include "osdepend.h"
#include "emu.h"
#include "uimenu.h"

#include "myosd.h"

enum
{
	KEY_ESCAPE,
	KEY_1,
	KEY_2,
	KEY_LOAD,
	KEY_SAVE,
	KEY_PGUP,
	KEY_PGDN,
	KEY_SERVICE,
	KEY_TOTAL
};

enum
{
	STATE_NORMAL,
	STATE_LOADSAVE
};

static int mystate = STATE_NORMAL;

// a single input device
static input_device *keyboard_device;

// the states
static int keyboard_state[KEY_TOTAL];
static int joy_buttons[4][8];
static int joy_axis[4][2];
static int joy_hats[4][4];

static INT32 my_get_state(void *device_internal, void *item_internal);
static INT32 my_axis_get_state(void *device_internal, void *item_internal);

void droid_ios_init_input(running_machine *machine)
{

	memset(keyboard_state,0,sizeof(keyboard_state));
	memset(joy_buttons,0,sizeof(joy_buttons));
	memset(joy_axis,0,sizeof(joy_axis));
	memset(joy_hats,0,sizeof(joy_hats));

	//input_device_class_enable(machine, DEVICE_CLASS_LIGHTGUN, TRUE);
	input_device_class_enable(machine, DEVICE_CLASS_JOYSTICK, TRUE);

	for (int i = 0; i < 4; i++)
	{
		char name[10];
		input_device *devinfo;

		snprintf(name, 10, "Joy %d", i + 1);
		devinfo = input_device_add(machine, DEVICE_CLASS_JOYSTICK, name, NULL);
		input_device_item_add(devinfo, "Bt_B", &joy_buttons[i][0], ITEM_ID_BUTTON1, my_get_state);
		input_device_item_add(devinfo, "Bt_X", &joy_buttons[i][1], ITEM_ID_BUTTON2, my_get_state);
		input_device_item_add(devinfo, "Bt_A", &joy_buttons[i][2], ITEM_ID_BUTTON3, my_get_state);
		input_device_item_add(devinfo, "Bt_Y", &joy_buttons[i][3], ITEM_ID_BUTTON4, my_get_state);
		input_device_item_add(devinfo, "Bt_L", &joy_buttons[i][4], ITEM_ID_BUTTON5, my_get_state);
		input_device_item_add(devinfo, "Bt_R", &joy_buttons[i][5], ITEM_ID_BUTTON6, my_get_state);

		input_device_item_add(devinfo, "Coin", &joy_buttons[i][6], ITEM_ID_BUTTON7, my_get_state);
		input_device_item_add(devinfo, "Start", &joy_buttons[i][7], ITEM_ID_BUTTON8, my_get_state);

		input_device_item_add(devinfo, "X Axis", &joy_axis[i][0], ITEM_ID_XAXIS, my_axis_get_state);
		input_device_item_add(devinfo, "Y Axis", &joy_axis[i][1], ITEM_ID_YAXIS, my_axis_get_state);

		input_device_item_add(devinfo, "D-Pad Up", &joy_hats[i][0],(input_item_id)( ITEM_ID_HAT1UP+i*4), my_get_state);
		input_device_item_add(devinfo, "D-Pad Down", &joy_hats[i][1],(input_item_id)( ITEM_ID_HAT1DOWN+i*4), my_get_state);
		input_device_item_add(devinfo, "D-Pad Left", &joy_hats[i][2], (input_item_id)(ITEM_ID_HAT1LEFT+i*4), my_get_state);
		input_device_item_add(devinfo, "D-Pad Right", &joy_hats[i][3], (input_item_id)(ITEM_ID_HAT1RIGHT+i*4), my_get_state);

	}

	keyboard_device = input_device_add(machine, DEVICE_CLASS_KEYBOARD, "Keyboard", NULL);
	if (keyboard_device == NULL)
		fatalerror("Error creating keyboard device");

	input_device_item_add(keyboard_device, "Exit", &keyboard_state[KEY_ESCAPE], ITEM_ID_ESC, my_get_state);

	input_device_item_add(keyboard_device, "1", &keyboard_state[KEY_1], ITEM_ID_1, my_get_state);
	input_device_item_add(keyboard_device, "2", &keyboard_state[KEY_2], ITEM_ID_2, my_get_state);

	input_device_item_add(keyboard_device, "Load", &keyboard_state[KEY_LOAD], ITEM_ID_F7, my_get_state);
	input_device_item_add(keyboard_device, "Save", &keyboard_state[KEY_SAVE], ITEM_ID_F8, my_get_state);

	input_device_item_add(keyboard_device, "PGUP", &keyboard_state[KEY_PGUP], ITEM_ID_PGUP, my_get_state);
	input_device_item_add(keyboard_device, "PGDN", &keyboard_state[KEY_PGDN], ITEM_ID_PGDN, my_get_state);

	input_device_item_add(keyboard_device, "Service", &keyboard_state[KEY_SERVICE], ITEM_ID_F2, my_get_state);
}


void droid_ios_poll_input(running_machine *machine)
{

	long _pad_status = myosd_joystick_read(0);

	if(mystate == STATE_NORMAL)
	{

		keyboard_state[KEY_1] = 0;
		keyboard_state[KEY_2] = 0;

		if(myosd_exitGame)
		{
			keyboard_state[KEY_ESCAPE] = 0x80;
			myosd_exitGame = 0;
		}
	    else
	    {
			keyboard_state[KEY_ESCAPE] = 0;
        }

		if(myosd_service && !myosd_in_menu)
		{
			keyboard_state[KEY_SERVICE] = 0x80;
			myosd_service = 0;
		}
	    else
	    {
			keyboard_state[KEY_SERVICE] = 0;
        }

		keyboard_state[KEY_LOAD] =  0;
		keyboard_state[KEY_SAVE] =  0;

		if(myosd_savestate)
		{
			keyboard_state[KEY_SAVE] =  0x80;
			myosd_savestate = 0;
			mystate = STATE_LOADSAVE;
		}

		if(myosd_loadstate)
		{
			keyboard_state[KEY_LOAD] =  0x80;
			myosd_loadstate = 0;
			mystate = STATE_LOADSAVE;
		}

		for(int i=0; i<4; i++)
		{
			if(i!=0  && myosd_in_menu==1)
				break;

			_pad_status = myosd_joystick_read(i);

			if(i==0)
			{
				if(!myosd_inGame && !myosd_in_menu)
				{
					keyboard_state[KEY_PGUP] = ((_pad_status & MYOSD_LEFT) != 0) ? 0x80 : 0;
					keyboard_state[KEY_PGDN] = ((_pad_status & MYOSD_RIGHT) != 0) ? 0x80 : 0;
				}
				else
				{
					keyboard_state[KEY_PGUP] = 0;
					keyboard_state[KEY_PGDN] = 0;
				}
			}

			if(myosd_joystick_read_analog(0, 'x') == 0 && myosd_joystick_read_analog(0, 'y')==0)
			{
			   joy_hats[i][0] = ((_pad_status & MYOSD_UP) != 0) ? 0x80 : 0;
			   joy_hats[i][1] = ((_pad_status & MYOSD_DOWN) != 0) ? 0x80 : 0;
			   joy_hats[i][2] = ((_pad_status & MYOSD_LEFT) != 0) ? 0x80 : 0;
			   joy_hats[i][3] = ((_pad_status & MYOSD_RIGHT) != 0) ? 0x80 : 0;
			   joy_axis[i][0] = 0;
			   joy_axis[i][1] = 0;
			}
			else
			{
			   joy_axis[i][0] = (int)(myosd_joystick_read_analog(i, 'x') *  32767 *  2 );
			   joy_axis[i][1] = (int)(myosd_joystick_read_analog(i, 'y') *  32767 * -2 );
			   joy_hats[i][0] = 0;
			   joy_hats[i][1] = 0;
			   joy_hats[i][2] = 0;
			   joy_hats[i][3] = 0;
			}

			joy_buttons[i][0]  = ((_pad_status & MYOSD_B) != 0) ? 0x80 : 0;
			joy_buttons[i][1]  = ((_pad_status & MYOSD_X) != 0) ? 0x80 : 0;
			joy_buttons[i][2]  = ((_pad_status & MYOSD_A) != 0) ? 0x80 : 0;
			joy_buttons[i][3]  = ((_pad_status & MYOSD_Y) != 0) ? 0x80 : 0;
			joy_buttons[i][4]  = ((_pad_status & MYOSD_L1) != 0) ? 0x80 : 0;
			joy_buttons[i][5]  = ((_pad_status & MYOSD_R1) != 0) ? 0x80 : 0;

			if(i!=0  && (myosd_num_of_joys==1 || myosd_num_of_joys==0))
				continue;

			joy_buttons[i][6]  = ((_pad_status & MYOSD_SELECT ) != 0) ? 0x80 : 0;
			joy_buttons[i][7]  = ((_pad_status & MYOSD_START  ) != 0) ? 0x80 : 0;
		}
	}
	else if(mystate == STATE_LOADSAVE)
	{

		keyboard_state[KEY_ESCAPE] = 0;
		keyboard_state[KEY_1] = 0;
		keyboard_state[KEY_2] = 0;

		if(myosd_exitGame)
		{
			keyboard_state[KEY_ESCAPE] = 0x80;
			myosd_exitGame = 0;
			mystate = STATE_NORMAL;
		}

		if ((_pad_status & MYOSD_B) != 0)
		{
			keyboard_state[KEY_1] = 0x80;
			mystate = STATE_NORMAL;
		}

		if ((_pad_status & MYOSD_X) != 0)
		{
			keyboard_state[KEY_2] = 0x80;
			mystate = STATE_NORMAL;
		}

	}
	else {/*???*/}
}

//============================================================
//  osd_customize_inputport_list
//============================================================

void osd_customize_input_type_list(input_type_desc *typelist)
{
	input_type_desc *typedesc;

	for (typedesc = typelist; typedesc != NULL; typedesc = typedesc->next)
	{
		switch (typedesc->type)
		{
			case IPT_UI_UP:
			case IPT_UI_DOWN:
			case IPT_UI_LEFT:
			case IPT_UI_RIGHT:
			case IPT_UI_CANCEL:
			case IPT_UI_PAGE_UP:
			case IPT_UI_PAGE_DOWN:
				continue;
		}

		if(typedesc->type >= __ipt_ui_start && typedesc->type <= __ipt_ui_end)
			input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);

		if(typedesc->type >= IPT_MAHJONG_A && typedesc->type <= IPT_SLOT_STOP_ALL)
			input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);

	}

	for (typedesc = typelist; typedesc != NULL; typedesc = typedesc->next)
	{

		switch (typedesc->type)
		{
			case IPT_UI_CONFIGURE:
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0));
				break;
			case IPT_COIN1:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0));
				break;
			case IPT_START1:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0));
				break;
			case IPT_COIN2:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 1));
				break;
			case IPT_START2:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 1));
				break;
			case IPT_COIN3:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 2));
				break;
			case IPT_START3:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 2));
				break;
			case IPT_COIN4:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 3));
				break;
			case IPT_START4:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 3));
				break;
			case IPT_UI_LOAD_STATE:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F7);
				break;
			case IPT_UI_SAVE_STATE:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F8);
				break;
			case IPT_UI_SELECT:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON1, 0));
				break;
			case IPT_UI_UP:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 0));
				break;
			case IPT_UI_DOWN:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 0));
				break;
			case IPT_UI_LEFT:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1LEFT), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 0));
				break;
			case IPT_UI_RIGHT:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 0));
				break;
			case IPT_JOYSTICK_UP:
			case IPT_JOYSTICKLEFT_UP:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2UP),1), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3UP),2), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4UP),3), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 3));
				break;
			case IPT_JOYSTICK_DOWN:
			case IPT_JOYSTICKLEFT_DOWN:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2DOWN),1), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3DOWN),2), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4DOWN),3), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 3));
				break;
			case IPT_JOYSTICK_LEFT:
			case IPT_JOYSTICKLEFT_LEFT:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1LEFT), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2LEFT),1), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3LEFT),2), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4LEFT),3), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 3));
				break;
			case IPT_JOYSTICK_RIGHT:
			case IPT_JOYSTICKLEFT_RIGHT:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2RIGHT),1), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3RIGHT),2), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4RIGHT),3), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 3));
				break;
			case IPT_POSITIONAL:
				if(typedesc->group == IPG_PLAYER1)
				{
					input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON5, 0));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON6, 0));
				}
				else if(typedesc->group == IPG_PLAYER2)
				{
					input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON5, 1));
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON6, 1));
				}
				else if(typedesc->group == IPG_PLAYER3)
				{
					input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON5, 2));
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON6, 2));
				}
				else if(typedesc->group == IPG_PLAYER4)
				{
					input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON5, 3));
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON6, 3));
				}
				break;
			case IPT_PADDLE:
			case IPT_TRACKBALL_X:
			case IPT_AD_STICK_X:
			case IPT_LIGHTGUN_X:
			case IPT_DIAL:
				if(typedesc->group == IPG_PLAYER1)
				{
				   input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_X,0));
                   input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1LEFT),0));
				   input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT),0));
				}
				else if(typedesc->group == IPG_PLAYER2)
				{
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_X,1));
	                input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2LEFT),1));
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2RIGHT),1));
				}
				else if(typedesc->group == IPG_PLAYER3)
				{
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_X,2));
	                input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3LEFT),2));
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3RIGHT),2));
				}
				else if(typedesc->group == IPG_PLAYER4)
				{
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_X,3));
	                input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4LEFT),3));
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4RIGHT),3));
				}
				break;
			case IPT_PADDLE_V:
			case IPT_TRACKBALL_Y:
			case IPT_AD_STICK_Y:
			case IPT_LIGHTGUN_Y:
			case IPT_POSITIONAL_V:
			case IPT_DIAL_V:
				if(typedesc->group == IPG_PLAYER1)
				{
                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,0));
                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP),0));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN),0));
				}
				else if(typedesc->group == IPG_PLAYER2)
				{
                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,1));
                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2UP),1));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2DOWN),1));
				}
				else if(typedesc->group == IPG_PLAYER3)
				{
                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,2));
                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3UP),2));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3DOWN),2));
				}
				else if(typedesc->group == IPG_PLAYER4)
				{
                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,3));
                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4UP),3));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4DOWN),3));
				}
				break;
			case IPT_MOUSE_X:
				if(typedesc->group == IPG_PLAYER1)
				{
                   input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1LEFT),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 0));
				   input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 0));
				}
				else if(typedesc->group == IPG_PLAYER2)
				{

					input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2LEFT),1),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 1));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2RIGHT),1),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 1));
				}
				else if(typedesc->group == IPG_PLAYER3)
				{
	                input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3LEFT),2),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 2));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3RIGHT),2),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 2));
				}
				else if(typedesc->group == IPG_PLAYER4)
				{
	                input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4LEFT),3),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 3));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4RIGHT),3),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 3));
				}
				break;
			case IPT_MOUSE_Y:
				if(typedesc->group == IPG_PLAYER1)
				{
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 0));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 0));
				}
				else if(typedesc->group == IPG_PLAYER2)
				{
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2UP),1),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 1));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2DOWN),1),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 1));
				}
				else if(typedesc->group == IPG_PLAYER3)
				{
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3UP),2),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 2));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3DOWN),2),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 2));
				}
				else if(typedesc->group == IPG_PLAYER4)
				{
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4UP),3),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 3));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4DOWN),3),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 3));
				}
				break;
			case IPT_JOYSTICKRIGHT_UP:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON4, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON4, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON4, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON4, 3));
				break;
			case IPT_JOYSTICKRIGHT_DOWN:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON2, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON2, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON2, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON2, 3));
				break;
			case IPT_JOYSTICKRIGHT_LEFT:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON3, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON3, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON3, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON3, 3));
				break;
			case IPT_JOYSTICKRIGHT_RIGHT:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON1, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON1, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON1, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON1, 3));
				break;
			case IPT_AD_STICK_Z:
			case IPT_START:
			case IPT_SELECT:
				input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
				break;
			//default:
				//input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
		}
	}
}

static INT32 my_get_state(void *device_internal, void *item_internal)
{
	UINT8 *keystate = (UINT8 *)item_internal;
	return *keystate;
}

static INT32 my_axis_get_state(void *device_internal, void *item_internal)
{
	INT32 *axisdata = (INT32 *) item_internal;
	return *axisdata;
}

