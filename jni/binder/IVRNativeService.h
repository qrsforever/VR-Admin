#ifndef __ANDROID_IDEMO_SERVICE_H__
#define __ANDROID_IDEMO_SERVICE_H__

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/String8.h>
#include "IVRPostEvent.h"

#define SERVER_PACK_NAME "com.android.VRAdmin.IVRNativeService"
#define SERVER_NAME "vrserver"

enum {
    NO_ERROR = 0,
    ERR_PARM = -1,
    ERR_PERM = -2,
    ERR_NOFOUND = -3,
    ERR_RDWR = -4,
};

namespace android {

class IVRNativeService : public IInterface {
public:
    DECLARE_META_INTERFACE(VRNativeService);
    virtual int getSystemNode(String8 const &path, String8 &value) const = 0;
    virtual int setSystemNode(String8 const &path, String8 const &value) const = 0;
    virtual int systemCall(String8 const &cmd, String8 &result) const = 0;
    virtual int getProperty(String8 const &key, String8 &value) const = 0;
    virtual int setProperty(String8 const &key, String8 const &value) const = 0;
    virtual int setThreadScheduler(int tid, int policy, int pri) const = 0;
    virtual int setPostEventObj(const sp<IVRPostEvent>& iPostEvent) = 0;
};

class BnVRNativeService : public BnInterface<IVRNativeService> {
public:
    virtual status_t onTransact(uint32_t code,
                                const Parcel &data,
                                Parcel *reply,
                                uint32_t flags = 0);
};

}
#endif
