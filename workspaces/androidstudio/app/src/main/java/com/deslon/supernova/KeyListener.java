package com.deslon.supernova;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnKeyListener;

public class KeyListener implements OnKeyListener {

	private Activity activity;
	private GLSurfaceView glSurfaceView;
	
	public KeyListener(Activity activity, GLSurfaceView glSurfaceView){
		this.activity = activity;
		this.glSurfaceView = glSurfaceView;
	}
	
	static private int convertToSupernova(int key){
		if (key == KeyEvent.KEYCODE_DPAD_UP)
			return 265;
		if (key == KeyEvent.KEYCODE_DPAD_DOWN)
			return 264;
		if (key == KeyEvent.KEYCODE_DPAD_RIGHT)
			return 262;
		if (key == KeyEvent.KEYCODE_DPAD_LEFT)
			return 263;
		if (key == KeyEvent.KEYCODE_D)
			return 68;
		if (key == KeyEvent.KEYCODE_Z)
			return 90;
		if (key == KeyEvent.KEYCODE_X)
			return 88;
		return 0;
	}

	@Override
	public boolean onKey(View v, final int keyCode, KeyEvent event) {
		final int supernovaKey = convertToSupernova(keyCode);

        if(event.getAction() == KeyEvent.ACTION_DOWN && supernovaKey > 0) {
            glSurfaceView.queueEvent(new Runnable() {
                @Override
                public void run() {
                	JNIWrapper.system_key_down(supernovaKey);
                }
            });
            return true;
        }else if(event.getAction() == KeyEvent.ACTION_UP && supernovaKey > 0) {
            glSurfaceView.queueEvent(new Runnable() {
                @Override
                public void run() {
                	JNIWrapper.system_key_up(supernovaKey);
                }
            });
            return true;
        }

        return false;
	}

}
