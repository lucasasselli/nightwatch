#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

#include "gfx.h"

static int update(void* userdata);
const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* font = NULL;

int init = 0;
float r = 0;

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

    if(!init){
        pd->system->logToConsole("Begin init.");
        load_obj_file(pd, "res/models/cube.obj");
        init = 1;
    }

	if ( event == kEventInit )
	{
		const char* err;
		font = pd->graphics->loadFont(fontpath, &err);
		
		if ( font == NULL )
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		// Note: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
		pd->system->setUpdateCallback(update, pd);
	}
	
	return 0;
}

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	
	pd->graphics->clear(kColorWhite);
	// pd->graphics->setFont(font);
	// pd->graphics->drawText("Hello World!", strlen("Hello World!"), kASCIIEncoding, x, y);

    memcpy(v_buff, o_buff, BUFFER_SIZE * sizeof(vec4_t));

    for (int i = 0; i < v_buff_cnt; i++)
    {
        vertex_scale(&v_buff[i], 1);
        vertex_move(&v_buff[i], 0, 0, -7, r);
    }

    convert_to_ndc(M_PI / 3, 0.6f, 1, 20);
    convert_to_raster();

    raster();
    gfx_draw(pd);
    r += M_PI / 100;
        
	pd->system->drawFPS(0,0);

	return 1;
}
