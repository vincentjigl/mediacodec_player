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
#include <utils/RefBase.h>

#include <time.h>

#define FRAME_SIZE 32768
#define FRAME_COUNT 8
#define MAX_BUFFER_SIZE (1024 * 1024)
#define BULK_SIZE 32768


using namespace android;
class VideoBuffer
{
public:
	VideoBuffer() {
	}
	
	void SetBuffer(unsigned char* buffer) {
		mBuffer = buffer;
		mTotalLength = 0;
	}
	
	void AppendBuffer(unsigned char* buffer, int len) {
		memcpy(mBuffer + mTotalLength, buffer, len);
		mTotalLength += len;
	}
	
	void DisposeOneFrame(int len) {
		memmove(mBuffer, mBuffer + len, mTotalLength - len);
		mTotalLength -= len;
	}
	
	int SearchStartCode() {
		int count = 0;
		for (int i = 4; i < mTotalLength; i++) {
			switch(count) {
				case 0:
				case 1:
				case 2:
					if (mBuffer[i] == 0) {
						count++;
					} else {
						count = 0;
					}
				    break;
				case 3:
					if (mBuffer[i] == 1) {
						return i - 3;
					} else {
						count = 0;
					}
			}
		}
		
		return 0;
	}
	
	unsigned char* GetBuffer() {
		return mBuffer;
	}

private:
	unsigned char* mBuffer;
	int mTotalLength;
};

class AVPlayer: public RefBase
{
public:
	AVPlayer()
    {

    }

    AVPlayer(int w, int h, int x, int y, const char* path);     
	~AVPlayer(){
		Dispose();

	}
	
	int InitVideo();
    void Start();
	
private:
    
    int FeedOneH264Frame(unsigned char* frame, int size);
	void MakeBackground();
    void RenderFrames();
    static void* VideoRenderThread(void* arg);
    void CheckIfFormatChange();
    int RenderOneFrame();
    void Dispose();

private:
	sp<MediaCodec> mCodec;
	Vector<sp<MediaCodecBuffer> > mInBuffers;
	Vector<sp<MediaCodecBuffer> > mOutBuffers;
	sp<SurfaceComposerClient> mComposerClient;
    sp<SurfaceControl> mControl;
	sp<SurfaceControl> mControlBG;
    sp<Surface> mSurface;
	sp<android::ALooper> mLooper;
	sp<AMessage> mFormat;

    int offsetx;
    int offsety;
	int mWidth;
	int mHeight;
	bool mRendering;

	int mVideoFrameCount;
	clock_t mBeginTime;
    VideoBuffer* buf;
    FILE* video_fp;
    const char* filePath;
};
#endif
