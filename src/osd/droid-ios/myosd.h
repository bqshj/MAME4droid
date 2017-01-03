//============================================================
//
//  myosd.c - Implementation of osd stuff
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  MAME4DROID MAME4iOS by David Valdeita (Seleuco)
//
//============================================================

#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>

#ifndef __MYOSD_H__
#define __MYOSD_H__

#if defined(__cplusplus)
extern "C" {
#endif

enum  { MYOSD_UP=0x1,       MYOSD_LEFT=0x4,       MYOSD_DOWN=0x10,  MYOSD_RIGHT=0x40,
        MYOSD_START=1<<8,   MYOSD_SELECT=1<<9,    MYOSD_L1=1<<10,    MYOSD_R1=1<<11,
        MYOSD_A=1<<12,      MYOSD_B=1<<13,        MYOSD_X=1<<14,    MYOSD_Y=1<<15,
        MYOSD_VOL_UP=1<<23, MYOSD_VOL_DOWN=1<<22, MYOSD_PUSH=1<<27, MYOSD_ESC=1<<28 };

extern unsigned short *myosd_screen15;
extern int  myosd_fps;
extern int  myosd_showinfo;
extern int  myosd_sleep;
extern int  myosd_inGame;
extern int  myosd_exitGame;
extern int  myosd_exitPause;
extern int  myosd_autosave;
extern int  myosd_cheat;
extern int  myosd_sound_value;
extern int  myosd_frameskip_value;
extern int  myosd_throttle;
extern int  myosd_savestate;
extern int  myosd_loadstate;
extern int  myosd_waysStick;
extern int  myosd_video_width;
extern int  myosd_video_height;
extern int  myosd_vis_video_width;
extern int  myosd_vis_video_height;
extern int  myosd_in_menu;
extern int  myosd_res;
extern int  myosd_force_pxaspect;
extern int  myosd_num_of_joys;
extern int  myosd_video_threaded;
extern int  myosd_service;

extern unsigned long myosd_pad_status;
extern int myosd_last_game_selected;

extern void myosd_init(void);
extern void myosd_deinit(void);
extern void myosd_video_flip(void);
extern unsigned long myosd_joystick_read(int n);
extern float myosd_joystick_read_analog(int n, char axis);
extern void myosd_set_video_mode(int width,int height,int vis_width, int vis_height);
extern void myosd_closeSound(void);
extern void myosd_openSound(int rate,int stereo);
extern void myosd_sound_play(void *buff, int len);
extern void myosd_check_pause(void);

#if defined(__cplusplus)
}
#endif

#endif
