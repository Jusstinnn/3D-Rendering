package com.example.fan3d;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * MainActivity - 3D Fan.
 *
 * Renders a hierarchical 3D table fan (base/pole/hub/4 blades), built from
 * one cube and driven by the Transform push/pop matrix stack in native code.
 *
 * Gestures (forwarded to native via queueEvent, same pipeline the
 * desktop/web mouse mapping in Scene/main.cpp mirrors):
 *   tap          -> toggles the fan on/off
 *   click-drag   -> spins the fan faster, proportional to drag speed
 */
public class MainActivity extends AppCompatActivity {

    private static final String TAG = "Fan3D";

    static {
        System.loadLibrary("fan3d");
    }

    private GLSurfaceView glSurfaceView;
    private boolean rendererInitialized = false;

    private native boolean nativeInit(AssetManager assetManager);
    private native void    nativeResize(int width, int height);
    private native void    nativeRender();
    private native void    nativeTouchDown(float x, float y);
    private native void    nativeTouchMove(float x, float y);
    private native void    nativeTouchUp(float x, float y);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);

        glSurfaceView = findViewById(R.id.gl_surface_view);
        glSurfaceView.setEGLContextClientVersion(3);

        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                Log.d(TAG, "onSurfaceCreated");
                boolean ok = nativeInit(getAssets());
                if (ok) {
                    rendererInitialized = true;
                    Log.d(TAG, "Renderer initialized");
                } else {
                    Log.e(TAG, "Renderer initialization failed");
                }
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                Log.d(TAG, "onSurfaceChanged: " + width + "x" + height);
                if (rendererInitialized) nativeResize(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                if (rendererInitialized) nativeRender();
            }
        });

        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }

    // Every native call must go through glSurfaceView.queueEvent(...) — touch
    // callbacks fire on the UI thread, but GL calls must happen on the GL thread.
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        final float x = event.getX();
        final float y = event.getY();

        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                glSurfaceView.queueEvent(() -> {
                    if (rendererInitialized) nativeTouchDown(x, y);
                });
                return true;

            case MotionEvent.ACTION_MOVE:
                glSurfaceView.queueEvent(() -> {
                    if (rendererInitialized) nativeTouchMove(x, y);
                });
                return true;

            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                glSurfaceView.queueEvent(() -> {
                    if (rendererInitialized) nativeTouchUp(x, y);
                });
                return true;

            default:
                return super.onTouchEvent(event);
        }
    }

    @Override
    protected void onPause()  { super.onPause();  glSurfaceView.onPause();  }

    @Override
    protected void onResume() { super.onResume(); glSurfaceView.onResume(); }
}
