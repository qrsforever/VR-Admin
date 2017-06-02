#ifndef ___VrSDK__H_
#define ___VrSDK__H_

#include <string>
#include <inttypes.h>
#include <map>

namespace VR {

class VrSDK  {
public:
    VrSDK(const char* type, const char* version);
    ~VrSDK();

    const std::string& getType() const { return mType; }
    const std::string& getVersion() const { return mVersion; }
    int addExternalSDKItem(const char* v, uint64_t f);
    uint64_t getFeature(const char* ev);

#if VRS_DEBUG
    void debugShow();
#endif

private:
    std::string mType;
    std::string mVersion;
    std::map<std::string, uint64_t> mVFs;
};

}

#endif
