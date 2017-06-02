#include "ClientConnector.h"

#include <unistd.h>
#include <string.h>

#include <utils/Log.h>
#include <cutils/properties.h>

#define BUFF_SIZE 64

namespace VR {

ClientConnector::ClientConnector(int connid, XmlDatabase& db)
    : Connector(connid), mDB(db)
{
}

ClientConnector::~ClientConnector()
{
}

int
ClientConnector::preSelect(fd_set* rset)
{
    if (mConnID > 0 && rset)
        FD_SET(mConnID, rset);
    return mConnID;
}

int
ClientConnector::postSelect(fd_set* rset)
{
    if (mConnID < 0 || !FD_ISSET(mConnID, rset))
        return RESULT_ERR;
    int num = -1, ret = -1;
    char rbuff[BUFF_SIZE+1] = { 0 };
    char wbuff[BUFF_SIZE+1] = { 0 };
    num = read(mConnID, rbuff, sizeof(rbuff));
    if (num < 0)
        return RESULT_ERR;
    else if (num == 0)
        return RESULT_QUIT; // force quit.

    if (!strncmp(rbuff, "quit", 4))
        return RESULT_QUIT;
    ret = processMessage(rbuff, wbuff);
    if (ret < 0)
        sprintf(wbuff, "%d", ret);
    num = write(mConnID, wbuff, strlen(wbuff));
    if (num < 0)
        return RESULT_ERR;
    ALOGI("ClientConnector[%d]::postSelect(%s) write[%s]\n", mConnID, rbuff, wbuff);
    return RESULT_OK;
}

int
ClientConnector::processMessage(char* r, char* w)
{
    if (!strncasecmp(r, "support_sdks", 6)) {
        std::string types = mDB.getAllSDKTypes();
        if (types.empty())
            return -2;
        strncpy(w, types.c_str(), BUFF_SIZE);
        return 0;
    }
    if (!strncasecmp(r, "lvr_sdk_version", 15)) {
        VrSDK* sdk = mDB.getSDK("lvr");
        if (!sdk)
            return -3;
        const std::string& ver = sdk->getVersion();
        if (ver.empty())
            return -2;
        strncpy(w, ver.c_str(), BUFF_SIZE);
        return 0;
    }
    if (!strncasecmp(r, "svr_sdk_version", 15)) {
        VrSDK* sdk = mDB.getSDK("svr");
        if (!sdk)
            return -3;
        const std::string& ver = sdk->getVersion();
        if (ver.empty())
            return -2;
        strncpy(w, ver.c_str(), BUFF_SIZE);
        return 0;
    }
    if (!strncasecmp(r, "lvr_feature:", 12)) {
        VrSDK* sdk = mDB.getSDK("lvr");
        if (!sdk)
            return -3;
        uint64_t f = sdk->getFeature(r+12);
        sprintf(w, "%lu", f);
        return 0;
    }
    if (!strncasecmp(r, "svr_feature:", 12)) {
        VrSDK* sdk = mDB.getSDK("svr");
        if (!sdk)
            return -3;
        uint64_t f = sdk->getFeature(r+12);
        sprintf(w, "%lu", f);
        return 0;
    }
    return -1;
}

}
