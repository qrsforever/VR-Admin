#ifndef __XmlDatabase__H_
#define __XmlDatabase__H_

#include <string>
#include <inttypes.h>
#include <vector>

#include "VrSDK.h"

namespace VR {

class XmlDatabase {
public:
    XmlDatabase();
    ~XmlDatabase();

    int parser(const char* filepath);
    std::string getAllSDKTypes();
    VrSDK* getSDK(const char* type) const;
    int getSDKSCount() { return mData.size(); }

#if VRS_DEBUG
    void debugShow();
#endif

private:
    VrSDK* _GenerateSDK(const char* type, const char* version);

private:
    std::vector<VrSDK*> mData;
};

}

#endif
