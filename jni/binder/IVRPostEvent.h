#ifndef __IVRPostEvent_H__
#define __IVRPostEvent_H__

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/String8.h>

#define POSTEVENT_PACK_NAME "com.android.VRAdmin.IVRPostEvent"

namespace android {

class IVRPostEvent : public IInterface {
public:
    DECLARE_META_INTERFACE(VRPostEvent);
    virtual int post(int what, int arg1, int arg2, const char *str = 0) const = 0;
};

class BnVRPostEvent : public BnInterface<IVRPostEvent> {
public:
    virtual status_t onTransact(uint32_t code,
                                const Parcel &data,
                                Parcel *reply,
                                uint32_t flags = 0);
};

}
#endif
