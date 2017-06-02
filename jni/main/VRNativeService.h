#ifndef __ANDROID_DEMOSERVICE_H__
#define __ANDROID_DEMOSERVICE_H__

#include "IVRNativeService.h"
#include "IVRPostEvent.h"
#include <utils/threads.h>

namespace android {

class IVRPostEvent;
class VRNativeService : public BnVRNativeService {
private:
    VRNativeService();
public:
    ~VRNativeService();
    static VRNativeService* getSingleInstance();
    virtual int getSystemNode(String8 const &path, String8 &value) const;
    virtual int setSystemNode(String8 const &path, String8 const &value) const;
    virtual int systemCall(String8 const &cmd, String8 &result) const;
    virtual int getProperty(String8 const &key, String8 &value) const;
    virtual int setProperty(String8 const &key, String8 const &value) const;
    virtual int setThreadScheduler(int tid, int policy, int pri) const;
    virtual int setPostEventObj(const sp<IVRPostEvent>& iPostEvent);

    virtual sp<IVRPostEvent>& getReporter() { return mIPostEvent; }
private:
    static Mutex mLock;
    sp<IVRPostEvent> mIPostEvent;
};

}

#endif
