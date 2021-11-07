package com.liyaan.gifpalyer

import android.graphics.Bitmap


class GifHandler(path:String) {

    init {
        System.loadLibrary("native-lib")
    }
    private var gifAddr:Long = loadPath(path)

    fun getWidth(): Int {
        return getWidth(gifAddr)
    }

    fun getHeight(): Int {
        return getHeight(gifAddr)
    }

    fun updateFrame(bitmap: Bitmap?): Int {
        return updateFrame(gifAddr, bitmap!!)
    }

    private external fun loadPath(path: String): Long

    external fun getWidth(ndkGif:Long):Int
    external fun getHeight(ndkGif:Long):Int
    external fun updateFrame(ndkGif:Long,bitmap:Bitmap):Int
}