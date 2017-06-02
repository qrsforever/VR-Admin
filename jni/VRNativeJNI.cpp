#include "JNIHelp.h"
#include <jni.h>
#include "utils/Log.h"
#include "android_runtime/AndroidRuntime.h"
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include "IVRNativeService.h"
#include "IVRPostEvent.h"

static const char* kVRManagerClass = "com/android/VRAdmin/VRManager";

#define FLUSH_COMMANDS()  { }

#if 0
#define FLUSH_COMMANDS() \
    do { \
        ProcessState::self()->startThreadPool(); \
        IPCThreadState::self()->flushCommands(); \
    } while(0);
#endif

using namespace android;

class JVRPostEvent : public BnVRPostEvent {
public:
    JVRPostEvent(JNIEnv* env, jobject thiz, jobject weak_thiz);
    ~JVRPostEvent();
    int post(int what, int arg1, int arg2, const char *str = 0) const;
private:
    jclass mClass;
    jobject mObject;
    jmethodID mPostMehodID;
};

JVRPostEvent::JVRPostEvent(JNIEnv* env, jobject thiz, jobject weak_thiz)
    : mClass(0), mObject(0), mPostMehodID(0)
{
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        ALOGE("Can't find %s\n", kVRManagerClass);
        // jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }

    mPostMehodID = env->GetStaticMethodID(
        clazz,
        "postEventFromNative",
        "(Ljava/lang/Object;IIILjava/lang/String;)V");

    if (mPostMehodID == NULL) {
        ALOGE("GetStaticMethodID(postEventFromNative) error!\n");
        // jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }
    mClass = (jclass)env->NewGlobalRef(clazz);
    mObject = env->NewGlobalRef(weak_thiz);
}

JVRPostEvent::~JVRPostEvent()
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);
}

int JVRPostEvent::post(int what, int arg1, int arg2, const char *str) const
{
    ALOGI("post(%d %d %d %s)\n", what, arg1, arg2, str);
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    if (str) {
        jstring jstr = env->NewStringUTF(str);
        env->CallStaticVoidMethod(
            mClass, mPostMehodID, mObject,
            what, arg1, arg2, jstr);
        if (jstr)
            env->DeleteLocalRef(jstr);
    }
    return 0;
}

static String8 JStringToString8(JNIEnv *env, jstring const &jstr) {
    String8 result;

    const char *s = env->GetStringUTFChars(jstr, NULL);
    if (s) {
        result = s;
        env->ReleaseStringUTFChars(jstr, s);
    }
    return result;
}

static sp<IVRNativeService> getVRService()
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder;

    if((binder = sm->getService(String16(SERVER_NAME))) == 0) {
        ALOGE("sm->getService(%s) error!\n", SERVER_NAME);
        return 0;
    }
    return interface_cast<IVRNativeService>(binder);
}

static void __native_init(JNIEnv *env)
{
    ALOGI("__native_init\n");
}

static void __native_setup(JNIEnv *env, jobject thiz, jobject refObj)
{
    ALOGI("__native_setup\n");
    sp<IVRPostEvent> reporter = new JVRPostEvent(env, thiz, refObj);
    sp<IVRNativeService> vr = getVRService();
    if (vr == NULL) {
        ALOGE("VRNativeService obj is null!\n");
        // jniThrowException(env, "java/lang/IllegalStateException", "VRNativeService obj is null");
        return;
    }
    vr->setPostEventObj(reporter);
}

static jstring __native_getSystemNode(JNIEnv *env, jobject thiz, jstring jpath)
{
    ALOGI("__native_getSystemNode\n");

    String8 path = JStringToString8(env, jpath);
    String8 value;
    if (jpath == NULL) {
        ALOGE("path or value param is null!\n");
        return env->NewStringUTF("");
    }
    sp<IVRNativeService> vr = getVRService();
    if (vr == NULL) {
        ALOGE("VRNativeService obj is null!\n");
        // jniThrowException(env, "java/lang/IllegalStateException", "VRNativeService obj is null");
        return env->NewStringUTF("");
    }
    vr->getSystemNode(path, value);
    FLUSH_COMMANDS();
    return env->NewStringUTF(value.string());
}

static jint __native_setSystemNode(JNIEnv *env, jobject thiz, jstring jpath, jstring jvalue)
{
    ALOGI("__native_setSystemNode\n");
    int result = -1;
    String8 path = JStringToString8(env, jpath);
    String8 value = JStringToString8(env, jvalue);
    if (jpath == NULL || jvalue == NULL) {
        ALOGE("path or value param is null!\n");
        return -1;
    }
    sp<IVRNativeService> vr = getVRService();
    if (vr == NULL) {
        ALOGE("VRNativeService obj is null!\n");
        // jniThrowException(env, "java/lang/IllegalStateException", "VRNativeService obj is null");
        return -1;
    }
    result = vr->setSystemNode(path, value);
    FLUSH_COMMANDS();
    return result;
}

static jstring __native_systemCall(JNIEnv *env, jobject thiz, jstring jcmd)
{
    ALOGI("__native_systemCall\n");

    String8 cmd = JStringToString8(env, jcmd);
    String8 result;

    sp<IVRNativeService> vr = getVRService();
    if (vr == NULL) {
        ALOGE("VRNativeService obj is null!\n");
        // jniThrowException(env, "java/lang/IllegalStateException", "VRNativeService obj is null");
        return env->NewStringUTF("");
    }
    vr->systemCall(cmd, result);
    FLUSH_COMMANDS();
    return env->NewStringUTF(result.string());
}

static jstring __native_getProperty(JNIEnv *env, jobject thiz, jstring jkey)
{
    ALOGI("__native_getProperty\n");

    String8 key = JStringToString8(env, jkey);
    String8 value;
    if (jkey == NULL) {
        ALOGE("key param is null!\n");
        return env->NewStringUTF("");
    }
    sp<IVRNativeService> vr = getVRService();
    if (vr == NULL) {
        ALOGE("VRNativeService obj is null!\n");
        // jniThrowException(env, "java/lang/IllegalStateException", "VRNativeService obj is null");
        return env->NewStringUTF("");
    }
    vr->getProperty(key, value);
    FLUSH_COMMANDS();
    return env->NewStringUTF(value.string());
}

static jint __native_setProperty(JNIEnv *env, jobject thiz, jstring jkey, jstring jvalue)
{
    ALOGI("__native_setProperty\n");
    int result = -1;
    String8 key = JStringToString8(env, jkey);
    String8 value = JStringToString8(env, jvalue);
    if (jkey == NULL || jvalue == NULL) {
        ALOGE("key or value param is null!\n");
        return -1;
    }
    sp<IVRNativeService> vr = getVRService();
    if (vr == NULL) {
        ALOGE("VRNativeService obj is null!\n");
        // jniThrowException(env, "java/lang/IllegalStateException", "VRNativeService obj is null");
        return -1;
    }
    result = vr->setProperty(key, value);
    FLUSH_COMMANDS();
    return result;
}

static jint __native_setThreadScheduler(JNIEnv *env, jobject thiz, jint jtid, jint jpolicy, jint jpri)
{
    ALOGI("__native_setThreadScheduler.\n");
    sp<IVRNativeService> vr = getVRService();
    if (vr == NULL) {
        ALOGE("VRNativeService obj is null!\n");
        // jniThrowException(env, "java/lang/IllegalStateException", "VRNativeService obj is null");
        return -1;
    }
    return vr->setThreadScheduler(jtid, jpolicy, jpri);
}

static JNINativeMethod gMethods[] = {
    { "native_init",            "()V",                            (void *)__native_init },
    { "native_setup",           "(Ljava/lang/Object;)V",          (void *)__native_setup },

    {
        "native_getSystemNode",
        "(Ljava/lang/String;)Ljava/lang/String;",
        (void *)__native_getSystemNode
    },
    {
        "native_setSystemNode",
        "(Ljava/lang/String;Ljava/lang/String;)I",
        (void *)__native_setSystemNode
    },
    {
        "native_systemCall",
        "(Ljava/lang/String;)Ljava/lang/String;",
        (void *)__native_systemCall
    },
    {
        "native_getProperty",
        "(Ljava/lang/String;)Ljava/lang/String;",
        (void *)__native_getProperty
    },
    {
        "native_setProperty",
        "(Ljava/lang/String;Ljava/lang/String;)I",
        (void *)__native_setProperty
    },
    {
        "native_setThreadScheduler",
        "(III)I",
        (void *)__native_setThreadScheduler
    },
};

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    ALOGI("JNI_OnLoad\n");
    JNIEnv* env = 0;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
        return JNI_ERR;
    jclass clazz = 0;
    if (!(clazz = env->FindClass(kVRManagerClass))) {
        ALOGE("JNI_OnLoad %s ERR\n", kVRManagerClass);
        return JNI_ERR;
    }
    env->RegisterNatives(clazz, gMethods, NELEM(gMethods));
    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM* vm, void* reserved)
{
    ALOGI("JNI_Unload\n");
}
