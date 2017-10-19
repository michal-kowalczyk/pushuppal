package pl.ekk.mkk.pushuppal

import android.animation.ValueAnimator
import android.app.Activity
import android.os.Bundle
import android.util.Log
import android.view.SurfaceView
import android.view.WindowManager

import org.opencv.android.CameraBridgeViewBase
import org.opencv.android.OpenCVLoader
import org.opencv.core.Core
import org.opencv.core.CvType
import org.opencv.core.Mat

import java.io.IOException

import pl.ekk.mkk.pushuppal.generated.PushUpListener
import pl.ekk.mkk.pushuppal.generated.PushUpPalApp

import kotlinx.android.synthetic.main.activity_main.*
import pl.ekk.mkk.pushuppal.generated.CameraOrientation

class MainActivity : Activity(), CameraBridgeViewBase.CvCameraViewListener2 {
    private val TAG = "PushUpPal"

    private var mOpenCvCameraView: CameraBridgeViewBase? = null
    private var mFrame: Mat? = null
    private var mFrameFlipped: Mat? = null
    private var mPushUpPalApp: PushUpPalApp? = null

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        setContentView(R.layout.activity_main)

        mOpenCvCameraView = javaSurfaceView
        mOpenCvCameraView!!.visibility = SurfaceView.VISIBLE

        mOpenCvCameraView!!.setCvCameraViewListener(this)

        val textSize = repsTextView.textSize

        try {
            mPushUpPalApp = PushUpPalApp.create(CameraOrientation.LANDSCAPE,
                    ResourcePathAccessor.getClassifierFilePath(this@MainActivity))

            mPushUpPalApp!!.setListener(object : PushUpListener() {
                override fun onPushUp(rep: Int) {
                    val reps = Integer.valueOf(rep)!!.toString()
                    runOnUiThread {
                        repsTextView.text = reps

                        val startSize = textSize / 10
                        val endSize = textSize / 2
                        val animationDuration = 300

                        val animator = ValueAnimator.ofFloat(startSize, endSize)
                        animator.duration = animationDuration.toLong()

                        animator.addUpdateListener { valueAnimator ->
                            val animatedValue = valueAnimator.animatedValue as Float
                            repsTextView.textSize = animatedValue
                        }

                        animator.start()
                    }
                }
            })

            startStopButton.setOnClickListener {
                if (mPushUpPalApp!!.isStarted) {
                    mPushUpPalApp!!.stop()
                    startStopButton.text = "Start"
                } else {
                    mPushUpPalApp!!.start()
                    startStopButton.text = "Stop"
                }
            }

            resetButton.setOnClickListener {
                mPushUpPalApp!!.reset()
                startStopButton.text = "Start"
            }
        } catch (e: IOException) {
            Log.e(TAG, "Couldn't access classifier file path")
        }
    }

    public override fun onPause() {
        super.onPause()
        if (mOpenCvCameraView != null) {
            mOpenCvCameraView!!.disableView()
        }
    }

    public override fun onResume() {
        super.onResume()
        if (OpenCVLoader.initDebug()) {
            mOpenCvCameraView!!.enableView()
        }
    }

    public override fun onDestroy() {
        super.onDestroy()
        mOpenCvCameraView!!.disableView()
    }

    override fun onCameraViewStarted(width: Int, height: Int) {
        mFrame = Mat(height, width, CvType.CV_8UC4)
        mFrameFlipped = Mat(height, width, CvType.CV_8UC4)
    }

    override fun onCameraViewStopped() {
        mFrame!!.release()
        mFrameFlipped!!.release()
    }

    override fun onCameraFrame(inputFrame: CameraBridgeViewBase.CvCameraViewFrame): Mat? {
        mFrame = inputFrame.rgba()
        Core.flip(mFrame, mFrameFlipped, 1)
        mPushUpPalApp!!.onFrame(mFrameFlipped)
        return mFrameFlipped
    }

    companion object {
        init {
            System.loadLibrary("native-pushuppal")
        }
    }
}
