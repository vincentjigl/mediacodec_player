#ifndef _AV_PLAYER_H_
#define _AV_PLAYER_H_
#include <unistd.h>
#include <pthread.h>

#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <media/ICrypto.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/DataSource.h>
#include <media/MediaCodecBuffer.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaCodecList.h>
#include <media/stagefright/MediaDefs.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/Surface.h>
#include <ui/DisplayInfo.h>
//#include <media/stagefright/NativeWindowWrapper.h>

#include <media/AudioTrack.h>
#include <time.h>

#define FRAME_SIZE 32768
#define FRAME_COUNT 8

struct audio_frame
{
    unsigned char* data;
    int len;
};

using namespace android;

class AVPlayer
{
public:
	AVPlayer() {
		mVideoFrameCount = 0;
		mBeginTime = 0;
    }
	
	int InitVideo();
	int FeedOneH264Frame(unsigned char* frame, int size);
	void MakeBackground();

	sp<MediaCodec> mCodec;
	Vector<sp<MediaCodecBuffer> > mInBuffers;
	Vector<sp<MediaCodecBuffer> > mOutBuffers;
	sp<SurfaceComposerClient> mComposerClient;
    sp<SurfaceControl> mControl;
	sp<SurfaceControl> mControlBG;
    sp<Surface> mSurface;
	sp<android::ALooper> mLooper;
	sp<AMessage> mFormat;
	
	int mWidth;
	int mHeight;
	bool mRendering;
	
	void CheckIfFormatChange();
	int RenderOneFrame();
	void RenderFrames();
	static void* VideoRenderThread(void* arg);
	
	void Dispose();
	
private:
	
	void* mPrivate;
	int mVideoFrameCount;
	clock_t mBeginTime;
};
#endif
