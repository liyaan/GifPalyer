package com.liyaan.gifpalyer

import android.app.Activity
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.os.*
import android.util.Log
import android.view.View
import android.widget.ImageView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import java.io.File


class MainActivity : AppCompatActivity() {


    private var mFile:File? = null
    private var gifHandler: GifHandler? = null
    private var bitmap:Bitmap? = null
    private var imageView:ImageView? = null

    private val PERMISSIONS_STORAGE = arrayOf(
        "android.permission.READ_EXTERNAL_STORAGE",
        "android.permission.WRITE_EXTERNAL_STORAGE" )

    private val handler = object:Handler(Looper.getMainLooper()){
        override fun handleMessage(msg: Message) {
            val mNextFrame = gifHandler!!.updateFrame(bitmap)
            sendEmptyMessageDelayed(1, mNextFrame!!.toLong())
            imageView?.setImageBitmap(bitmap)

        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        imageView = findViewById(R.id.imageView)
        verifyStoragePermissions(this)        // Example of a call to a native method
        findViewById<TextView>(R.id.sample_text).text = stringFromJNI()
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }

    fun ndkLoadGif(view: View) {
        if (mFile==null){
            mFile = File(Environment.getExternalStorageDirectory(), "demo.gif")
            Log.i("aaaaa",mFile!!.absolutePath)
            gifHandler = GifHandler(mFile!!.absolutePath)
            val width = gifHandler?.getWidth()
            val height = gifHandler?.getHeight()
            bitmap = Bitmap.createBitmap(width!!,height!!,Bitmap.Config.ARGB_8888)
            val nextFrame = gifHandler?.updateFrame(bitmap)
            handler.sendEmptyMessageDelayed(1,nextFrame!!.toLong())
        }

    }

    /*
     * android 动态权限申请
     * */
    fun verifyStoragePermissions(activity: AppCompatActivity?) {
        try {
            //检测是否有写的权限
            val permission = ActivityCompat.checkSelfPermission(
                activity!!,
                "android.permission.WRITE_EXTERNAL_STORAGE"
            )
            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(
                    activity,
                    PERMISSIONS_STORAGE,
                    1
                )
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
}