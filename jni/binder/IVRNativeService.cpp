#include "IVRNativeService.h"
#include <utils/Log.h>

namespace android {

enum {
    VRACTION_GETNODE = IBinder::FIRST_CALL_TRANSACTION,
    VRACTION_SETNODE,
    VRACTION_SYSCALL,
    VRACTION_GETATTR,
    VRACTION_SETATTR,

    VRACTION_SETSCHED,
    VRACTION_SETPOSTEVENTOBJ,
};

class BpVRNativeService : public BpInterface<IVRNativeService> {
public:
    BpVRNativeService(const sp<IBinder>& impl)
        : BpInterface<IVRNativeService>(impl) { }

    virtual int getSystemNode(String8 const &path, String8 &value) const {
        Parcel data, reply;
        data.writeInterfaceToken(IVRNativeService::getInterfaceDescriptor());
        data.writeString8(path);
        int status = remote()->transact(VRACTION_GETNODE, data, &reply);
        if (status != 0) {
            ALOGW("remote call VRACTION_GETATTR error!\n");
            return status;
        }
        value = reply.readString8();
        return reply.readInt32();
    }
    virtual int setSystemNode(String8 const &path, String8 const &value) const {
        Parcel data, reply;
        data.writeInterfaceToken(IVRNativeService::getInterfaceDescriptor());
        data.writeString8(path);
        data.writeString8(value);
        int status = remote()->transact(VRACTION_SETNODE, data, &reply);
        if (status != 0) {
            ALOGW("remote call VRACTION_SETATTR error!\n");
            return status;
        }
        return reply.readInt32();
    }
    virtual int systemCall(String8 const &cmd, String8 &result) const {
        Parcel data, reply;
        data.writeInterfaceToken(IVRNativeService::getInterfaceDescriptor());
        data.writeString8(cmd);
        int status = remote()->transact(VRACTION_SYSCALL, data, &reply);
        if (status != 0) {
            ALOGW("remote call VRACTION_SYSCALL error!\n");
            return status;
        }
        result = reply.readString8();
        return reply.readInt32();
    }
    virtual int getProperty(String8 const &key, String8 &value) const {
        Parcel data, reply;
        data.writeInterfaceToken(IVRNativeService::getInterfaceDescriptor());
        data.writeString8(key);
        int status = remote()->transact(VRACTION_GETATTR, data, &reply);
        if (status != 0) {
            ALOGW("remote call VRACTION_GETATTR error!\n");
            return status;
        }
        value = reply.readString8();
        return reply.readInt32();
    }
    virtual int setProperty(String8 const &key, String8 const &value) const {
        Parcel data, reply;
        data.writeInterfaceToken(IVRNativeService::getInterfaceDescriptor());
        data.writeString8(key);
        data.writeString8(value);
        int status = remote()->transact(VRACTION_SETATTR, data, &reply);
        if (status != 0) {
            ALOGW("remote call VRACTION_SETATTR error!\n");
            return status;
        }
        return reply.readInt32();
    }
    virtual int setThreadScheduler(int tid, int policy, int pri) const {
        Parcel data, reply;
        data.writeInterfaceToken(IVRNativeService::getInterfaceDescriptor());
        data.writeInt32(tid);
        data.writeInt32(policy);
        data.writeInt32(pri);
        int status = remote()->transact(VRACTION_SETSCHED, data, &reply);
        if (status != 0) {
            ALOGW("remote call VRACTION_SETATTR error!\n");
            return status;
        }
        return reply.readInt32();
    }
    virtual int setPostEventObj(const sp<IVRPostEvent>& iPostEvent) {
        Parcel data, reply;
        data.writeInterfaceToken(IVRNativeService::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(iPostEvent));
        int status = remote()->transact(VRACTION_SETPOSTEVENTOBJ, data, &reply);
        if (status != 0) {
            ALOGW("remote call VRACTION_SETPOSTEVENTOBJ error!\n");
            return status;
        }
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(VRNativeService, SERVER_PACK_NAME);

status_t BnVRNativeService::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    int ret = -1;
    int len = 0;
    switch (code) {
    case VRACTION_GETNODE: {
            CHECK_INTERFACE(IVRNativeService, data, reply);
            String8 path = data.readString8();
            String8 value;
            ret = getSystemNode(path, value);
            reply->writeString8(value);
            reply->writeInt32(ret);
        }
        break;
    case VRACTION_SETNODE: {
            CHECK_INTERFACE(IVRNativeService, data, reply);
            String8 path = data.readString8();
            String8 value = data.readString8();
            ret = setSystemNode(path, value);
            reply->writeInt32(ret);
        }
        break;
    case VRACTION_SYSCALL: {
            CHECK_INTERFACE(IVRNativeService, data, reply);
            String8 cmd = data.readString8();
            String8 result;
            ret = systemCall(cmd, result);
            reply->writeString8(result);
            reply->writeInt32(ret);
        }
        break;
    case VRACTION_GETATTR: {
            CHECK_INTERFACE(IVRNativeService, data, reply);
            String8 key = data.readString8();
            String8 value;
            ret = getProperty(key, value);
            reply->writeString8(value);
            reply->writeInt32(ret);
        }
        break;
    case VRACTION_SETATTR: {
            CHECK_INTERFACE(IVRNativeService, data, reply);
            String8 key = data.readString8();
            String8 value = data.readString8();
            ret = setProperty(key, value);
            reply->writeInt32(ret);
        }
        break;
    case VRACTION_SETSCHED: {
            CHECK_INTERFACE(IVRNativeService, data, reply);
            ret = setThreadScheduler(data.readInt32(), data.readInt32(), data.readInt32());
            reply->writeInt32(ret);
        }
        break;
    case VRACTION_SETPOSTEVENTOBJ: {
            CHECK_INTERFACE(IVRNativeService, data, reply);
            sp<IVRPostEvent> ipost = interface_cast<IVRPostEvent>(data.readStrongBinder());
            ret = setPostEventObj(ipost);
            reply->writeInt32(ret);
            return NO_ERROR;
        }
        break;
    default:
        return BBinder::onTransact(code, data, reply, flags);
    }
    return 0;
}

}
