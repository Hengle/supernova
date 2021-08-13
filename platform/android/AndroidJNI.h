#ifndef ANDROID_JNI_H_
#define ANDROID_JNI_H_

#include <jni.h>
#include <android/asset_manager.h>

class AndroidJNI {

public:

	static jclass mainActivityClsRef;
	static jmethodID getScreenWidthRef;
	static jmethodID getScreenHeightRef;
	static jmethodID getUserDataPathRef;
	static jmethodID showSoftKeyboardRef;
	static jmethodID hideSoftKeyboardRef;

	static jmethodID getBoolForKeyRef;
	static jmethodID getIntegerForKeyRef;
	static jmethodID getLongForKeyRef;
	static jmethodID getFloatForKeyRef;
	static jmethodID getDoubleForKeyRef;
	static jmethodID getStringForKeyRef;

	static jmethodID setBoolForKeyRef;
	static jmethodID setIntegerForKeyRef;
	static jmethodID setLongForKeyRef;
	static jmethodID setFloatForKeyRef;
	static jmethodID setDoubleForKeyRef;
	static jmethodID setStringForKeyRef;

	static jmethodID removeKeyRef;

	static jobject mainActivityObjRef;
	static JNIEnv *envRef;

	static AAssetManager* android_asset_manager;
};


extern "C" {
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_init_1native(JNIEnv * env, jclass cls, jobject main_activity, jobject java_asset_manager);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1start(JNIEnv * env, jclass cls);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1surface_1created(JNIEnv * env, jclass cls);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1surface_1changed(JNIEnv * env, jclass cls);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1draw(JNIEnv * env, jclass cls);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1pause(JNIEnv * env, jclass cls);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1resume(JNIEnv * env, jclass cls);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1touch_1start(JNIEnv * env, jclass cls, jint pointer, jfloat x, jfloat y);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1touch_1end(JNIEnv * env, jclass cls, jint pointer, jfloat x, jfloat y);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1touch_1drag(JNIEnv * env, jclass cls, jint pointer, jfloat x, jfloat y);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1key_1down(JNIEnv * env, jclass cls, jint key);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1key_1up(JNIEnv * env, jclass cls, jint key);
	JNIEXPORT void JNICALL Java_com_deslon_supernova_JNIWrapper_system_1text_1input(JNIEnv * env, jclass cls, jstring text);
}

//void showSoftKeyboard();


#endif /* ANDROID_JNI_H_ */
