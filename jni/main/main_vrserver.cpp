#include "VRNativeService.h"

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include "ServerListener.h"

using namespace android;

int main(int , char**)
{
#if defined(VRS_BUILDTIME)
    ALOGI("main_vrserver start, pid = %ld, buildtime = %s\n", getpid(), VRS_BUILDTIME);
#endif

    sp<IServiceManager> sm = defaultServiceManager();
    sm->addService(String16(SERVER_NAME), VRNativeService::getSingleInstance());
    sp<ProcessState> proc(ProcessState::self());

    VR::ServerListener::self().start();

    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();
    ALOGE("main_vrserver quit\n");
    return 0;
}
