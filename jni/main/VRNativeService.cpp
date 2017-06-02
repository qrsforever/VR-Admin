#include "VRNativeService.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <error.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <cutils/sched_policy.h>

namespace android {

Mutex VRNativeService::mLock;

static const int kCpuMaxCount = 10;
static const int kCpuNodeCount = 3;
static const char *gCpuNodeNames[kCpuNodeCount] = {
    "cpufreq/scaling_min_freq",
    "cpufreq/scaling_available_frequencies",
    "cpufreq/scaling_cur_freq"
};

static const int kGpuMaxCount = 4;
static const int kGpuNodeCount = 1;
static const char *gGpuNodeNames[kGpuNodeCount] = {
    "msm_cmd_autorefresh_en"
};

static bool _CheckAccessNodePerm(const char* nodepath)
{
    if (!nodepath)
        return false;

    int i, j;
    char buffer[128] = { 0 };
    for (i = 0; i < kCpuMaxCount; ++i) {
        for (j = 0; j < kCpuNodeCount; ++j) {
            snprintf(buffer, 127, "/sys/devices/system/cpu/cpu%d/%s", i, gCpuNodeNames[j]);
            if (0 == strcmp(buffer, nodepath))
                return true;
        }
    }
    for (i = 0; i < kGpuMaxCount; ++i) {
        for (j = 0; j < kGpuNodeCount; ++j) {
            snprintf(buffer, 127, "/sys/devices/virtual/graphics/fb%d/%s", i, gGpuNodeNames[j]);
            if (0 == strcmp(buffer, nodepath))
                return true;
        }
    }
    return false;
}

static const int kSysCallCount = 2;
static const char *gSysCallCommands[kSysCallCount] = {
    "dfu-util",
    "service call"
};

static bool _CheckSyscallPerm(const char* cmd)
{
    if (!cmd)
        return false;

    const char* temp = 0;
    for (int i = 0; i < kSysCallCount; ++i) {
        temp = gSysCallCommands[i];
        if (0 == strncmp(temp, cmd, strlen(temp)))
            return true;
    }
    return false;
}


VRNativeService::VRNativeService() : mIPostEvent(0)
{
}

VRNativeService::~VRNativeService()
{
}

int VRNativeService::getSystemNode(String8 const &path, String8 &value) const
{
    if ((access(path, F_OK) < 0))
        return ERR_NOFOUND;

    if (!_CheckAccessNodePerm(path.string())) {
        ALOGE("[%s] is not in white list!\n", path.string());
        return ERR_PERM;
    }

    char temp[128] = { 0 };
    int ret = -1;
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        ALOGE("getSystemNode open err: %s\n", strerror(errno));
        return ERR_PERM;
    }
    ret = read(fd, temp, 128);
    if (ret < 0) {
        ALOGE("getSystemNode write err: %s\n", strerror(errno));
        close(fd);
        return ERR_RDWR;
    }
    close(fd);

    value = temp;
    ALOGI("getSystemNode(%s) = %s.\n", path.string(), temp);
    return NO_ERROR;
}

int VRNativeService::setSystemNode(String8 const &path, String8 const &value) const
{
    if ((access(path, F_OK) < 0)) {
        ALOGE("setSystemNode access err: %s\n", strerror(errno));
        return ERR_NOFOUND;
    }

    if (!_CheckAccessNodePerm(path.string())) {
        ALOGE("[%s] is not in white list!\n", path.string());
        return ERR_PERM;
    }

    int ret = -1;
    int fd = open(path.string(), O_WRONLY);
    if (fd < 0) {
        ALOGE("setSystemNode open err: %s\n", strerror(errno));
        return ERR_PERM;
    }
    ret = write(fd, value.string(), value.length());
    if (ret < 0) {
        ALOGE("setSystemNode write err: %s\n", strerror(errno));
        close(fd);
        return ERR_RDWR;
    }
    close(fd);
    ALOGI("setSystemNode(%s, %s) ok.\n", path.string(), value.string());
    return NO_ERROR;
}

int VRNativeService::systemCall(String8 const &cmd, String8 &result) const
{
    if (!_CheckSyscallPerm(cmd.string())) {
        ALOGE("[%s] is not in white list!\n", cmd.string());
        return -1;
    }

    char line[1024] = { 0 };
    FILE *fp = popen(cmd.string(), "r");
    if (!fp) {
        ALOGE("popen(%s) error: %s\n", cmd.string(), strerror(errno));
        return -1;
    }
    int sum = 0, len = 0;
    while ((fgets(line, 1024, fp)) != NULL) {
        len = strlen(line);
        result.append(line, len);
        sum += len;
    }
    pclose(fp);
    ALOGI("systemCall(%s) result len = %d.\n", cmd.string(), sum);
    return sum;
}

int VRNativeService::getProperty(String8 const &key, String8 &value) const
{
    Mutex::Autolock autoLock(mLock);
    char temp[256] = { 0 };
    int status = property_get(key.string(), temp, "");
    if (status < 0) {
        ALOGE("property_get(%s) error! status(%d)\n", key.string(), status);
        return -1;
    }
    value = temp;
    ALOGI("property_get(%s) = %s\n", key.string(), temp);
    return status;
}

int VRNativeService::setProperty(String8 const &key, String8 const &value) const
{
    Mutex::Autolock autoLock(mLock);
    int status = property_set(key.string(), value.string());
    if (status < 0) {
        ALOGE("property_set(%s, %s) error! status(%d)\n", key.string(), value.string(), status);
        return -1;
    }
    return status;
}

int VRNativeService::setThreadScheduler(int tid, int policy, int pri) const
{
    struct sched_param param;
    param.sched_priority = pri;
    int ret = sched_setscheduler(tid, policy, &param);
    return (ret == 0) ? 0 : errno * -1;
}

int VRNativeService::setPostEventObj(const sp<IVRPostEvent>& iPostEvent)
{
    ALOGI("setPostEventObj\n");
    mIPostEvent = iPostEvent;
    return 0;
}

VRNativeService* VRNativeService::getSingleInstance()
{
    static VRNativeService* s_vr = 0;
    if (!s_vr)
        s_vr = new VRNativeService();
    return s_vr;
}

}
