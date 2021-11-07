#include <jni.h>
#include <string>
#include <malloc.h>
#include <cstring>
#include "gif_lib.h"
#include <android/log.h>
#include <android/bitmap.h>
#define  LOG_TAG    "wangyi"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)

typedef struct GifBean{
    int current_frame;
    int total_frame;
    int *dealys;
} GifBean;

void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels){
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];
    int *px = (int *)pixels;
    int pointPixels;
    GifImageDesc frameInfo = savedImage.ImageDesc;
    GifByteType gifByteType;
    ColorMapObject *colorMapObject = frameInfo.ColorMap;
    px = (int *)((char *)px+info.stride*frameInfo.Top);
    int *line;
    for(int y = frameInfo.Top;y<frameInfo.Top+frameInfo.Height;++y){
        line = px;
        for(int x = frameInfo.Left;x<frameInfo.Left+frameInfo.Width;++x){
            pointPixels = (y-frameInfo.Top)*frameInfo.Width+(x-frameInfo.Width);
            gifByteType = savedImage.RasterBits[pointPixels];
            GifColorType  gifColorType = colorMapObject->Colors[gifByteType];
            line[x] = argb(255,gifColorType.Red,gifColorType.Green,gifColorType.Blue);
        }
        px = (int *)((char*)px + info.stride);
    }
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_liyaan_gifpalyer_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_liyaan_gifpalyer_GifHandler_loadPath(JNIEnv *env, jobject thiz, jstring path_) {
    const char *path = env->GetStringUTFChars(path_,0);
    int err;
    GifFileType *gifFileType = DGifOpenFileName(path,&err);
    DGifSlurp(gifFileType);
    GifBean *gifBean = (GifBean *)malloc(sizeof(GifBean));
    memset(gifBean,0,sizeof(GifBean));
    gifFileType->UserData = gifBean;
    gifBean->dealys = (int *)malloc(sizeof(int)*gifFileType->ImageCount);
    memset(gifBean->dealys,0,sizeof(int)*gifFileType->ImageCount);
    gifFileType->UserData = gifBean;
    gifBean->current_frame = 0;
    gifBean->total_frame = gifFileType->ImageCount;
    ExtensionBlock * ext;
    for(int i=0;i<gifFileType->ImageCount;++i){
        SavedImage frame = gifFileType->SavedImages[i];
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            if (frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                ext = &frame.ExtensionBlocks[j];
                break;
            }
        }
        if (ext) {
//	Delay Time - 单位1/100秒   1s/100
            int frame_delay =10*(ext->Bytes[1] | (ext->Bytes[2] << 8));//ms
            LOGE("时间  %d   ",frame_delay);
            gifBean->dealys[i] = frame_delay;
        }
    }
    LOGE("gif  长度大小    %d  ",gifFileType->ImageCount);

    env->ReleaseStringUTFChars(path_, path);
    return (jlong)gifFileType;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_liyaan_gifpalyer_GifHandler_getWidth(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    GifFileType *gifFileType = (GifFileType *)ndk_gif;
    return gifFileType->SWidth;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_liyaan_gifpalyer_GifHandler_getHeight(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    GifFileType *gifFileType = (GifFileType *)ndk_gif;
    return gifFileType->SHeight;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_liyaan_gifpalyer_GifHandler_updateFrame(JNIEnv *env, jobject thiz, jlong ndk_gif,
                                                 jobject bitmap) {
    GifFileType *gifFileType = (GifFileType *)ndk_gif;
    GifBean *gifBean = (GifBean *)gifFileType->UserData;
    AndroidBitmapInfo info;
    AndroidBitmap_getInfo(env,bitmap,&info);
    void *pixels;
    AndroidBitmap_lockPixels(env,bitmap,&pixels);
    drawFrame(gifFileType,gifBean,info,pixels);
    gifBean->current_frame+=1;
    if (gifBean->current_frame>=gifBean->total_frame-1){
        gifBean->current_frame = 0;
        LOGE("重新过来  %d  ",gifBean->current_frame);
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    return gifBean->dealys[gifBean->current_frame];
}