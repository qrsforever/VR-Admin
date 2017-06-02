#include "XmlDatabase.h"

#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <utils/Log.h>

namespace VR {


XmlDatabase::XmlDatabase()
    : mData(0)
{

}

XmlDatabase::~XmlDatabase()
{
    int count = mData.size();
    for (int i = 0; i < count; ++i)
        delete mData[i];
    mData.clear();
}

#if VRS_DEBUG
void
XmlDatabase::debugShow()
{
    int cnt = mData.size();
    for (int i = 0; i < cnt; ++i)
        mData[i]->debugShow();
}
#endif

std::string
XmlDatabase::getAllSDKTypes()
{
    std::string types("");
    int cnt = mData.size();
    for (int i = 0; i < cnt; ++i) {
        if (i != 0)
            types.append("^");
        types.append(mData[i]->getType());
    }
    return types;
}

VrSDK*
XmlDatabase::getSDK(const char* type) const
{
    int cnt = mData.size();
    for (int i = 0; i < cnt; ++i) {
        if (!strcasecmp(type, mData[i]->getType().c_str())) {
            return mData[i];
        }
    }
    return 0;
}

VrSDK*
XmlDatabase::_GenerateSDK(const char* type, const char* version)
{
    if (!type || !version)
        return 0;

    int cnt = mData.size();
    for (int i = 0; i < cnt; ++i) {
        if (!strcmp(type, mData[i]->getType().c_str())) {
            return mData[i];
        }
    }
    VrSDK* sdk = new VrSDK(type, version);
    mData.push_back(sdk);
    return sdk;
}

int
XmlDatabase::parser(const char* filepath)
{
    if (!filepath)
        return -1;

    const char *val = 0, *pro = 0;
    VrSDK *sdk = 0;

    xmlDocPtr doc = 0;
    xmlNodePtr curNode = 0;

    doc = xmlReadFile(filepath, "UTF8", XML_PARSE_RECOVER);
    if (!doc) {
        ALOGE("xmlReadFile[%s] error!\n", filepath);
        return -1;
    }
    curNode = xmlDocGetRootElement(doc);
    if (!curNode) {
        ALOGE("empty document\n");
        xmlFreeDoc(doc);
        return -1;
    }
    const char* type = (const char*)curNode->name;
    const char* version = (const char*)xmlGetProp(curNode, BAD_CAST "Version");
    sdk = _GenerateSDK(type, version);
    if (!sdk) {
       xmlFreeDoc(doc);
       return -1;
    }
    curNode = curNode->children;
    while (curNode) {
        if (!xmlStrcmp(curNode->name, BAD_CAST "ExternalSDK")) {
            pro = (const char*)xmlGetProp(curNode, BAD_CAST "Version");
            val = (const char*)xmlNodeGetContent(curNode);
            if (pro && val)
                sdk->addExternalSDKItem(pro, atoll(val));
            if (pro) xmlFree(BAD_CAST pro);
            if (val) xmlFree(BAD_CAST val);
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return 0;
}

}
