#include <stdio.h>
#include <android/native_window.h>

#include "avplayer.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

int AVPlayer::InitVideo()
{
	mWidth = SCREEN_WIDTH;
	mHeight = SCREEN_HEIGHT;
	
	mRendering = true;
	
	ProcessState::self()->startThreadPool();

    //DataSource::RegisterDefaultSniffers();
	
	mFormat = new AMessage;
	
	mLooper = new android::ALooper;
	mLooper->start();
	
    mComposerClient = new SurfaceComposerClient;
    CHECK_EQ(mComposerClient->initCheck(), (status_t)OK);
	
	mControl = mComposerClient->createSurface(
		String8("A Surface"), mWidth, mHeight, PIXEL_FORMAT_RGB_565, 0);
		
	CHECK(mControl != NULL);
    CHECK(mControl->isValid());

    SurfaceComposerClient::Transaction{}
        .setLayer(mControl, INT_MAX)
        .show(mControl)
        .apply();
	
    mSurface = mControl->getSurface();
    CHECK(mSurface != NULL);
	
	MakeBackground();
	
	mCodec = MediaCodec::CreateByType(
                mLooper, "video/avc", false);
    
    sp<AMessage> format = new AMessage;
    format->setString("mime", "video/avc");
    format->setInt32("width", mWidth);
    format->setInt32("height", mHeight);

    mCodec->configure(format, mSurface, NULL, 0);
				
    mCodec->start();
	
	int err = mCodec->getInputBuffers(&mInBuffers);
    CHECK_EQ(err, (status_t)OK);
    
    err = mCodec->getOutputBuffers(&mOutBuffers);
    CHECK_EQ(err, (status_t)OK);

	pthread_t tid;
    pthread_create(&tid, NULL, VideoRenderThread, this);	
	
    return 0;
}

void AVPlayer::MakeBackground()
{
	mControlBG = mComposerClient->createSurface(
		String8("A Surface"), mWidth, mHeight, PIXEL_FORMAT_RGB_565, 0);
	
	CHECK(mControlBG != NULL);
	CHECK(mControlBG->isValid());

	SurfaceComposerClient::Transaction{}
		.setLayer(mControlBG, INT_MAX -1)
		.show(mControlBG)
		.apply();

	sp<Surface> service = mControlBG->getSurface();
	
	ANativeWindow_Buffer ob;
    service->lock(&ob, NULL);
    service->unlockAndPost();
}

void AVPlayer::CheckIfFormatChange()
{
	mCodec->getOutputFormat(&mFormat);
		
	int width, height;
	if (mFormat->findInt32("width", &width) &&
		mFormat->findInt32("height", &height)) {
		float scale_x = (SCREEN_WIDTH + 0.0) / width;
		float scale_y = (SCREEN_HEIGHT + 0.0) / height;
		float scale = (scale_x < scale_y) ? scale_x : scale_y;
		
		scale = (scale > 1) ? 1 : scale;
		
		if (scale < 1) {
			int new_width = width * scale;
			int new_height = height * scale;
			
			new_width = (new_width > SCREEN_WIDTH) ? SCREEN_WIDTH : new_width;
			new_height = (new_height > SCREEN_HEIGHT) ? SCREEN_HEIGHT : new_height;
			
			width = new_width;
			height = new_height;
		}
		
		if (width > SCREEN_WIDTH)
			width = SCREEN_WIDTH;
		
		if (height > SCREEN_HEIGHT)
			height = SCREEN_HEIGHT;
		
		if (width != mWidth || height != mHeight) {
			mWidth = width;
			mHeight = height;
			
			int x = (SCREEN_WIDTH - width) / 2;
			int y = (SCREEN_HEIGHT - height) / 2;

			SurfaceComposerClient::Transaction{}
					.setPosition(mControl, width, height)
					.setSize(mControl, x, y);
		}
	}	
}

void AVPlayer::RenderFrames()
{
	size_t index, offset, size;
	int64_t pts;
	uint32_t flags;
	
	int err;
	
	do {
		CheckIfFormatChange();
		
		err = mCodec->dequeueOutputBuffer(
			&index, &offset, &size, &pts, &flags);

		if (err == OK) {
			mCodec->renderOutputBufferAndRelease(index);
			
			mVideoFrameCount++;
			if (mBeginTime == 0) {
				mBeginTime = clock();
			} else {
				float fps = mVideoFrameCount / (float(clock() - mBeginTime) / CLOCKS_PER_SEC);
				printf("### %f\n", fps);
			}
		}
	} while(err == OK
                || err == INFO_FORMAT_CHANGED
                || err == INFO_OUTPUT_BUFFERS_CHANGED);
}

void* AVPlayer::VideoRenderThread(void* arg)
{
	AVPlayer* player = (AVPlayer*)arg;

    while(player->mRendering) {
        player->RenderFrames();
    }
    
    return NULL;	
}

int AVPlayer::FeedOneH264Frame(unsigned char* frame, int size)
{
	size_t index;

	int err = mCodec->dequeueInputBuffer(&index, -1ll);
	
	CHECK_EQ(err, (status_t)OK);
	
	const sp<MediaCodecBuffer> &buffer = mInBuffers.itemAt(index);
	sp<ABuffer> dst =new ABuffer(buffer->base(), buffer->capacity());
	//CHECK_LE(size, dst->capacity());
	
	dst->setRange(0, size);
	memcpy(dst->data(), frame, size);
	
	err = mCodec->queueInputBuffer(index, 0, size, 0ll, 0);
	return err;
}
	
void AVPlayer::Dispose()
{
	mCodec->stop();
	mCodec->reset();
	mCodec->release();
	mLooper->stop();
	
	
	mRendering = false;
	SurfaceComposerClient::Transaction{}.hide(mControl);
	SurfaceComposerClient::Transaction{}.hide(mControlBG);
	
	mComposerClient->dispose();
	mControl->release();
	mControlBG->release();


}
