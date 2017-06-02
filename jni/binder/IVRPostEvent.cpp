#include "IVRPostEvent.h"
#include <utils/Log.h>

namespace android {

enum {
    VRACTION_POSTEVENT = IBinder::FIRST_CALL_TRANSACTION,
};

class BpVRPostEvent : public BpInterface<IVRPostEvent> {
public:
    BpVRPostEvent(const sp<IBinder>& impl)
        : BpInterface<IVRPostEvent>(impl) { }

    virtual int post(int what, int arg1, int arg2, const char *str) const {
        Parcel data, reply;
        data.writeInterfaceToken(IVRPostEvent::getInterfaceDescriptor());
        data.writeInt32(what);
        data.writeInt32(arg1);
        data.writeInt32(arg2);
        if (str) {
            data.writeInt32(1);
            data.writeString8(String8(str));
        } else {
            data.writeInt32(0);
        }
        int status = remote()->transact(VRACTION_POSTEVENT, data, &reply);
        if (status != 0) {
            ALOGW("remote call VRACTION_POSTEVENT error!\n");
            return status;
        }
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(VRPostEvent, POSTEVENT_PACK_NAME);

status_t BnVRPostEvent::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    int ret = -1;
    int len = 0;
    switch (code) {
    case VRACTION_POSTEVENT: {
            CHECK_INTERFACE(IVRPostEvent, data, reply);
            int what = data.readInt32();
            int arg1 = data.readInt32();
            int arg2 = data.readInt32();
            int bstr = data.readInt32();
            ret = post(what, arg1, arg2, bstr ? data.readString8().string() : "null");
            reply->writeInt32(ret);
        }
        break;
    default:
        return BBinder::onTransact(code, data, reply, flags);
    }
    return 0;
}

}
