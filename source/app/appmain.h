


#ifndef appmain_H
#define appmain_H

#include "../window.h"

#define VERSION				1
#define TITLE				"Bw"

extern int g_appmode;

#define APPMODE_LOADING	0
#define APPMODE_RELOAD	1
#define APPMODE_PLAY	2
#define APPMODE_MENU	3

int32_t HandleEvent(void *userdata, SDL_Event *e);

#endif
