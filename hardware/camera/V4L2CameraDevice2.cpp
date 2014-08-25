
#include "CameraDebug.h"
#include "hdr.h"
#if DBG_V4L2_CAMERA
#define LOG_NDEBUG 0
#endif
#define LOG_TAG "V4L2CameraDevice"
#include <cutils/log.h>

#include <sys/mman.h> 
#include <sys/time.h>
#include <videodev2.h>
#include <linux/videodev.h> 

#ifdef USE_MP_CONVERT
#include <g2d_driver.h>
#endif

#include "V4L2CameraDevice2.h"
#include "CallbackNotifier.h"
#include "PreviewWindow.h"
#include "CameraHardware2.h"
#include "HALCameraFactory.h"

#include "math.h"

#define CHECK_NO_ERROR(a)						\
	if (a != NO_ERROR) {						\
		if (mCameraFd != NULL) {				\
			close(mCameraFd);					\
			LOGE("error close device");			\
			mCameraFd = NULL;					\
		}										\
		return EINVAL;							\
	}
	
extern void PreviewCnr(unsigned int snr_level, unsigned int gain, int width, int height, char *src, char *dst);
extern void CaptureProcessing(unsigned char * image, int min_val, int level, int width, int height);
	
namespace android {
	
// defined in HALCameraFactory.cpp
extern void getCallingProcessName(char *name);


static void calculateCrop(Rect * rect, int new_zoom, int max_zoom, int width, int height)
{
	if (max_zoom == 0)
	{
		rect->left		= 0;
		rect->top		= 0;
		rect->right 	= width -1;
		rect->bottom	= height -1;
	}
	else
	{
		int new_ratio = (new_zoom * 2 * 100 / max_zoom + 100);
		rect->left		= (width - (width * 100) / new_ratio)/2;
		rect->top		= (height - (height * 100) / new_ratio)/2;
		rect->right 	= rect->left + (width * 100) / new_ratio -1;
		rect->bottom	= rect->top  + (height * 100) / new_ratio - 1;
	}
	
	// LOGD("crop: [%d, %d, %d, %d]", rect->left, rect->top, rect->right, rect->bottom);
}


static void YUYVToNV12(const void* yuyv, void *nv12, int width, int height)
{
	uint8_t* Y	= (uint8_t*)nv12;
	uint8_t* UV = (uint8_t*)Y + width * height;
	
	for(int i = 0; i < height; i += 2)
	{
		for (int j = 0; j < width; j++)
		{
			*(uint8_t*)((uint8_t*)Y + i * width + j) = *(uint8_t*)((uint8_t*)yuyv + i * width * 2 + j * 2);
			*(uint8_t*)((uint8_t*)Y + (i + 1) * width + j) = *(uint8_t*)((uint8_t*)yuyv + (i + 1) * width * 2 + j * 2);
			*(uint8_t*)((uint8_t*)UV + ((i * width) >> 1) + j) = *(uint8_t*)((uint8_t*)yuyv + i * width * 2 + j * 2 + 1);
		}
	}
}

static void YUYVToNV21(const void* yuyv, void *nv21, int width, int height)
{
	uint8_t* Y	= (uint8_t*)nv21;
	uint8_t* VU = (uint8_t*)Y + width * height;
	
	for(int i = 0; i < height; i += 2)
	{
		for (int j = 0; j < width; j++)
		{
			*(uint8_t*)((uint8_t*)Y + i * width + j) = *(uint8_t*)((uint8_t*)yuyv + i * width * 2 + j * 2);
			*(uint8_t*)((uint8_t*)Y + (i + 1) * width + j) = *(uint8_t*)((uint8_t*)yuyv + (i + 1) * width * 2 + j * 2);

			if (j % 2)
			{
				if (j < width - 1)
				{
					*(uint8_t*)((uint8_t*)VU + ((i * width) >> 1) + j) = *(uint8_t*)((uint8_t*)yuyv + i * width * 2 + (j + 1) * 2 + 1);
				}
			}
			else
			{
				if (j > 1)
				{
					*(uint8_t*)((uint8_t*)VU + ((i * width) >> 1) + j) = *(uint8_t*)((uint8_t*)yuyv + i * width * 2 + (j - 1) * 2 + 1); 		
				}
			}
		}
	}
}

#ifdef USE_MP_CONVERT



void V4L2CameraDevice::YUYVToYUV420C(const void* yuyv, void *yuv420, int width, int height)
{
	g2d_blt		blit_para;
	int 		err;
	
	blit_para.src_image.addr[0]      = (int)yuyv;
	blit_para.src_image.addr[1]      = (int)yuyv + width * height;
	blit_para.src_image.w            = width;	      /* src buffer width in pixel units */
	blit_para.src_image.h            = height;	      /* src buffer height in pixel units */
	blit_para.src_image.format       = G2D_FMT_IYUV422;
	blit_para.src_image.pixel_seq    = G2D_SEQ_NORMAL;          /* not use now */
	blit_para.src_rect.x             = 0;						/* src rect->x in pixel */
	blit_para.src_rect.y             = 0;						/* src rect->y in pixel */
	blit_para.src_rect.w             = width;			/* src rect->w in pixel */
	blit_para.src_rect.h             = height;			/* src rect->h in pixel */

	blit_para.dst_image.addr[0]      = (int)yuv420;
	blit_para.dst_image.addr[1]      = (int)yuv420 + width * height;
	blit_para.dst_image.w            = width;	      /* dst buffer width in pixel units */			
	blit_para.dst_image.h            = height;	      /* dst buffer height in pixel units */
	blit_para.dst_image.format       = G2D_FMT_PYUV420UVC;
	blit_para.dst_image.pixel_seq    = (mVideoFormat == V4L2_PIX_FMT_NV12) ? G2D_SEQ_NORMAL : G2D_SEQ_VUVU;          /* not use now */
	blit_para.dst_x                  = 0;					/* dst rect->x in pixel */
	blit_para.dst_y                  = 0;					/* dst rect->y in pixel */
	blit_para.color                  = 0xff;          		/* fix me*/
	blit_para.alpha                  = 0xff;                /* globe alpha */ 

	blit_para.flag = G2D_BLT_NONE; // G2D_BLT_FLIP_HORIZONTAL;

	err = ioctl(mG2DHandle, G2D_CMD_BITBLT, (unsigned long)&blit_para);				
	if(err < 0) 	
	{			
		LOGE("ioctl, G2D_CMD_BITBLT failed");
		return;
	}
}

void V4L2CameraDevice::NV21ToYV12(const void* nv21, void *yv12, int width, int height)
{
	g2d_blt		blit_para;
	int 		err;
	int			u, v;
	if (mVideoFormat == V4L2_PIX_FMT_NV21)
	{
		u = 1;
		v = 2;
	}
	else
	{
		u = 2;
		v = 1;
	}
	
	blit_para.src_image.addr[0]      = (int)nv21;
	blit_para.src_image.addr[1]      = (int)nv21 + width * height;
	blit_para.src_image.w            = width;	      /* src buffer width in pixel units */
	blit_para.src_image.h            = height;	      /* src buffer height in pixel units */
	blit_para.src_image.format       = G2D_FMT_PYUV420UVC;
	blit_para.src_image.pixel_seq    = G2D_SEQ_NORMAL;//G2D_SEQ_VUVU;          /*  */
	blit_para.src_rect.x             = 0;						/* src rect->x in pixel */
	blit_para.src_rect.y             = 0;						/* src rect->y in pixel */
	blit_para.src_rect.w             = width;			/* src rect->w in pixel */
	blit_para.src_rect.h             = height;			/* src rect->h in pixel */

	blit_para.dst_image.addr[0]      = (int)yv12;							// y
	blit_para.dst_image.addr[u]      = (int)yv12 + width * height;			// v
	blit_para.dst_image.addr[v]      = (int)yv12 + width * height * 5 / 4;	// u
	blit_para.dst_image.w            = width;	      /* dst buffer width in pixel units */			
	blit_para.dst_image.h            = height;	      /* dst buffer height in pixel units */
	blit_para.dst_image.format       = G2D_FMT_PYUV420;
	blit_para.dst_image.pixel_seq    = G2D_SEQ_NORMAL;          /* not use now */
	blit_para.dst_x                  = 0;					/* dst rect->x in pixel */
	blit_para.dst_y                  = 0;					/* dst rect->y in pixel */
	blit_para.color                  = 0xff;          		/* fix me*/
	blit_para.alpha                  = 0xff;                /* globe alpha */ 
	
	blit_para.flag = G2D_BLT_NONE;
	
	err = ioctl(mG2DHandle , G2D_CMD_BITBLT, (unsigned long)&blit_para);				
	if(err < 0)
	{
		LOGE("NV21ToYV12 ioctl, G2D_CMD_BITBLT failed");
		return;
	}
}
#endif

DBG_TIME_AVG_BEGIN(TAG_CONTINUOUS_PICTURE);
static bool saveframe(char *str,void *p,int length,int is_oneframe)
{      
               int fd; 
               if(is_oneframe)
                       fd = open(str,O_CREAT|O_RDWR|O_TRUNC,0777);        //save one frame data
               else            
                       fd = open(str,O_CREAT|O_RDWR|O_APPEND,0777);       //save more frames
               if(!fd) {
                       LOGE("Open file error");
                       return false;
               }
               if(write(fd,p,length)){
                       //LOGD("Write file successfully");
                       close(fd);
                       return true;
                       }
               else {
                       LOGE("Write file fail");
                       close(fd);
                       return false;
}
}
void  V4L2CameraDevice::showformat(int format,char *str)
{
	switch(format){
	case V4L2_PIX_FMT_YUYV:
		LOGV("The %s foramt is V4L2_PIX_FMT_YUYV",str);
		break;
	case V4L2_PIX_FMT_MJPEG:
		LOGV("The %s foramt is V4L2_PIX_FMT_MJPEG",str);
		break;
	case V4L2_PIX_FMT_YVU420:
		LOGV("The %s foramt is V4L2_PIX_FMT_YVU420",str);
		break;
	case V4L2_PIX_FMT_NV12:
		LOGV("The %s foramt is V4L2_PIX_FMT_NV12",str);
		break;
	case V4L2_PIX_FMT_NV21:
		LOGV("The %s foramt is V4L2_PIX_FMT_NV21",str);
		break;
	case V4L2_PIX_FMT_H264:
		LOGV("The %s foramt is V4L2_PIX_FMT_H264",str);
		break;
	default:
		LOGV("The %s format can't be showed",str);
	}
}
V4L2CameraDevice::V4L2CameraDevice(CameraHardware* camera_hal,
								   PreviewWindow * preview_window, 
    							   CallbackNotifier * cb)
    : mCameraHardware(camera_hal),
      mPreviewWindow(preview_window),
      mCallbackNotifier(cb),
      mCameraDeviceState(STATE_CONSTRUCTED),
      mCaptureThreadState(CAPTURE_STATE_NULL),
      mCameraFd(0),
      mIsUsbCamera(false),
      mTakePictureState(TAKE_PICTURE_NULL),
      mIsPicCopy(false),
      mFrameWidth(0),
      mFrameHeight(0),
      mThumbWidth(0),
      mThumbHeight(0),
      mCurFrameTimestamp(0),
      mBufferCnt(NB_BUFFER),
      mUseHwEncoder(false),
	  mNewZoom(0),
	  mLastZoom(-1),
	  mMaxZoom(0xffffffff),
	  mCaptureFormat(V4L2_PIX_FMT_NV21),
	  mVideoFormat(V4L2_PIX_FMT_NV21),
	  mFrameRate(30),
	  mStartSmartTimeout(false),
	  gain_bright(0),
	  gain_dark(0),
	  HDRMode(0),
	  DenoiseMode(0),
	  hdrWidth(0),
	  hdrHeight(0),
	  hdrStep1(false),
	  hdrStep2(false),
	  hdrStep3(false),
	  hdrStep4(false)
#ifdef USE_MP_CONVERT
	  ,mG2DHandle(0)
#endif
	  ,mCurrentV4l2buf(NULL)
	  ,mCanBeDisconnected(false)
	  ,mContinuousPictureStarted(false)
	  ,mContinuousPictureCnt(0)
	  ,mContinuousPictureMax(0)
	  ,mContinuousPictureStartTime(0)
	  ,mContinuousPictureLast(0)
	  ,mContinuousPictureAfter(0)
	  ,mSmartPictureDone(true)
	  ,mSceneModePictureDone(true)
	  ,mSceneModePicture(0)
	  ,mImage0Yuv(NULL)
	  ,mImage1Yuv(NULL)
	  ,mImage2Yuv(NULL)
	  ,mImage3Yuv(NULL)
	  ,mImage4Yuv(NULL)
	  ,mFaceDectectLast(0)
	  ,mFaceDectectAfter(0)
	  ,mPreviewLast(0)
	  ,mPreviewAfter(0)
	  ,mVideoHint(false)
	  ,mCurAvailBufferCnt(0)
	  ,mStatisicsIndex(0)
	  ,mNeedHalfFrameRate(false)
	  ,mShouldPreview(true)
	  ,mIsThumbUsedForVideo(false)
	  ,mVideoWidth(640)
	  ,mVideoHeight(480)
{
	LOGV("V4L2CameraDevice construct");

	memset(&mMapMem,0,sizeof(mMapMem));
	memset(&mVideoBuffer,0,sizeof(mVideoBuffer));

	memset(&mHalCameraInfo, 0, sizeof(mHalCameraInfo));
	memset(&mRectCrop, 0, sizeof(Rect));

	// init preview buffer queue
	OSAL_QueueCreate(&mQueueBufferPreview, NB_BUFFER);
	OSAL_QueueCreate(&mQueueBufferPicture, 4);
	
	// init capture thread
	mCaptureThread = new DoCaptureThread(this);
	pthread_mutex_init(&mCaptureMutex, NULL);
	pthread_cond_init(&mCaptureCond, NULL);
	mCaptureThreadState = CAPTURE_STATE_PAUSED;
	mCaptureThread->startThread();

	// init preview thread
	mPreviewThread = new DoPreviewThread(this);
	pthread_mutex_init(&mPreviewMutex, NULL);
	pthread_cond_init(&mPreviewCond, NULL);
	mPreviewThread->startThread();

	// init picture thread
	mPictureThread = new DoPictureThread(this);
	pthread_mutex_init(&mPictureMutex, NULL);
	pthread_cond_init(&mPictureCond, NULL);
	mPictureThread->startThread();
	
	pthread_mutex_init(&mConnectMutex, NULL);
	pthread_cond_init(&mConnectCond, NULL);
	
	// init continuous picture thread
	mContinuousPictureThread = new DoContinuousPictureThread(this);
	pthread_mutex_init(&mContinuousPictureMutex, NULL);
	pthread_cond_init(&mContinuousPictureCond, NULL);
	mContinuousPictureThread->startThread();

	// init smart picture thread
	mSmartPictureThread = new DoSmartPictureThread(this);
	pthread_mutex_init(&mSmartPictureMutex, NULL);
	pthread_cond_init(&mSmartPictureCond, NULL);
	mSmartPictureThread->startThread();

	// init scene mode picture thread
	mSceneModePictureThread = new DoSceneModePictureThread(this);
	pthread_mutex_init(&mSceneModePictureMutex, NULL);
	pthread_cond_init(&mSceneModePictureCond, NULL);
	mSceneModePictureThread->startThread();
}

V4L2CameraDevice::~V4L2CameraDevice()
{
	LOGV("V4L2CameraDevice disconstruct");

	if (mCaptureThread != NULL)
	{
		mCaptureThread->stopThread();
		pthread_cond_signal(&mCaptureCond);
		mCaptureThread.clear();
		mCaptureThread = 0;
	}

	if (mPreviewThread != NULL)
	{
		mPreviewThread->stopThread();
		pthread_cond_signal(&mPreviewCond);
		mPreviewThread.clear();
		mPreviewThread = 0;
	}

	if (mPictureThread != NULL)
	{
		mPictureThread->stopThread();
		pthread_cond_signal(&mPictureCond);
		mPictureThread.clear();
		mPictureThread = 0;
	}

	if (mContinuousPictureThread != NULL)
	{
		mContinuousPictureThread->stopThread();
		pthread_cond_signal(&mContinuousPictureCond);
		mContinuousPictureThread.clear();
		mContinuousPictureThread = 0;
	}

	if (mSmartPictureThread != NULL)
	{
		mSmartPictureThread->stopThread();
		pthread_cond_signal(&mSmartPictureCond);
		mSmartPictureThread.clear();
		mSmartPictureThread = 0;
	}

	if (mSceneModePictureThread != NULL)
	{
		mSceneModePictureThread->stopThread();
		pthread_cond_signal(&mSceneModePictureCond);
		mSceneModePictureThread.clear();
		mSceneModePictureThread = 0;
	}

	pthread_mutex_destroy(&mCaptureMutex);
	pthread_cond_destroy(&mCaptureCond);

	pthread_mutex_destroy(&mPreviewMutex);
	pthread_cond_destroy(&mPreviewCond);
	
	pthread_mutex_destroy(&mPictureMutex);
	pthread_cond_destroy(&mPictureCond);
	
	pthread_mutex_destroy(&mConnectMutex);
	pthread_cond_destroy(&mConnectCond);
	
	pthread_mutex_destroy(&mContinuousPictureMutex);
	pthread_cond_destroy(&mContinuousPictureCond);

	pthread_mutex_destroy(&mSmartPictureMutex);
	pthread_cond_destroy(&mSmartPictureCond);

	pthread_mutex_destroy(&mSceneModePictureMutex);
	pthread_cond_destroy(&mSceneModePictureCond);
	
	OSAL_QueueTerminate(&mQueueBufferPreview);
	OSAL_QueueTerminate(&mQueueBufferPicture);
}

/****************************************************************************
 * V4L2CameraDevice interface implementation.
 ***************************************************************************/

status_t V4L2CameraDevice::connectDevice(HALCameraInfo * halInfo)
{
	F_LOG;

    Mutex::Autolock locker(&mObjectLock);
	
	if (isConnected()) 
	{
		LOGW("%s: camera device is already connected.", __FUNCTION__);
		return NO_ERROR;
	}

	// open v4l2 camera device
	int ret = openCameraDev(halInfo);
	if (ret != OK)
	{
		return ret;
	}

	if (!mIsUsbCamera)
	{
		//get sensor type form driver
		switch((v4l2_sensor_type)getSensorType()){
			case V4L2_SENSOR_TYPE_YUV:
				LOGV("the sensor is YUV sensor");
				mSensor_Type = V4L2_SENSOR_TYPE_YUV;
				break;
			case V4L2_SENSOR_TYPE_RAW:
				LOGV("the sensor is RAW sensor");
				mSensor_Type = V4L2_SENSOR_TYPE_RAW;
				break;
			default:
				LOGE("get the sensor type failed");
				goto END_ERROR;
		}
		halInfo->fast_picture_mode = (int)mSensor_Type;	//set the CameraHardware Class fast_picture_mode member
	}
	memcpy((void*)&mHalCameraInfo, (void*)halInfo, sizeof(HALCameraInfo));
#ifdef USE_MP_CONVERT
	if (mIsUsbCamera)
	{
		// open MP driver
		mG2DHandle = open("/dev/g2d", O_RDWR, 0);
		if (mG2DHandle < 0)
		{
			LOGE("open /dev/g2d failed");
			goto END_ERROR;
		}
		LOGV("open /dev/g2d OK");
	}
#endif 

#ifdef USE_ION_MEM_ALLOCATOR
	ret = ion_alloc_open();
	if (ret < 0)
	{
		LOGE("ion_alloc_open failed");
		goto END_ERROR;
	}
	LOGV("ion_alloc_open ok");

	// used for normal picture mode
	mPicBuffer.addrVirY = (int)ion_alloc_alloc(MAX_YUV_SENSOR_PICTURE_SIZE);
	mPicBuffer.addrPhyY = ion_alloc_vir2phy((void*)mPicBuffer.addrVirY);
#elif USE_SUNXI_MEM_ALLOCATOR
	ret = sunxi_alloc_open();
	if (ret < 0)
	{
		LOGE("sunxi_alloc_open failed");
		goto END_ERROR;
	}
	LOGV("sunxi_alloc_open ok");

	// used for normal picture mode
	mPicBuffer.addrVirY = (int)sunxi_alloc_alloc(MAX_YUV_SENSOR_PICTURE_SIZE);
	mPicBuffer.addrPhyY = sunxi_alloc_vir2phy((void*)mPicBuffer.addrVirY);
#endif

	/* There is a device to connect to. */
	mCameraDeviceState = STATE_CONNECTED;

    return NO_ERROR;
	
END_ERROR:

	if (mCameraFd != NULL)
	{
		close(mCameraFd);
		mCameraFd = NULL;
	}
	
#ifdef USE_MP_CONVERT
	if(mG2DHandle != NULL)
	{
		close(mG2DHandle);
		mG2DHandle = NULL;
	}
#endif

	return UNKNOWN_ERROR;
}

status_t V4L2CameraDevice::disconnectDevice()
{
	F_LOG;
	
	Mutex::Autolock locker(&mObjectLock);
	
	if (!isConnected()) 
	{
		LOGW("%s: camera device is already disconnected.", __FUNCTION__);
		return NO_ERROR;
	}
	
	if (isStarted()) 
	{
		LOGE("%s: Cannot disconnect from the started device.", __FUNCTION__);
		return -EINVAL;
	}
	
	// close v4l2 camera device
	closeCameraDev();
	
#ifdef USE_MP_CONVERT
	if(mG2DHandle != NULL)
	{
		close(mG2DHandle);
		mG2DHandle = NULL;
	}
#endif

#ifdef USE_ION_MEM_ALLOCATOR
        if (mPicBuffer.addrVirY != NULL)
	{
		ion_alloc_free((void*)mPicBuffer.addrVirY);
		mPicBuffer.addrPhyY = 0;
	}

	int ret = ion_alloc_close();
	if (ret < 0)
	{
		LOGE("sunxi_alloc_close failed\n");
	}
	else
	{
		LOGV("sunxi_alloc_close ok");
	}
#elif USE_SUNXI_MEM_ALLOCATOR
	if (mPicBuffer.addrVirY != NULL)
	{
		sunxi_alloc_free((void*)mPicBuffer.addrVirY);
		mPicBuffer.addrPhyY = 0;
	}

	int ret = sunxi_alloc_close();
	if (ret < 0)
	{
		LOGE("sunxi_alloc_close failed\n");
	}
	else
	{
		LOGV("sunxi_alloc_close ok");
	}
#endif

    /* There is no device to disconnect from. */
    mCameraDeviceState = STATE_CONSTRUCTED;

    return NO_ERROR;
}

status_t V4L2CameraDevice::startDevice(int width,
                                       int height,
                                       uint32_t pix_fmt,
                                       bool video_hint)
{
	LOGD("%s, wxh: %dx%d, fmt: %d", __FUNCTION__, width, height, pix_fmt);
	
	Mutex::Autolock locker(&mObjectLock);
	
	if (!isConnected()) 
	{
		LOGE("%s: camera device is not connected.", __FUNCTION__);
		return EINVAL;
	}
	
	if (isStarted()) 
	{
		LOGE("%s: camera device is already started.", __FUNCTION__);
		return EINVAL;
	}

	// VE encoder need this format
	mVideoFormat = pix_fmt;
	mCurrentV4l2buf = NULL;

	mVideoHint = video_hint;
	mCanBeDisconnected = false;

	// set capture mode and fps
	// CHECK_NO_ERROR(v4l2setCaptureParams());	// do not check this error
	v4l2setCaptureParams();
	
	// set v4l2 device parameters, it maybe change the value of mFrameWidth and mFrameHeight.
	CHECK_NO_ERROR(v4l2SetVideoParams(width, height, pix_fmt));
	
	// v4l2 request buffers
	int buf_cnt = (mTakePictureState == TAKE_PICTURE_NORMAL) ? 1 : NB_BUFFER;
	CHECK_NO_ERROR(v4l2ReqBufs(&buf_cnt));
	mBufferCnt = buf_cnt;
	mCurAvailBufferCnt = mBufferCnt;

	// v4l2 query buffers
	CHECK_NO_ERROR(v4l2QueryBuf());
	
	// stream on the v4l2 device
	CHECK_NO_ERROR(v4l2StartStreaming());

	mCameraDeviceState = STATE_STARTED;

	mContinuousPictureAfter = 1000000 / 10;
	mFaceDectectAfter = 1000000 / 15;
	mPreviewAfter = 1000000 / 24;
	
    return NO_ERROR;
}

status_t V4L2CameraDevice::stopDevice()
{
	LOGD("V4L2CameraDevice::stopDevice");
	
	pthread_mutex_lock(&mConnectMutex);
	if (!mCanBeDisconnected)
	{
		LOGW("wait until capture thread pause or exit");
		pthread_cond_wait(&mConnectCond, &mConnectMutex);
	}
	pthread_mutex_unlock(&mConnectMutex);
	
	Mutex::Autolock locker(&mObjectLock);
	
	if (!isStarted()) 
	{
		LOGW("%s: camera device is not started.", __FUNCTION__);
		return NO_ERROR;
	}

	// v4l2 device stop stream
	v4l2StopStreaming();
	
	// v4l2 device unmap buffers
    v4l2UnmapBuf();
	
	for(int i = 0; i < NB_BUFFER; i++)
	{
		memset(&mV4l2buf[i], 0, sizeof(V4L2BUF_t));
	}
	
	mCameraDeviceState = STATE_CONNECTED;

	mLastZoom = -1;
	
	mCurrentV4l2buf = NULL;
	
    return NO_ERROR;
}

status_t V4L2CameraDevice::startDeliveringFrames()
{
	F_LOG;
	
	pthread_mutex_lock(&mCaptureMutex);

	if (mCaptureThreadState == CAPTURE_STATE_NULL)
	{
		LOGE("error state of capture thread, %s", __FUNCTION__);
		pthread_mutex_unlock(&mCaptureMutex);
		return EINVAL;
	}

	if (mCaptureThreadState == CAPTURE_STATE_STARTED)
	{
		LOGW("capture thread has already started");
		pthread_mutex_unlock(&mCaptureMutex);
		return NO_ERROR;
	}

	// singal to start capture thread
	mCaptureThreadState = CAPTURE_STATE_STARTED;
	pthread_cond_signal(&mCaptureCond);
	pthread_mutex_unlock(&mCaptureMutex);

	return NO_ERROR;
}

status_t V4L2CameraDevice::stopDeliveringFrames()
{
	F_LOG;
	
	pthread_mutex_lock(&mCaptureMutex);
	if (mCaptureThreadState == CAPTURE_STATE_NULL)
	{
		LOGE("error state of capture thread, %s", __FUNCTION__);
		pthread_mutex_unlock(&mCaptureMutex);
		return EINVAL;
	}

	if (mCaptureThreadState == CAPTURE_STATE_PAUSED)
	{
		LOGW("capture thread has already paused");
		pthread_mutex_unlock(&mCaptureMutex);
		return NO_ERROR;
	}

	mCaptureThreadState = CAPTURE_STATE_PAUSED;
	pthread_mutex_unlock(&mCaptureMutex);

	return NO_ERROR;
}


/****************************************************************************
 * Worker thread management.
 ***************************************************************************/

int V4L2CameraDevice::v4l2WaitCameraReady()
{
	fd_set fds;		
	struct timeval tv;
	int r;

	FD_ZERO(&fds);
	FD_SET(mCameraFd, &fds);		
	
	/* Timeout */
	tv.tv_sec  = 2;
	tv.tv_usec = 0;
	
	r = select(mCameraFd + 1, &fds, NULL, NULL, &tv);
	if (r == -1) 
	{
		LOGE("select err, %s", strerror(errno));
		return -1;
	} 
	else if (r == 0) 
	{
		LOGE("select timeout");
		return -1;
	}

	return 0;
}

void V4L2CameraDevice::singalDisconnect()
{
	pthread_mutex_lock(&mConnectMutex);
	mCanBeDisconnected = true;
	pthread_cond_signal(&mConnectCond);
	pthread_mutex_unlock(&mConnectMutex);
}

bool V4L2CameraDevice::captureThread()
{
	pthread_mutex_lock(&mCaptureMutex);
	// stop capture
	if (mCaptureThreadState == CAPTURE_STATE_PAUSED)
	{
		singalDisconnect();
		// wait for signal of starting to capture a frame
		LOGV("capture thread paused");
		pthread_cond_wait(&mCaptureCond, &mCaptureMutex);
	}

	// thread exit
	if (mCaptureThreadState == CAPTURE_STATE_EXIT)
	{
		singalDisconnect();
		LOGV("capture thread exit");
		pthread_mutex_unlock(&mCaptureMutex);
		return false;
	}
	pthread_mutex_unlock(&mCaptureMutex);

	int ret = v4l2WaitCameraReady();

	pthread_mutex_lock(&mCaptureMutex);
	// stop capture or thread exit
	if (mCaptureThreadState == CAPTURE_STATE_PAUSED
		|| mCaptureThreadState == CAPTURE_STATE_EXIT)
	{
		singalDisconnect();
		LOGW("should stop capture now");
		pthread_mutex_unlock(&mCaptureMutex);
		return __LINE__;
	}

	if (ret != 0)
	{
		LOGW("wait v4l2 buffer time out");
		pthread_mutex_unlock(&mCaptureMutex);

		LOGW("preview queue has %d items.", OSAL_GetElemNum(&mQueueBufferPreview));
		return __LINE__;
	}

	// get one video frame
	struct v4l2_buffer buf;
	memset(&buf, 0, sizeof(v4l2_buffer));
	ret = getPreviewFrame(&buf);
	if (ret != OK)
	{	
		pthread_mutex_unlock(&mCaptureMutex);
		
		usleep(10000);
		return ret;
	}

	mCurAvailBufferCnt--;

	if (mCurAvailBufferCnt <= 4)
	{
		mNeedHalfFrameRate = true;
		mStatisicsIndex = 0;
	}
	else if (mNeedHalfFrameRate)
	{
		mStatisicsIndex++;
		if (mStatisicsIndex >= STATISICS_CNT)
		{
			mNeedHalfFrameRate = false;
		}
	}
	
	// deal with this frame
	mCurFrameTimestamp = (int64_t)((int64_t)buf.timestamp.tv_usec + (((int64_t)buf.timestamp.tv_sec) * 1000000));

	if (mLastZoom != mNewZoom)
	{
		float widthRate = (float)mFrameWidth / (float)mVideoWidth;
		float heightRate = (float)mFrameHeight / (float)mVideoHeight;
		if (mIsThumbUsedForVideo && (widthRate > 4.0) && (heightRate > 4.0))
		{
			calculateCrop(&mRectCrop, mNewZoom, mMaxZoom, mFrameWidth/2, mFrameHeight/2);	// for cts, to do 
		}
		else
		{
			// the main frame crop
			calculateCrop(&mRectCrop, mNewZoom, mMaxZoom, mFrameWidth, mFrameHeight);
		}
		mCameraHardware->setNewCrop(&mRectCrop);
		
		// the sub-frame crop
		if (mHalCameraInfo.fast_picture_mode)
		{
			calculateCrop(&mThumbRectCrop, mNewZoom, mMaxZoom, mThumbWidth, mThumbHeight);
		}
		
		mLastZoom = mNewZoom;

		LOGV("CROP: [%d, %d, %d, %d]", mRectCrop.left, mRectCrop.top, mRectCrop.right, mRectCrop.bottom);
		LOGV("thumb CROP: [%d, %d, %d, %d]", mThumbRectCrop.left, mThumbRectCrop.top, mThumbRectCrop.right, mThumbRectCrop.bottom);
	}

	if (mVideoFormat != V4L2_PIX_FMT_YUYV
		&& mCaptureFormat == V4L2_PIX_FMT_YUYV)
	{
#ifdef USE_MP_CONVERT
		YUYVToYUV420C((void*)buf.m.offset, 
					  (void*)(mVideoBuffer.buf_phy_addr[buf.index] | 0x40000000),
					  mFrameWidth, 
					  mFrameHeight);
#else
		ion_flush_cache((void*)mVideoBuffer.buf_vir_addr[buf.index], mFrameWidth * mFrameHeight * 2);

		YUYVToNV21(mMapMem.mem[buf.index], 
					   (void*)mVideoBuffer.buf_vir_addr[buf.index], 
					   mFrameWidth, 
					   mFrameHeight);
		ion_flush_cache((void*)mVideoBuffer.buf_vir_addr[buf.index], mFrameWidth * mFrameHeight * 2);
#endif
	}

	// V4L2BUF_t for preview and HW encoder
	V4L2BUF_t v4l2_buf;
	if (mVideoFormat != V4L2_PIX_FMT_YUYV
		&& mCaptureFormat == V4L2_PIX_FMT_YUYV)
	{
		v4l2_buf.addrPhyY		= mVideoBuffer.buf_phy_addr[buf.index];//- 0x20000000; 
		v4l2_buf.addrVirY		= mVideoBuffer.buf_vir_addr[buf.index]; 
	}
	else
	{
		v4l2_buf.addrPhyY		= buf.m.offset - 0x20000000;  //dram addr offset for ve 
		v4l2_buf.addrVirY		= (unsigned int)mMapMem.mem[buf.index];
	}
	v4l2_buf.index				= buf.index;
	v4l2_buf.timeStamp			= mCurFrameTimestamp;
	v4l2_buf.width				= mFrameWidth;
	v4l2_buf.height				= mFrameHeight;
	v4l2_buf.crop_rect.left		= mRectCrop.left;
	v4l2_buf.crop_rect.top		= mRectCrop.top;
	v4l2_buf.crop_rect.width	= mRectCrop.right - mRectCrop.left + 1;
	v4l2_buf.crop_rect.height	= mRectCrop.bottom - mRectCrop.top + 1;
	v4l2_buf.format				= mVideoFormat;

	if (mHalCameraInfo.fast_picture_mode)
	{
		v4l2_buf.isThumbAvailable		= 1;
		v4l2_buf.thumbUsedForPreview	= 1;
		v4l2_buf.thumbUsedForPhoto		= 0;
		if(mIsThumbUsedForVideo == true)
		{
			v4l2_buf.thumbUsedForVideo		= 1;
		}
		else
		{
			v4l2_buf.thumbUsedForVideo		= 0;
		}
		v4l2_buf.thumbAddrPhyY			= v4l2_buf.addrPhyY + ALIGN_4K(ALIGN_16B(mFrameWidth) * mFrameHeight * 3 / 2);	// to do
		v4l2_buf.thumbAddrVirY			= v4l2_buf.addrVirY + ALIGN_4K(ALIGN_16B(mFrameWidth) * mFrameHeight * 3 / 2);	// to do
		v4l2_buf.thumbWidth				= mThumbWidth;
		v4l2_buf.thumbHeight			= mThumbHeight;
		v4l2_buf.thumb_crop_rect.left	= mThumbRectCrop.left;
		v4l2_buf.thumb_crop_rect.top	= mThumbRectCrop.top;
		v4l2_buf.thumb_crop_rect.width	= mThumbRectCrop.right - mThumbRectCrop.left;
		v4l2_buf.thumb_crop_rect.height	= mThumbRectCrop.bottom - mThumbRectCrop.top;
		v4l2_buf.thumbFormat			= mVideoFormat;
	}
	else
	{
		v4l2_buf.isThumbAvailable		= 0;
	}

	v4l2_buf.refCnt = 1;
	memcpy(&mV4l2buf[v4l2_buf.index], &v4l2_buf, sizeof(V4L2BUF_t));
	if ((!mVideoHint) && (mTakePictureState != TAKE_PICTURE_NORMAL))
	{
		// face detection only use when picture mode
		mCurrentV4l2buf = &mV4l2buf[v4l2_buf.index];
	}

	if (mTakePictureState == TAKE_PICTURE_NORMAL)
	{
		//copy picture buffer
		unsigned int phy_addr = mPicBuffer.addrPhyY;
		unsigned int vir_addr = mPicBuffer.addrVirY;
		int frame_size = mFrameWidth * mFrameHeight * 3 >> 1;

		if (frame_size > MAX_YUV_SENSOR_PICTURE_SIZE)
		{
			LOGE("picture buffer size(%d) is smaller than the frame buffer size(%d)", MAX_YUV_SENSOR_PICTURE_SIZE, frame_size);
			pthread_mutex_unlock(&mCaptureMutex);
			return false;
		}
		
		memcpy((void*)&mPicBuffer, &v4l2_buf, sizeof(V4L2BUF_t));
		mPicBuffer.addrPhyY = phy_addr;
		mPicBuffer.addrVirY = vir_addr;
	
		#ifdef USE_ION_MEM_ALLOCATOR
		ion_flush_cache((void*)v4l2_buf.addrVirY, frame_size);
		memcpy((void*)mPicBuffer.addrVirY, (void*)v4l2_buf.addrVirY, frame_size);
		ion_flush_cache((void*)mPicBuffer.addrVirY, frame_size);
		#elif USE_SUNXI_MEM_ALLOCATOR
		sunxi_flush_cache((void*)v4l2_buf.addrVirY, frame_size);
		memcpy((void*)mPicBuffer.addrVirY, (void*)v4l2_buf.addrVirY, frame_size);
		sunxi_flush_cache((void*)mPicBuffer.addrVirY, frame_size);
		#endif

		// enqueue picture buffer
		ret = OSAL_Queue(&mQueueBufferPicture, &mPicBuffer);
		if (ret != 0)
		{
			LOGW("picture queue full");
			pthread_cond_signal(&mPictureCond);
			goto DEC_REF;
		}
		
		mIsPicCopy = true;
		mCaptureThreadState = CAPTURE_STATE_PAUSED;
		mTakePictureState = TAKE_PICTURE_NULL;
		pthread_cond_signal(&mPictureCond);
		
		goto DEC_REF;
	}
	else
	{
		ret = OSAL_Queue(&mQueueBufferPreview, &mV4l2buf[v4l2_buf.index]);
		if (ret != 0)
		{
			LOGW("preview queue full");
			goto DEC_REF;
		}

		// add reference count
		mV4l2buf[v4l2_buf.index].refCnt++;
		// signal a new frame for preview
		pthread_cond_signal(&mPreviewCond);

		if (mTakePictureState == TAKE_PICTURE_SCENE_MODE)
		{
			int FrameCnt = getHDRFrameCnt();
			LOGD("FrameCnt = %d", FrameCnt);
			
			if(FrameCnt == 1)
			{
				if(hdrStep1)
				{
				    ALOGD("step 1");
					//TO DO: save dark picture
					int frame_size = mFrameWidth * mFrameHeight * 3 >> 1;
					#ifdef USE_ION_MEM_ALLOCATOR
					ion_flush_cache((void*)v4l2_buf.addrVirY, frame_size);
					memcpy((void *)mImage0Yuv, (void *)v4l2_buf.addrVirY, frame_size);
					#elif USE_SUNXI_MEM_ALLOCATOR
					sunxi_flush_cache((void*)v4l2_buf.addrVirY, frame_size);
					memcpy((void *)mImage0Yuv, (void *)v4l2_buf.addrVirY, frame_size);
					#endif
					hdrStep1 = false;
				}

			}
			
			if(FrameCnt == 2)
			{
				if(hdrStep2)
				{
				    ALOGD("step 2");
					//TO DO: save light picture
					int frame_size = mFrameWidth * mFrameHeight * 3 >> 1;
					#ifdef USE_ION_MEM_ALLOCATOR
					ion_flush_cache((void *)v4l2_buf.addrVirY, frame_size);
					memcpy((void *)mImage1Yuv, (void *)v4l2_buf.addrVirY, frame_size);
					#elif USE_SUNXI_MEM_ALLOCATOR
					sunxi_flush_cache((void*)v4l2_buf.addrVirY, frame_size);
					memcpy((void *)mImage1Yuv, (void *)v4l2_buf.addrVirY, frame_size);
					#endif
					
					HDRMode = 1;

					hdrWidth = v4l2_buf.width,
					hdrHeight = v4l2_buf.height,

					hdrStep2 = false;				
				}
			}
			
			if((FrameCnt <= 2) && (hdrStep2 == true))
				goto DEC_REF;
			if(FrameCnt>2)
			{
				struct isp_hdr_setting_t hdr_setting;
				hdr_setting.hdr_en = 0;
				hdr_setting.total_frames = 5;
				hdr_setting.values[0] = -50;
				hdr_setting.values[1] = 0;
				hdr_setting.values[2] = 0;
				hdr_setting.values[3] = 0;
				hdr_setting.values[4] = 0;
				setHDRMode((void*)&hdr_setting);
				set3ALock(0);
			}

			if (hdrStep4)
			{
				if(hdrStep3 == false)
				{
				    ALOGD("step 4");
					if(HDRMode)
					{
					    int frame_size = mFrameWidth * mFrameHeight * 3 >> 1;
						memcpy((void*)v4l2_buf.addrVirY, (void *)mImage3Yuv, frame_size);
						//hdrStep4 = false;
					}
					ret = OSAL_Queue(&mQueueBufferPicture, &mV4l2buf[v4l2_buf.index]);
					
					
					if (ret != 0)
					{
						LOGW("picture queue full");
						//pthread_cond_signal(&mPictureCond);
						pthread_cond_signal(&mSceneModePictureCond);
						goto DEC_REF;
					}

					set3ALock(0);
					// add reference count
					mV4l2buf[v4l2_buf.index].refCnt++;
					mTakePictureState = TAKE_PICTURE_NULL;  //stop take picture
					mIsPicCopy = false;
					//pthread_cond_signal(&mPictureCond);
					pthread_cond_signal(&mSceneModePictureCond);
					
					hdrStep4 = false;
					gettimeofday(&tv_end, NULL);
					float delta = float(tv_end.tv_sec-tv_begin.tv_sec)+float(tv_end.tv_usec-tv_begin.tv_usec)/1000000;
					LOGD("HDR time = %f s\n", delta);
					//mPicNum ++;
				}
				else
				{
				    ret = OSAL_Queue(&mQueueBufferPicture, &mV4l2buf[v4l2_buf.index]);
					
					if (ret != 0)
					{
						LOGW("picture queue full");
						//pthread_cond_signal(&mPictureCond);
						pthread_cond_signal(&mSceneModePictureCond);
						goto DEC_REF;
					}
					set3ALock(0);
					// add reference count
					mV4l2buf[v4l2_buf.index].refCnt++;
					//mTakePictureState = TAKE_PICTURE_NULL;  //stop take picture
					mIsPicCopy = false;
					pthread_cond_signal(&mSceneModePictureCond);

					gettimeofday(&tv_end, NULL);
					float delta = float(tv_end.tv_sec-tv_begin.tv_sec)+float(tv_end.tv_usec-tv_begin.tv_usec)/1000000;
					LOGD("HDR time = %f s\n", delta);
				}
			}

		}

		if (mTakePictureState == TAKE_PICTURE_FAST
			|| mTakePictureState == TAKE_PICTURE_RECORD)
		{
			if (mHalCameraInfo.fast_picture_mode)
			{
				if (buf.reserved == 0xFFFFFFFF)
				{
					goto DEC_REF;
				}
				int SnrValue = getSnrValue();
				int SnrMin = (SnrValue&0xff000000) >> 24;
				int SnrLevel = (SnrValue&0x00fff000) >> 12;
				if (SnrLevel != 0)
				{
					CaptureProcessing((unsigned char *)v4l2_buf.addrVirY, SnrMin,SnrLevel, mFrameWidth,mFrameHeight);
					LOGD("CaptureProcessing Min = %d, Level = %d!", SnrMin,SnrLevel);
				}
			}
			
			// enqueue picture buffer
			ret = OSAL_Queue(&mQueueBufferPicture, &mV4l2buf[v4l2_buf.index]);
			if (ret != 0)
			{
				LOGW("picture queue full");
				pthread_cond_signal(&mPictureCond);
				goto DEC_REF;
			}
			
			// add reference count
			mV4l2buf[v4l2_buf.index].refCnt++;
			mTakePictureState = TAKE_PICTURE_NULL;
			mIsPicCopy = false;
			pthread_cond_signal(&mPictureCond);
		}

		if (mTakePictureState == TAKE_PICTURE_SMART)
		{
			// enqueue picture buffer
			ret = OSAL_Queue(&mQueueBufferPicture, &mV4l2buf[v4l2_buf.index]);
			if (ret != 0)
			{
				LOGW("picture queue full");
				pthread_cond_signal(&mSmartPictureCond);
				goto DEC_REF;
			}
			
			// add reference count
			mV4l2buf[v4l2_buf.index].refCnt++;
			//mTakePictureState = TAKE_PICTURE_NULL;
			mIsPicCopy = false;
			pthread_cond_signal(&mSmartPictureCond);
		}
		
		if ((mTakePictureState == TAKE_PICTURE_CONTINUOUS
			|| mTakePictureState == TAKE_PICTURE_CONTINUOUS_FAST)
			&& isContinuousPictureTime())
		{
			// enqueue picture buffer
			ret = OSAL_Queue(&mQueueBufferPicture, &mV4l2buf[v4l2_buf.index]);
			if (ret != 0)
			{
				// LOGV("continuous picture queue full");
				pthread_cond_signal(&mContinuousPictureCond);
				goto DEC_REF;
			}

			// add reference count
			mV4l2buf[v4l2_buf.index].refCnt++;
			mIsPicCopy = false;
			pthread_cond_signal(&mContinuousPictureCond);
		}
	}

DEC_REF:
	pthread_mutex_unlock(&mCaptureMutex);
	
	releasePreviewFrame(v4l2_buf.index);
	
    return true;
}

bool V4L2CameraDevice::previewThread()
{
	V4L2BUF_t * pbuf = (V4L2BUF_t *)OSAL_Dequeue(&mQueueBufferPreview);
	if (pbuf == NULL)
	{
		// LOGV("picture queue no buffer, sleep...");
		pthread_mutex_lock(&mPreviewMutex);
		pthread_cond_wait(&mPreviewCond, &mPreviewMutex);
		pthread_mutex_unlock(&mPreviewMutex);
		return true;
	}

	Mutex::Autolock locker(&mObjectLock);
	if (mMapMem.mem[pbuf->index] == NULL
		|| pbuf->addrPhyY == 0)
	{
		LOGV("preview buffer have been released...");
		return true;
	}
	//showformat(pbuf->format,"CallBack");
	// callback
	mCallbackNotifier->onNextFrameAvailable((void*)pbuf, mUseHwEncoder);

	// preview
	// if (isPreviewTime())
	if (!mNeedHalfFrameRate || mShouldPreview)
	{
		mPreviewWindow->onNextFrameAvailable((void*)pbuf);
	}
	mShouldPreview = mShouldPreview ? false : true;

	// LOGD("preview id : %d", pbuf->index);

	releasePreviewFrame(pbuf->index);

	return true;
}

// singal picture
bool V4L2CameraDevice::pictureThread()
{
	V4L2BUF_t * pbuf = (V4L2BUF_t *)OSAL_Dequeue(&mQueueBufferPicture);
	if (pbuf == NULL)
	{
		LOGV("picture queue no buffer, sleep...");
		pthread_mutex_lock(&mPictureMutex);
		pthread_cond_wait(&mPictureCond, &mPictureMutex);
		pthread_mutex_unlock(&mPictureMutex);
		return true;
	}

	DBG_TIME_BEGIN("taking picture", 0);

	// notify picture cb
	mCameraHardware->notifyPictureMsg((void*)pbuf);

	DBG_TIME_DIFF("notifyPictureMsg");

	mCallbackNotifier->takePicture((void*)pbuf);
	
	char str[128];
	sprintf(str, "hw picture size: %dx%d", pbuf->width, pbuf->height);
	DBG_TIME_DIFF(str);
	
	if (!mIsPicCopy)
	{
		releasePreviewFrame(pbuf->index);
	}

	DBG_TIME_END("Take picture", 0);

	return true;
}

// blink picture
bool V4L2CameraDevice::smartPictureThread()
{
	V4L2BUF_t * pbuf = (V4L2BUF_t *)OSAL_Dequeue(&mQueueBufferPicture);

	if (pbuf == NULL)
	{
		LOGV("smartPictureThread queue no buffer, sleep...");
		pthread_mutex_lock(&mSmartPictureMutex);
		pthread_cond_wait(&mSmartPictureCond, &mSmartPictureMutex);
		pthread_mutex_unlock(&mSmartPictureMutex);
		return true;
	}
	
	// apk stop smart pictures
	if (mSmartPictureDone)
	{
		mTakePictureState = TAKE_PICTURE_NULL;
		if (!mIsPicCopy)
		{
			releasePreviewFrame(pbuf->index);
		}
		return true;
	}

    #if 0

	ALOGD("!! mCameraHardware->mBlinkPictureResult %d state %d", mCameraHardware->mBlinkPictureResult, mCameraHardware->mBlinkDetectionState);

	if ((mCameraHardware->mBlinkPictureResult == true) && (mCameraHardware->mBlinkDetectionState == FACE_DETECTION_PREPARED)) 
	{
		DBG_TIME_BEGIN("taking blink picture", 0);

		// notify picture cb
		mCameraHardware->notifyPictureMsg((void*)pbuf);

		DBG_TIME_DIFF("notifyPictureMsg");

		mCallbackNotifier->takePicture((void*)pbuf);

		stopSmartPicture();
        mTakePictureState = TAKE_PICTURE_NULL;
	}
	
	#endif

	#if 1

	ALOGD("!!! mCameraHardware->mSmilePictureResult %d, state %d", mCameraHardware->mSmilePictureResult, mCameraHardware->mSmileDetectionState);

	if ((mCameraHardware->mSmilePictureResult == true) && (mCameraHardware->mSmileDetectionState == FACE_DETECTION_PREPARED))
	{
		DBG_TIME_BEGIN("taking smile picture", 0);

		// notify picture cb
		mCameraHardware->notifyPictureMsg((void*)pbuf);

		DBG_TIME_DIFF("notifyPictureMsg");

		mCallbackNotifier->takePicture((void*)pbuf);

		stopSmartPicture();
        mTakePictureState = TAKE_PICTURE_NULL;
	}
	
	#endif

    #if 1

	if (mStartSmartTimeout == false)
	{
		if ((systemTime() / 1000000 - mStartSmartTimeMs) > 5000)	// 5s timeout
		{
			mStartSmartTimeout = true;

			DBG_TIME_BEGIN("taking smile picture time out!!!", 0);

			// notify picture cb
			mCameraHardware->notifyPictureMsg((void*)pbuf);

			DBG_TIME_DIFF("notifyPictureMsg");

			mCallbackNotifier->takePicture((void*)pbuf);

			stopSmartPicture();
	        mTakePictureState = TAKE_PICTURE_NULL;
		}
	}
	
	#endif

	char str[128];
	sprintf(str, "hw picture size: %dx%d", pbuf->width, pbuf->height);
	DBG_TIME_DIFF(str);
	
	if (!mIsPicCopy)
	{
		releasePreviewFrame(pbuf->index);
	}

	DBG_TIME_END("Take smart picture", 0);
	

	return true;
}

void V4L2CameraDevice::startSmartPicture()
{
	F_LOG;

	mSmartPictureDone = false;
	mStartSmartTimeout = false;
	mStartSmartTimeMs = systemTime() / 1000000;

	DBG_TIME_AVG_INIT(TAG_SMART_PICTURE);
}

void V4L2CameraDevice::stopSmartPicture()
{
	F_LOG;

	if (mSmartPictureDone)
	{
		LOGD("Smart picture has already stopped");
		return;
	}
	mStartSmartTimeout = true;
	mSmartPictureDone = true;

	DBG_TIME_AVG_END(TAG_SMART_PICTURE, "picture enc");
}

// scene mode picture
bool V4L2CameraDevice::sceneModePictureThread()
{
	V4L2BUF_t * pbuf = (V4L2BUF_t *)OSAL_Dequeue(&mQueueBufferPicture);

	if (pbuf == NULL)
	{
		LOGV("sceneModePictureThread queue no buffer, sleep...");
		pthread_mutex_lock(&mSceneModePictureMutex);
		pthread_cond_wait(&mSceneModePictureCond, &mSceneModePictureMutex);
		pthread_mutex_unlock(&mSceneModePictureMutex);
		return true;
	}
	
	// apk stop scene mode pictures
	if (mSceneModePictureDone)
	{
		mTakePictureState = TAKE_PICTURE_NULL;
		if (!mIsPicCopy)
		{
			releasePreviewFrame(pbuf->index);
		}
		return true;
	}

	if ((hdrStep2 == false) && (hdrStep3 == true))
	{
	    ALOGD("step 3");
		captureHDR((void *)mImage0Yuv,
				   (void *)mImage1Yuv,
				   (void *)mImage2Yuv,
				   (void *)mImage3Yuv,
				   &HDRMode,
				   hdrWidth,
				   hdrHeight,
				   gain_bright,
				   gain_dark);
	   hdrStep3 = false;
    }

    if (mTakePictureState == TAKE_PICTURE_NULL)
	{
		DBG_TIME_BEGIN("taking scene mode picture", 0);

		// notify picture cb
		mCameraHardware->notifyPictureMsg((void*)pbuf);

		DBG_TIME_DIFF("notifyPictureMsg");

		mCallbackNotifier->takePicture((void*)pbuf);

		stopSceneModePicture();
        mTakePictureState = TAKE_PICTURE_NULL;
	}
	
	char str[128];
	sprintf(str, "hw picture size: %dx%d", pbuf->width, pbuf->height);
	DBG_TIME_DIFF(str);
	
	if (!mIsPicCopy)
	{
		releasePreviewFrame(pbuf->index);
	}

	DBG_TIME_END("Take scene mode picture", 0);
	
	return true;
}

status_t V4L2CameraDevice::openSceneMode()
{
	F_LOG;
	
    status_t ret = 0;
	// used for HDR picture mode
	if (mImage0Yuv == NULL)
	    mImage0Yuv = (void *)malloc(MAX_PICTURE_SIZE);
	if (mImage1Yuv == NULL)
	    mImage1Yuv = (void *)malloc(MAX_PICTURE_SIZE);
	if (mImage2Yuv == NULL)
	    mImage2Yuv = (void *)malloc(MAX_PICTURE_SIZE);
	if (mImage3Yuv == NULL)
	    mImage3Yuv = (void *)malloc(MAX_PICTURE_SIZE);
	if (mImage4Yuv == NULL)
	    mImage4Yuv = (void *)malloc(MAX_PICTURE_SIZE);

	if((mImage0Yuv == NULL) || (mImage1Yuv == NULL) || (mImage2Yuv == NULL) || 
		(mImage3Yuv == NULL) || (mImage4Yuv == NULL))
	{
	    if (mImage0Yuv != NULL)
		{
			free(mImage0Yuv);
			mImage0Yuv = NULL;
		}
		if (mImage1Yuv != NULL)
		{
			free(mImage1Yuv);
			mImage1Yuv = NULL;
		}
		if (mImage2Yuv != NULL)
		{
			free(mImage2Yuv);
			mImage2Yuv = NULL;
		}
		if (mImage3Yuv != NULL)
		{
			free(mImage3Yuv);
			mImage3Yuv = NULL;
		}
		if (mImage4Yuv != NULL)
		{
			free(mImage4Yuv);
			mImage4Yuv = NULL;
		}
		ALOGE("open scene mode failed!!!");
        ret = -1;
	}
	
	DBG_TIME_AVG_INIT(TAG_SCENE_MODE);
	return ret;
}

void V4L2CameraDevice::closeSceneMode()
{
	F_LOG;

	if (mImage0Yuv != NULL)
	{
		free(mImage0Yuv);
		mImage0Yuv = NULL;
	}
	if (mImage1Yuv != NULL)
	{
		free(mImage1Yuv);
		mImage1Yuv = NULL;
	}
	if (mImage2Yuv != NULL)
	{
		free(mImage2Yuv);
		mImage2Yuv = NULL;
	}
	if (mImage3Yuv != NULL)
	{
		free(mImage3Yuv);
		mImage3Yuv = NULL;
	}
	if (mImage4Yuv != NULL)
	{
		free(mImage4Yuv);
		mImage4Yuv = NULL;
	}

	DBG_TIME_AVG_END(TAG_SCENE_MODE, "picture enc");
}


void V4L2CameraDevice::startSceneModePicture(int scenemode)
{
	F_LOG;
	mSceneModePicture = scenemode;
	
    setSceneMode(scenemode);
	hdrStep1 = true;
	hdrStep2 = true;
    hdrStep3 = true;
	hdrStep4 = true;
	mSceneModePictureDone = false;
	setSceneMode(scenemode);
		
	DBG_TIME_AVG_INIT(TAG_SCENE_MODE_PICTURE);
}

void V4L2CameraDevice::stopSceneModePicture()
{
	F_LOG;
	int scenemode = 0;
	mSceneModePicture = scenemode;
	hdrStep1 = false;
	hdrStep2 = false;
	hdrStep3 = false;
	hdrStep4 = false;
	mSceneModePictureDone = true;
	//setSceneMode(scenemode);

	DBG_TIME_AVG_END(TAG_SCENE_MODE_PICTURE, "picture enc");
}

void V4L2CameraDevice::setSceneMode(int scenemode)
{ 
    if (scenemode == V4L2_SCENE_MODE_HDR) {
		
        gettimeofday(&tv_begin, NULL);
		struct isp_hdr_setting_t hdr_setting;
		unsigned short *gamma;
		const double LOG_2 = log(2);
		int value_dark, value_bright;
		getAeStat(&AeBuf);
		getHistStat(&HistBuf);
		getGammaStat(&GammaBuf);
		gamma = (unsigned short*)GammaBuf.buf;
		LOGD("gamma  = %d,%d,%d,%d,%d,%d, %d", gamma[0], gamma[4],gamma[8],gamma[16],gamma[32],gamma[64],gamma[128]);
		GetExposureGain(mFrameWidth, mFrameHeight, (uint32_t *)AeBuf.buf, (uint32_t *)HistBuf.buf, &gain_dark, &gain_bright);
		value_dark = - int(25.0*log(1.0/gain_dark)/LOG_2+0.5);
		value_bright = int(25.0*log(gain_bright)/LOG_2+0.5);
		hdr_setting.hdr_en = 1;
		hdr_setting.total_frames = 2;
		hdr_setting.values[0] = value_dark;
		hdr_setting.values[1] = value_bright;
		hdr_setting.values[2] = 0;
		hdr_setting.values[3] = 0;
		hdr_setting.values[4] = 0;

		//TO DO: get/set GAIN and EXP
		LOGD("setHDRMode retrun = %d\n",setHDRMode((void*)&hdr_setting));
		free(AeBuf.buf);
		free(HistBuf.buf);					
		free(GammaBuf.buf);
		set3ALock(8);
		
    } else if(scenemode == 0) {
   
    }
	
}


// continuous picture
bool V4L2CameraDevice::continuousPictureThread()
{
	V4L2BUF_t * pbuf = (V4L2BUF_t *)OSAL_Dequeue(&mQueueBufferPicture);
	if (pbuf == NULL)
	{
		LOGV("continuousPictureThread queue no buffer, sleep...");
		pthread_mutex_lock(&mContinuousPictureMutex);
		pthread_cond_wait(&mContinuousPictureCond, &mContinuousPictureMutex);
		pthread_mutex_unlock(&mContinuousPictureMutex);
		return true;
	}
	
	Mutex::Autolock locker(&mObjectLock);
	if (mMapMem.mem[pbuf->index] == NULL
		|| pbuf->addrPhyY == 0)
	{
		LOGV("picture buffer have been released...");
		return true;
	}
	
	DBG_TIME_AVG_AREA_IN(TAG_CONTINUOUS_PICTURE);

	// reach the max number of pictures
	if (mContinuousPictureCnt >= mContinuousPictureMax)
	{
		mTakePictureState = TAKE_PICTURE_NULL;
		stopContinuousPicture();
		releasePreviewFrame(pbuf->index);
		return true;
	}

	// apk stop continuous pictures
	if (!mContinuousPictureStarted)
	{
		mTakePictureState = TAKE_PICTURE_NULL;
		releasePreviewFrame(pbuf->index);
		return true;
	}

	bool ret = mCallbackNotifier->takePicture((void*)pbuf, true);
	if (ret)
	{
		mContinuousPictureCnt++;
	
		DBG_TIME_AVG_AREA_OUT(TAG_CONTINUOUS_PICTURE);
	}
	else
	{
		// LOGW("do not encoder jpeg");
	}
	
	releasePreviewFrame(pbuf->index);

	return true;
}

void V4L2CameraDevice::startContinuousPicture()
{
	F_LOG;

	mContinuousPictureCnt = 0;
	mContinuousPictureStarted = true;
	mContinuousPictureStartTime = systemTime(SYSTEM_TIME_MONOTONIC);

	DBG_TIME_AVG_INIT(TAG_CONTINUOUS_PICTURE);
}

void V4L2CameraDevice::stopContinuousPicture()
{
	F_LOG;

	if (!mContinuousPictureStarted)
	{
		LOGD("Continuous picture has already stopped");
		return;
	}
	
	mContinuousPictureStarted = false;

	nsecs_t time = (systemTime(SYSTEM_TIME_MONOTONIC) - mContinuousPictureStartTime)/1000000;
	LOGD("Continuous picture cnt: %d, use time %lld(ms)", mContinuousPictureCnt, time);
	if (time != 0)
	{
		LOGD("Continuous picture %f(fps)", (float)mContinuousPictureCnt/(float)time * 1000);
	}

	DBG_TIME_AVG_END(TAG_CONTINUOUS_PICTURE, "picture enc");
}

void V4L2CameraDevice::setContinuousPictureCnt(int cnt)
{
	F_LOG;
	mContinuousPictureMax = cnt;
}

bool V4L2CameraDevice::isContinuousPictureTime()
{
	if (mTakePictureState == TAKE_PICTURE_CONTINUOUS_FAST)
	{
		return true;
	}
	
    timeval cur_time;
    gettimeofday(&cur_time, NULL);
    const uint64_t cur_mks = cur_time.tv_sec * 1000000LL + cur_time.tv_usec;
    if ((cur_mks - mContinuousPictureLast) >= mContinuousPictureAfter) {
        mContinuousPictureLast = cur_mks;
        return true;
    }
    return false;
}

bool V4L2CameraDevice::isPreviewTime()
{
	if (mVideoHint != true)
	{
		return true;
	}
	
    timeval cur_time;
    gettimeofday(&cur_time, NULL);
    const uint64_t cur_mks = cur_time.tv_sec * 1000000LL + cur_time.tv_usec;
    if ((cur_mks - mPreviewLast) >= mPreviewAfter) {
        mPreviewLast = cur_mks;
        return true;
    }
    return false;
}

void V4L2CameraDevice::waitFaceDectectTime()
{
    timeval cur_time;
    gettimeofday(&cur_time, NULL);
    const uint64_t cur_mks = cur_time.tv_sec * 1000000LL + cur_time.tv_usec;
	
    if ((cur_mks - mFaceDectectLast) >= mFaceDectectAfter)
	{
		mFaceDectectLast = cur_mks;
	}	
	else
	{
		usleep(mFaceDectectAfter - (cur_mks - mFaceDectectLast));
	    gettimeofday(&cur_time, NULL);
		mFaceDectectLast = cur_time.tv_sec * 1000000LL + cur_time.tv_usec;
	}
}

int V4L2CameraDevice::getCurrentFaceFrame(void* frame, int* width, int* height)
{
	if (frame == NULL)
	{
		LOGE("getCurrentFrame: error in null pointer");
		return -1;
	}

	pthread_mutex_lock(&mCaptureMutex);
	// stop capture
	if (mCaptureThreadState != CAPTURE_STATE_STARTED)
	{
		LOGW("capture thread dose not started");
		pthread_mutex_unlock(&mCaptureMutex);
		return -1;
	}
	pthread_mutex_unlock(&mCaptureMutex);
	
	//waitFaceDectectTime();

    Mutex::Autolock locker(&mObjectLock);
	
	if (mCurrentV4l2buf == NULL
		|| mCurrentV4l2buf->addrVirY == 0)
	{
		LOGW("frame buffer not ready");
		return -1;
	}

	if ((mCurrentV4l2buf->isThumbAvailable == 1)
		&& (mCurrentV4l2buf->thumbUsedForPreview == 1))
	{
	    #ifdef USE_ION_MEM_ALLOCATOR
		ion_flush_cache((void*)mCurrentV4l2buf->addrVirY + (ALIGN_16B(mCurrentV4l2buf->width) * mCurrentV4l2buf->height * 3 / 2), ALIGN_16B(mCurrentV4l2buf->thumbWidth) * mCurrentV4l2buf->thumbHeight);
		memcpy(frame, 
				(void*)mCurrentV4l2buf->addrVirY + ALIGN_4K((ALIGN_16B(mCurrentV4l2buf->width) * mCurrentV4l2buf->height * 3 / 2)), 
				ALIGN_16B(mCurrentV4l2buf->thumbWidth) * mCurrentV4l2buf->thumbHeight);
		*width = mCurrentV4l2buf->thumbWidth;
		*height = mCurrentV4l2buf->thumbHeight;
		#elif USE_SUNXI_MEM_ALLOCATOR
		sunxi_flush_cache((void*)mCurrentV4l2buf->addrVirY + ALIGN_4K((ALIGN_16B(mCurrentV4l2buf->width) * mCurrentV4l2buf->height * 3 / 2)), ALIGN_16B(mCurrentV4l2buf->thumbWidth) * mCurrentV4l2buf->thumbHeight);
		memcpy(frame, 
				(void*)mCurrentV4l2buf->addrVirY + (ALIGN_16B(mCurrentV4l2buf->width) * mCurrentV4l2buf->height * 3 / 2), 
				ALIGN_16B(mCurrentV4l2buf->thumbWidth) * mCurrentV4l2buf->thumbHeight);
		*width = mCurrentV4l2buf->thumbWidth;
		*height = mCurrentV4l2buf->thumbHeight;
		#endif
	}
	else
	{
	    #ifdef USE_ION_MEM_ALLOCATOR
		ion_flush_cache((void*)mCurrentV4l2buf->addrVirY, mCurrentV4l2buf->width * mCurrentV4l2buf->height);
		memcpy(frame, (void*)mCurrentV4l2buf->addrVirY, mCurrentV4l2buf->width * mCurrentV4l2buf->height);
		*width = mCurrentV4l2buf->width;
		*height = mCurrentV4l2buf->height;
		#elif USE_SUNXI_MEM_ALLOCATOR
		sunxi_flush_cache((void*)mCurrentV4l2buf->addrVirY, mCurrentV4l2buf->width * mCurrentV4l2buf->height);
		memcpy(frame, (void*)mCurrentV4l2buf->addrVirY, mCurrentV4l2buf->width * mCurrentV4l2buf->height);
		*width = mCurrentV4l2buf->width;
		*height = mCurrentV4l2buf->height;
		#endif
	}
	return 0;
}

// -----------------------------------------------------------------------------
// extended interfaces here <***** star *****>
// -----------------------------------------------------------------------------
int V4L2CameraDevice::openCameraDev(HALCameraInfo * halInfo)
{
	F_LOG;
	
	int ret = -1;
	struct v4l2_input inp;
	struct v4l2_capability cap; 
	char dev_node[16];

	if (halInfo == NULL)
	{
		LOGE("error HAL camera info");
		return -1;
	}
	
	// open V4L2 device
    //-----------------------------------------------
    if((access(halInfo->device_name, F_OK)) == 0)
    {
        strcpy(dev_node,halInfo->device_name);
    }
    else
    {
        for (int i = 0; i < MAX_NUM_OF_CAMERAS; i++)
        {
            sprintf(dev_node, "/dev/video%d", i);
            ret = access(dev_node, F_OK);
            if(ret == 0)
            {
                break;
            }
        }
    }
    mCameraFd = open(dev_node, O_RDWR | O_NONBLOCK, 0);
    if (mCameraFd == -1)
    {
        LOGE("ERROR opening %s: %s", dev_node, strerror(errno));
        return -1;
    }
    //-------------------------------------------------
	// check v4l2 device capabilities
	ret = ioctl (mCameraFd, VIDIOC_QUERYCAP, &cap); 
    if (ret < 0) 
	{ 
        LOGE("Error opening device: unable to query device."); 
        goto END_ERROR;
    } 

    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) 
	{ 
        LOGE("Error opening device: video capture not supported."); 
        goto END_ERROR;
    } 
  
    if ((cap.capabilities & V4L2_CAP_STREAMING) == 0) 
	{ 
        LOGE("Capture device does not support streaming i/o"); 
        goto END_ERROR;
    } 

	if (!strcmp((char *)cap.driver, "uvcvideo"))
	{
		mIsUsbCamera = true;
	}
	LOGD("The name of the Camera is '%s'",cap.card);

	if (!mIsUsbCamera)
	{
		// uvc do not need to set input
		inp.index = halInfo->device_id;
		if (-1 == ioctl (mCameraFd, VIDIOC_S_INPUT, &inp))
		{
			LOGE("VIDIOC_S_INPUT error!");
			goto END_ERROR;
		}
	}

	if (mIsUsbCamera)
	{
		// try to support this format: NV21, YUYV
		// we do not support mjpeg camera now
		if (tryFmt(V4L2_PIX_FMT_NV21) == OK)
		{
			mCaptureFormat = V4L2_PIX_FMT_NV21;
			LOGV("capture format: V4L2_PIX_FMT_NV21");
		}
		else if(tryFmt(V4L2_PIX_FMT_YUYV) == OK)
		{
			mCaptureFormat = V4L2_PIX_FMT_YUYV;		// maybe usb camera
			LOGV("capture format: V4L2_PIX_FMT_YUYV");
		}
		else
		{
			LOGE("driver should surpport NV21/NV12 or YUYV format, but it not!");
			goto END_ERROR;
		}
	}

	return OK;

END_ERROR:

	if (mCameraFd != NULL)
	{
		close(mCameraFd);
		mCameraFd = NULL;
	}
	
	return -1;
}

void V4L2CameraDevice::closeCameraDev()
{
	F_LOG;
	
	if (mCameraFd != NULL)
	{
		close(mCameraFd);
		mCameraFd = NULL;
	}
}

int V4L2CameraDevice::v4l2SetVideoParams(int width, int height, uint32_t pix_fmt)
{
	int ret = UNKNOWN_ERROR;
	struct v4l2_format format;

	LOGV("%s, line: %d, w: %d, h: %d, pfmt: %d", 
		__FUNCTION__, __LINE__, width, height, pix_fmt);
	
	memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    format.fmt.pix.width  = width;
    format.fmt.pix.height = height;
    if (mCaptureFormat == V4L2_PIX_FMT_YUYV)
	{
    	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; 
	}
	else
	{
		format.fmt.pix.pixelformat = pix_fmt; 
	}
	format.fmt.pix.field = V4L2_FIELD_NONE;

	struct v4l2_pix_format sub_fmt;
	if (mHalCameraInfo.fast_picture_mode)
	{
		memset(&sub_fmt, 0, sizeof(sub_fmt));
		
		int scale = getSuitableThumbScale(width, height);
		if (scale <= 0)
		{
			LOGE("error thumb scale: %d, src_size: %dx%d", scale, width, height);
			return UNKNOWN_ERROR;
		}
		
		format.fmt.pix.subchannel = &sub_fmt;
		format.fmt.pix.subchannel->width = format.fmt.pix.width / scale;
		format.fmt.pix.subchannel->height = format.fmt.pix.height / scale;
		format.fmt.pix.subchannel->pixelformat = pix_fmt;
		format.fmt.pix.subchannel->field = V4L2_FIELD_NONE;	
		LOGV("to camera params: w: %d, h: %d, sub: %dx%d, pfmt: %d, pfield: %d", 
			format.fmt.pix.width, format.fmt.pix.height,
			format.fmt.pix.subchannel->width, format.fmt.pix.subchannel->height, pix_fmt, V4L2_FIELD_NONE);
	}

	ret = ioctl(mCameraFd, VIDIOC_S_FMT, &format); 
	if (ret < 0) 
	{ 
		LOGE("VIDIOC_S_FMT Failed: %s", strerror(errno)); 
		return ret; 
	} 

	mFrameWidth = format.fmt.pix.width;
	mFrameHeight = format.fmt.pix.height;

	if (mHalCameraInfo.fast_picture_mode)
	{
		mThumbWidth = format.fmt.pix.subchannel->width;
		mThumbHeight = format.fmt.pix.subchannel->height;
	}
	
	LOGV("camera params: w: %d, h: %d, sub: %dx%d, pfmt: %d, pfield: %d", 
		mFrameWidth, mFrameHeight, mThumbWidth, mThumbHeight, pix_fmt, V4L2_FIELD_NONE);

	return OK;
}

int V4L2CameraDevice::v4l2ReqBufs(int * buf_cnt)
{
	F_LOG;
	int ret = UNKNOWN_ERROR;
	struct v4l2_requestbuffers rb;

	LOGV("TO VIDIOC_REQBUFS count: %d", *buf_cnt);
	
	memset(&rb, 0, sizeof(rb));
    rb.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    rb.memory = V4L2_MEMORY_MMAP; 
    rb.count  = *buf_cnt; 
	
	ret = ioctl(mCameraFd, VIDIOC_REQBUFS, &rb); 
    if (ret < 0) 
	{ 
        LOGE("Init: VIDIOC_REQBUFS failed: %s", strerror(errno)); 
		return ret;
    } 

	*buf_cnt = rb.count;
	LOGV("VIDIOC_REQBUFS count: %d", *buf_cnt);

	return OK;
}

int V4L2CameraDevice::v4l2QueryBuf()
{
	F_LOG;
	int ret = UNKNOWN_ERROR;
	struct v4l2_buffer buf;
	
	for (int i = 0; i < mBufferCnt; i++) 
	{  
        memset (&buf, 0, sizeof (struct v4l2_buffer)); 
		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
		buf.memory = V4L2_MEMORY_MMAP; 
		buf.index  = i; 
		
		ret = ioctl (mCameraFd, VIDIOC_QUERYBUF, &buf); 
        if (ret < 0) 
		{ 
            LOGE("Unable to query buffer (%s)", strerror(errno)); 
            return ret; 
        } 

 
        mMapMem.mem[i] = mmap (0, buf.length, 
                            PROT_READ | PROT_WRITE, 
                            MAP_SHARED, 
                            mCameraFd, 
                            buf.m.offset); 
		mMapMem.length = buf.length;
		LOGV("index: %d, mem: %x, len: %x, offset: %x", i, (int)mMapMem.mem[i], buf.length, buf.m.offset);
 
        if (mMapMem.mem[i] == MAP_FAILED) 
		{ 
			LOGE("Unable to map buffer (%s)", strerror(errno)); 
			for(int j = 0;j < i;j++){
				munmap(mMapMem.mem[j], mMapMem.length);
			}
            return -1; 
        } 

		// start with all buffers in queue
        ret = ioctl(mCameraFd, VIDIOC_QBUF, &buf); 
        if (ret < 0) 
		{ 
            LOGE("VIDIOC_QBUF Failed"); 
            return ret; 
        } 

		if (mIsUsbCamera)		// star to do
		{
			int buffer_len = mFrameWidth * mFrameHeight * 3 / 2;
		
		    #ifdef USE_ION_MEM_ALLOCATOR
			mVideoBuffer.buf_vir_addr[i] = (int)ion_alloc_alloc(buffer_len);
			mVideoBuffer.buf_phy_addr[i] = ion_alloc_vir2phy((void*)mVideoBuffer.buf_vir_addr[i]);
	            #elif USE_SUNXI_MEM_ALLOCATOR
			mVideoBuffer.buf_vir_addr[i] = (int)sunxi_alloc_alloc(buffer_len);
			mVideoBuffer.buf_phy_addr[i] = sunxi_alloc_vir2phy((void*)mVideoBuffer.buf_vir_addr[i]);
		    #endif
			LOGV("video buffer: index: %d, vir: %x, phy: %x, len: %x", 
					i, mVideoBuffer.buf_vir_addr[i], mVideoBuffer.buf_phy_addr[i], buffer_len);
			
			memset((void*)mVideoBuffer.buf_vir_addr[i], 0x10, mFrameWidth * mFrameHeight);
			memset((void*)mVideoBuffer.buf_vir_addr[i] + mFrameWidth * mFrameHeight, 
					0x80, mFrameWidth * mFrameHeight / 2);
		}
	} 

	return OK;
}

int V4L2CameraDevice::v4l2StartStreaming()
{
	F_LOG;
	int ret = UNKNOWN_ERROR; 
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	
  	ret = ioctl (mCameraFd, VIDIOC_STREAMON, &type); 
	if (ret < 0) 
	{ 
		LOGE("StartStreaming: Unable to start capture: %s", strerror(errno)); 
		return ret; 
	} 

	return OK;
}

int V4L2CameraDevice::v4l2StopStreaming()
{
	F_LOG;
	int ret = UNKNOWN_ERROR; 
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	
	ret = ioctl (mCameraFd, VIDIOC_STREAMOFF, &type); 
	if (ret < 0) 
	{ 
		LOGE("StopStreaming: Unable to stop capture: %s", strerror(errno)); 
		return ret; 
	} 
	LOGV("V4L2Camera::v4l2StopStreaming OK");

	return OK;
}

int V4L2CameraDevice::v4l2UnmapBuf()
{
	F_LOG;
	int ret = UNKNOWN_ERROR;
	
	for (int i = 0; i < mBufferCnt; i++) 
	{
		ret = munmap(mMapMem.mem[i], mMapMem.length);
        if (ret < 0) 
		{
            LOGE("v4l2CloseBuf Unmap failed"); 
			return ret;
		}
		mMapMem.mem[i] = NULL;

		if (mVideoBuffer.buf_vir_addr[i] != 0)
		{
		    #ifdef USE_ION_MEM_ALLOCATOR
			ion_alloc_free((void*)mVideoBuffer.buf_vir_addr[i]);
			mVideoBuffer.buf_phy_addr[i] = 0;
		    #elif USE_SUNXI_MEM_ALLOCATOR
			sunxi_alloc_free((void*)mVideoBuffer.buf_vir_addr[i]);
			mVideoBuffer.buf_phy_addr[i] = 0;
		    #endif
		}
	}
	mVideoBuffer.buf_unused = NB_BUFFER;
	mVideoBuffer.read_id = 0;
	mVideoBuffer.read_id = 0;
	
	return OK;
}

void V4L2CameraDevice::releasePreviewFrame(int index)
{
	int ret = UNKNOWN_ERROR;
	struct v4l2_buffer buf;
	
	pthread_mutex_lock(&mCaptureMutex);

	// decrease buffer reference count first, if the reference count is no more than 0, release it.
	if (mV4l2buf[index].refCnt > 0
		&& --mV4l2buf[index].refCnt == 0)
	{
		memset(&buf, 0, sizeof(v4l2_buffer));
		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	    buf.memory = V4L2_MEMORY_MMAP; 
		buf.index = index;
		
		// LOGD("r ID: %d", buf.index);
	    ret = ioctl(mCameraFd, VIDIOC_QBUF, &buf); 
	    if (ret != 0) 
		{
	        LOGE("releasePreviewFrame: VIDIOC_QBUF Failed: index = %d, ret = %d, %s", 
				buf.index, ret, strerror(errno)); 
	    }
		else
		{
			mCurAvailBufferCnt++;
		}
	}

	pthread_mutex_unlock(&mCaptureMutex);
}

int V4L2CameraDevice::getPreviewFrame(v4l2_buffer *buf)
{
	int ret = UNKNOWN_ERROR;
	
	buf->type   = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    buf->memory = V4L2_MEMORY_MMAP; 
 
    ret = ioctl(mCameraFd, VIDIOC_DQBUF, buf); 
    if (ret < 0) 
	{ 
        LOGW("GetPreviewFrame: VIDIOC_DQBUF Failed, %s", strerror(errno)); 
        return __LINE__; 			// can not return false
    }

	return OK;
}

int V4L2CameraDevice::tryFmt(int format)
{	
	struct v4l2_fmtdesc fmtdesc;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	for(int i = 0; i < 12; i++)
	{
		fmtdesc.index = i;
		if (-1 == ioctl (mCameraFd, VIDIOC_ENUM_FMT, &fmtdesc))
		{
			break;
		}
		LOGV("format index = %d, name = %s, v4l2 pixel format = %x\n",
			i, fmtdesc.description, fmtdesc.pixelformat);

		if (fmtdesc.pixelformat == format)
		{
			return OK;
		}
	}

	return -1;
}

int V4L2CameraDevice::tryFmtSize(int * width, int * height)
{
	F_LOG;
	int ret = -1;
	struct v4l2_format fmt;

	LOGV("V4L2Camera::TryFmtSize: w: %d, h: %d", *width, *height);


	memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    fmt.fmt.pix.width  = *width; 
    fmt.fmt.pix.height = *height; 
    if (mCaptureFormat == V4L2_PIX_FMT_YUYV)
	{
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	}
	else
	{
    	fmt.fmt.pix.pixelformat = mVideoFormat; 
	}
	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	ret = ioctl(mCameraFd, VIDIOC_TRY_FMT, &fmt); 
	if (ret < 0) 
	{ 
		LOGE("VIDIOC_TRY_FMT Failed: %s", strerror(errno)); 
		return ret; 
	} 

	// driver surpport this size
	*width = fmt.fmt.pix.width; 
    *height = fmt.fmt.pix.height; 

	return 0;
}

int V4L2CameraDevice::setFrameRate(int rate)
{
	mFrameRate = rate;
	return OK;
}

int V4L2CameraDevice::getFrameRate()
{
	F_LOG;
	int ret = -1;

	struct v4l2_streamparm parms;
	parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl (mCameraFd, VIDIOC_G_PARM, &parms);
	if (ret < 0) 
	{
		LOGE("VIDIOC_G_PARM getFrameRate error, %s", strerror(errno));
		return ret;
	}

	int numerator = parms.parm.capture.timeperframe.numerator;
	int denominator = parms.parm.capture.timeperframe.denominator;
	
	LOGV("frame rate: numerator = %d, denominator = %d", numerator, denominator);

	if (numerator != 0
		&& denominator != 0)
	{
		return denominator / numerator;
	}
	else
	{
		LOGW("unsupported frame rate: %d/%d", denominator, numerator);
		return 30;
	}
}

int V4L2CameraDevice::setImageEffect(int effect)
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_COLORFX;
	ctrl.value = effect;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGV("setImageEffect failed!");
	else 
		LOGV("setImageEffect ok");

	return ret;
}

int V4L2CameraDevice::setWhiteBalance(int wb)
{
	struct v4l2_control ctrl;
	int ret = -1;

	ctrl.id = V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE;
	ctrl.value = wb;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGV("setWhiteBalance failed, %s", strerror(errno));
	else 
		LOGV("setWhiteBalance ok");

	return ret;
}

int V4L2CameraDevice::setTakePictureCtrl()
{
	struct v4l2_control ctrl;
	int ret = -1;
	if (mHalCameraInfo.fast_picture_mode){
		ctrl.id = V4L2_CID_TAKE_PICTURE;
		ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
		if (ret < 0)
			LOGV("setTakePictureCtrl failed, %s", strerror(errno));
		else 
			LOGV("setTakePictureCtrl ok");

		return ret;
	}
	return 0;
}

// ae mode
int V4L2CameraDevice::setExposureMode(int mode)
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_EXPOSURE_AUTO;
	ctrl.value = mode;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGV("setExposureMode failed, %s", strerror(errno));
	else 
		LOGV("setExposureMode ok");

	return ret;
}

// ae compensation
int V4L2CameraDevice::setExposureCompensation(int val)
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_AUTO_EXPOSURE_BIAS;
	ctrl.value = val;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGV("setExposureCompensation failed, %s", strerror(errno));
	else 
		LOGV("setExposureCompensation ok");

	return ret;
}

// ae compensation
int V4L2CameraDevice::setExposureWind(int num, void *wind)
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_AUTO_EXPOSURE_WIN_NUM;
	ctrl.value = num;
	ctrl.user_pt = (unsigned int)wind;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGE("setExposureWind failed, %s", strerror(errno));
	else 
		LOGV("setExposureWind ok");

	return ret;
}

// flash mode
int V4L2CameraDevice::setFlashMode(int mode)
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_FLASH_LED_MODE;
	ctrl.value = mode;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGV("setFlashMode failed, %s", strerror(errno));
	else 
		LOGV("setFlashMode ok");

	return ret;
}

// af init
int V4L2CameraDevice::setAutoFocusInit()
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_AUTO_FOCUS_INIT;
	ctrl.value = 0;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGE("setAutoFocusInit failed, %s", strerror(errno));
	else 
		LOGV("setAutoFocusInit ok");

	return ret;
}

// af release
int V4L2CameraDevice::setAutoFocusRelease()
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_AUTO_FOCUS_RELEASE;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGE("setAutoFocusRelease failed, %s", strerror(errno));
	else 
		LOGV("setAutoFocusRelease ok");

	return ret;
}

// af range
int V4L2CameraDevice::setAutoFocusRange(int af_range)
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_FOCUS_AUTO;
	ctrl.value = 1;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGV("setAutoFocusRange id V4L2_CID_FOCUS_AUTO failed, %s", strerror(errno));
	else 
		LOGV("setAutoFocusRange id V4L2_CID_FOCUS_AUTO ok");

	ctrl.id = V4L2_CID_AUTO_FOCUS_RANGE;
	ctrl.value = af_range;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGV("setAutoFocusRange id V4L2_CID_AUTO_FOCUS_RANGE failed, %s", strerror(errno));
	else 
		LOGV("setAutoFocusRange id V4L2_CID_AUTO_FOCUS_RANGE ok");

	return ret;
}

// af wind
int V4L2CameraDevice::setAutoFocusWind(int num, void *wind)
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_AUTO_FOCUS_WIN_NUM;
	ctrl.value = num;
	ctrl.user_pt = (unsigned int)wind;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGE("setAutoFocusCtrl failed, %s", strerror(errno));
	else 
		LOGV("setAutoFocusCtrl ok");

	return ret;
}

// af start
int V4L2CameraDevice::setAutoFocusStart()
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_AUTO_FOCUS_START;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGE("setAutoFocusStart failed, %s", strerror(errno));
	else 
		LOGV("setAutoFocusStart ok");

	return ret;
}

// af stop
int V4L2CameraDevice::setAutoFocusStop()
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_AUTO_FOCUS_STOP;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGE("setAutoFocusStart failed, %s", strerror(errno));
	else 
		LOGV("setAutoFocusStart ok");

	return ret;
}

// get af statue
int V4L2CameraDevice::getAutoFocusStatus()
{
	//F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;

	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}

	ctrl.id = V4L2_CID_AUTO_FOCUS_STATUS;
	ret = ioctl(mCameraFd, VIDIOC_G_CTRL, &ctrl);
	if (ret >= 0)
	{
		//LOGV("getAutoFocusCtrl ok");
	}
	
	return ret;
}

int V4L2CameraDevice::getSnrValue()
{
	//F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;
	struct v4l2_queryctrl qc_ctrl;

	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}

	ctrl.id = V4L2_CID_GAIN;
	qc_ctrl.id = V4L2_CID_GAIN;

	if (-1 == ioctl (mCameraFd, VIDIOC_QUERYCTRL, &qc_ctrl))
    {
	   return 0;
    }
	
	ret = ioctl(mCameraFd, VIDIOC_G_CTRL, &ctrl);
	return ctrl.value;
}



int V4L2CameraDevice::getGainValue() //get gain (trait specially, need the last 8 bits)
{
	//F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;
	struct v4l2_queryctrl qc_ctrl;

	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}

	ctrl.id = V4L2_CID_GAIN;
	qc_ctrl.id = V4L2_CID_GAIN;

	if (-1 == ioctl (mCameraFd, VIDIOC_QUERYCTRL, &qc_ctrl))
    {
	   return 0;
    }
	
	ret = ioctl(mCameraFd, VIDIOC_G_CTRL, &ctrl);
	ctrl.value = ctrl.value &0xff;
	return ctrl.value;
}

int V4L2CameraDevice::getExpValue() //get gain (trait specially, need the last 8 bits)
{
	//F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;
	struct v4l2_queryctrl qc_ctrl;

	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}

	ctrl.id = V4L2_CID_EXPOSURE;
	qc_ctrl.id = V4L2_CID_EXPOSURE;

	if (-1 == ioctl (mCameraFd, VIDIOC_QUERYCTRL, &qc_ctrl))
    {
	   return 0;
    }
	
	ret = ioctl(mCameraFd, VIDIOC_G_CTRL, &ctrl);
	return ctrl.value;
}

int V4L2CameraDevice::setHDRMode(void *hdr_setting)
{
	int ret = -1;
	struct v4l2_control ctrl;
	struct v4l2_queryctrl qc_ctrl;
	
	ctrl.value = 0;
	ctrl.user_pt = (unsigned int)hdr_setting;
	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}
	ctrl.id = V4L2_CID_HDR;
	qc_ctrl.id = V4L2_CID_HDR;

	if (-1 == ioctl (mCameraFd, VIDIOC_QUERYCTRL, &qc_ctrl))
    {
	   return 0;
    }
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	return ret;
}

int V4L2CameraDevice::getHDRFrameCnt()
{
	int ret = -1;
	struct v4l2_control ctrl;
	struct v4l2_queryctrl qc_ctrl;
	
	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}

	ctrl.id = V4L2_CID_HDR;	
	qc_ctrl.id = V4L2_CID_HDR;

	if (-1 == ioctl (mCameraFd, VIDIOC_QUERYCTRL, &qc_ctrl))
    {
	   return 0;
    }

	ret = ioctl(mCameraFd, VIDIOC_G_CTRL, &ctrl);
	
	return ctrl.value;
}

int V4L2CameraDevice::getAeStat(struct isp_stat_buf *AeBuf)
{
	int ret = -1;
	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}
	AeBuf->buf = malloc(0xc00);
	memset(AeBuf->buf,0,0xc00);	
	LOGD("AeBuf->buf == %x\n",AeBuf->buf);
	if(AeBuf->buf == NULL){
		return -1;
	}
	ret = ioctl(mCameraFd, VIDIOC_ISP_AE_STAT_REQ, AeBuf);
	return ret;
}
int V4L2CameraDevice::getGammaStat(struct isp_stat_buf *GammaBuf)
{
	int ret = -1;
	#if 1
	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}
	GammaBuf->buf = malloc(0x200);
	memset(GammaBuf->buf,0,0x200);	
	LOGD("GammaBuf->buf == %x\n",GammaBuf->buf);
	if(GammaBuf->buf == NULL){
		return -1;
	}
	//ret = ioctl(mCameraFd, VIDIOC_ISP_GAMMA_REQ, GammaBuf);
	#endif
	return ret;
}

int V4L2CameraDevice::getHistStat(struct isp_stat_buf *HistBuf)
{
	int ret = -1;
	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}	
	HistBuf->buf = malloc(0x200);
	memset(HistBuf->buf,0,0x200);
	if(HistBuf->buf == NULL){
		return -1;
	}
	ret = ioctl(mCameraFd, VIDIOC_ISP_HIST_STAT_REQ, HistBuf);
	return ret;
}

int V4L2CameraDevice::setGainValue(int Gain)
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;	

	ctrl.id = V4L2_CID_GAIN;
	ctrl.value = Gain;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGE("setGain failed, %s", strerror(errno));
	else 
		LOGV("setGain ok");
	return ret;
}

int V4L2CameraDevice::setExpValue(int Exp)
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;
	ctrl.id = V4L2_CID_EXPOSURE;	
	ctrl.value = Exp;
	
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
		LOGE("Set V4L2_CID_EXPOSURE failed, %s", strerror(errno));
	else 
		LOGV("Set V4L2_CID_EXPOSURE ok");

	return ret;
}



int V4L2CameraDevice::set3ALock(int lock)
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;
	
	ctrl.id = V4L2_CID_3A_LOCK;
	ctrl.value = lock;
	ret = ioctl(mCameraFd, VIDIOC_S_CTRL, &ctrl);
	if (ret >= 0)
		LOGV("set3ALock ok");

	return ret;
}

int V4L2CameraDevice::v4l2setCaptureParams()
{
	F_LOG;
	int ret = -1;
	
	struct v4l2_streamparm params;
	params.parm.capture.timeperframe.numerator = 1;
	params.parm.capture.timeperframe.denominator = mFrameRate;
	params.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (mTakePictureState == TAKE_PICTURE_NORMAL)
	{
		params.parm.capture.capturemode = V4L2_MODE_IMAGE;
	}
	else
	{
		if(mVideoHint == true)
		{
			params.parm.capture.capturemode = V4L2_MODE_VIDEO;
		}
		else
		{
			params.parm.capture.capturemode = V4L2_MODE_PREVIEW;
		}
	}

	LOGV("VIDIOC_S_PARM mFrameRate: %d, capture mode: %d", mFrameRate, params.parm.capture.capturemode);

	ret = ioctl(mCameraFd, VIDIOC_S_PARM, &params);
	if (ret < 0)
		LOGE("v4l2setCaptureParams failed, %s", strerror(errno));
	else 
		LOGV("v4l2setCaptureParams ok");

	return ret;
}

int V4L2CameraDevice::enumSize(char * pSize, int len)
{
	F_LOG;

	struct v4l2_frmsizeenum size_enum;
	size_enum.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	size_enum.pixel_format = mCaptureFormat;

	if (pSize == NULL)
	{
		LOGE("error input params");
		return -1;
	}

	char str[16];
	memset(str, 0, 16);
	memset(pSize, 0, len);
	
	for(int i = 0; i < 20; i++)
	{
		size_enum.index = i;
		if (-1 == ioctl (mCameraFd, VIDIOC_ENUM_FRAMESIZES, &size_enum))
		{
			break;
		}
		// LOGV("format index = %d, size_enum: %dx%d", i, size_enum.discrete.width, size_enum.discrete.height);
		sprintf(str, "%dx%d", size_enum.discrete.width, size_enum.discrete.height);
		if (i != 0)
		{
			strcat(pSize, ",");
		}
		strcat(pSize, str);
	}

	return OK;
}

int V4L2CameraDevice::getFullSize(int * full_w, int * full_h)
{
	F_LOG;

	struct v4l2_frmsizeenum size_enum;
	size_enum.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	size_enum.pixel_format = mCaptureFormat;
	size_enum.index = 0;
	if (-1 == ioctl (mCameraFd, VIDIOC_ENUM_FRAMESIZES, &size_enum))
	{
		LOGE("getFullSize failed");
		return -1;
	}
	
	*full_w = size_enum.discrete.width;
	*full_h = size_enum.discrete.height;

	LOGV("getFullSize: %dx%d", *full_w, *full_h);
	
	return OK;
}

int V4L2CameraDevice::getSuitableThumbScale(int full_w, int full_h)
{
	F_LOG;

	int scale = 1;
	if(mIsThumbUsedForVideo == true)
	{
		scale = 2;
	}
	//TODO: Get the screen size to calculate the scaler factor
	if (full_w*full_h > 10*1024*1024)		//maybe 12m,13m,16m
		return 2;
	else if(full_w*full_h > 4.5*1024*1024)	//maybe 5m,8m
		return 2;
	else return scale;						//others
#if 0
	if ((full_w == 4608)
		&& (full_h == 3456))
	{
		return 4;	// 1000x750
	}
	if ((full_w == 3840)
		&& (full_h == 2160))
	{
		return 4;	// 1000x750
	}
	if ((full_w == 4000)
		&& (full_h == 3000))
	{
		return 4;	// 1000x750
	}
	else if ((full_w == 3264)
		&& (full_h == 2448))
	{
		return 2;	// 1632x1224
	}
	else if ((full_w == 2592)
		&& (full_h == 1936))
	{
		return 2;	// 1296x968
	}
	else if ((full_w == 1280)
		&& (full_h == 960))
	{
		return 1 * scale;	// 1280x960
	}
	else if ((full_w == 1920)
		&& (full_h == 1080))
	{
		return 2;	// 960x540
	}
	else if ((full_w == 1280)
		&& (full_h == 720))
	{
		
		return 1 * scale;	// 1280x720
	}
	else if ((full_w == 640)
		&& (full_h == 480))
	{
		return 1;	// 640x480
	}

	LOGW("getSuitableThumbScale unknown size: %dx%d", full_w, full_h);
	return 1;		// failed
#endif
}
int V4L2CameraDevice::getSensorType()
{
	F_LOG;
	int ret = -1;
	struct v4l2_control ctrl;
	struct v4l2_queryctrl qc_ctrl;

	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}

	ctrl.id = V4L2_CID_SENSOR_TYPE;
	qc_ctrl.id = V4L2_CID_SENSOR_TYPE;

	if (-1 == ioctl (mCameraFd, VIDIOC_QUERYCTRL, &qc_ctrl))
	{
		LOGE("query sensor type ctrl failed");
		return -1;
	}
	ret = ioctl(mCameraFd, VIDIOC_G_CTRL, &ctrl);
	return ctrl.value;
}
int V4L2CameraDevice::getExifInfo(struct isp_exif_attribute *exif_attri)
{
	int ret = -1;
	if (mCameraFd == NULL)
	{
		return 0xFF000000;
	}
	ret = ioctl(mCameraFd, VIDIOC_ISP_EXIF_REQ, exif_attri);
	return ret;
}


}; /* namespace android */
