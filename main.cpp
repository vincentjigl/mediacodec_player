#include <stdio.h>
#include "avplayer.h"



int main()
{
    const sp<IBinder> display = SurfaceComposerClient::getInternalDisplayToken();
    CHECK(display != nullptr);
    
    DisplayInfo info;
    CHECK_EQ(SurfaceComposerClient::getDisplayInfo(display, &info), NO_ERROR);
    int width = info.w;
    int height = info.h;

	const char* path = "/data/test.264";
	
	sp<AVPlayer> avplayer = new AVPlayer(width/3, height/3, 0, 0, path);
	avplayer->InitVideo();
    avplayer->Start();
	
	return 0;	
}
