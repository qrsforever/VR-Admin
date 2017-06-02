#include "VrSDK.h"

#include <stdio.h>
#include <ctype.h>
#include <utils/Log.h>
#include <cutils/properties.h>

namespace VR {

VrSDK::VrSDK(const char* type, const char* version)
    : mType(type), mVersion(version)
{
    const int len = strlen(type);
    char buff[len + 1] = { 0 }; 
    char prop[128] = { 0 };
    for (int i = 0; i < len; ++i)
        buff[i] = tolower(type[i]);
    snprintf(prop, 127, "vr.%s.version", buff);
    int status = property_set(prop, version);
    if (status < 0)
        ALOGE("property_set(%s, %s) error! status(%d)\n", prop, version, status);
}

VrSDK::~VrSDK()
{
    mVFs.clear();
}

#if VRS_DEBUG
void
VrSDK::debugShow()
{
    ALOGI("SDK type[%s] version[%s]\n", mType.c_str(), mVersion.c_str());
    std::map<std::string, uint64_t>::iterator it;
    for (it = mVFs.begin(); it != mVFs.end(); ++it)
        ALOGI("\t ExternalSDK V[%s] F[%llu]\n", it->first.c_str(), it->second);
}
#endif

int
VrSDK::addExternalSDKItem(const char* v, uint64_t f)
{
    if (!v)
        return -1;
    std::map<std::string, uint64_t>::iterator it = mVFs.find(v);
    if (it != mVFs.end()) {
        ALOGW("addExternSDKItem(%s) already exist!\n", v);
        return 1;
    }
    mVFs.insert(std::pair<std::string, uint64_t>(v, f));
    return 0;
}

uint64_t
VrSDK::getFeature(const char* v)
{
    if (!v)
        return 0;
    std::string str(v);
    std::size_t found = str.find('.');
    if (found != std::string::npos)
        str = str.substr(0, found);
    std::map<std::string, uint64_t>::iterator it = mVFs.find(str);
    if (it == mVFs.end()) {
        ALOGW("getFeature(%s) cannot found!\n", str.c_str());
        return 0;
    }
    return it->second;
}

}
