#include "ServerListener.h"
#include "ClientConnector.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>
#include <utils/Log.h>
#include <cutils/sockets.h>

#include <vector>

namespace VR {

#define XML_PATH_PREFIX     "/system/vendor/etc"
#define SOCKET_NAME         "vrserver_socket"

ServerListener::ServerListener()
    : mWakeupRead(-1), mWakeupWrite(-1), mListenFd(-1)
{
    ALOGI("ServerListener::ServerListener()\n");
    mXmlData.parser(XML_PATH_PREFIX"/svrinfo.xml");
    mXmlData.parser(XML_PATH_PREFIX"/lvrinfo.xml");
#if VRS_DEBUG
    mXmlData.debugShow();
#endif
}

ServerListener::~ServerListener()
{
}

int
ServerListener::addConnector(int confd)
{
    ALOGI("addConnector(%d)\n", confd);
    std::list<Connector*>::iterator iter;
    for (iter = mConnectors.begin(); iter != mConnectors.end(); ++iter) {
        if ((*iter)->getConnID() == confd)
            return -1;
    }
    ClientConnector *cc = new ClientConnector(confd, mXmlData);
    if (cc) {
        mConnectors.push_back(cc);
        // _LoopRefresh();
    }
    return 0;
}

int
ServerListener::delConnector(int confd)
{
    ALOGI("delConnector(%d)\n", confd);
    std::list<Connector*>::iterator iter;
    for (iter = mConnectors.begin(); iter != mConnectors.end(); ++iter) {
        if ((*iter)->getConnID() == confd) {
            delete *iter;
            mConnectors.erase(iter);
            break;
        }
    }
    return 0;
}

int
ServerListener::_LoopRefresh()
{
    int nouse = 1;
    return write(mWakeupWrite, &nouse, sizeof(nouse));
}

void
ServerListener::run()
{
    ALOGI("ServerListener thread run start!\n");
    if (0 == mXmlData.getSDKSCount()) {
        ALOGE("No sdk service!\n");
        return;
    }
    int ret = -1;
    int rwfds[2];
    ret = pipe(rwfds);
    if (ret < 0) {
        printf("pipe error!\n");
        return;
    }
    mWakeupRead = rwfds[0];
    mWakeupWrite = rwfds[1];
    fcntl(mWakeupRead, F_SETFL, O_NONBLOCK);

    int listenfd = android_get_control_socket(SOCKET_NAME);
    ret = listen(listenfd , 0);
    if (ret < 0) {
        ALOGE("listen error! %s\n", strerror(errno));
        close(listenfd );
        return;
    }
    mListenFd = listenfd;
    fcntl(mListenFd, F_SETFL, O_NONBLOCK);

    socklen_t len;
    int nfds = -1;
    int maxfd = -1;
    fd_set rset;

    bool rQuit = false;
    std::vector<int> quitConns;
    std::list<Connector*>::iterator iter;
    for (;;) {
        FD_ZERO(&rset);
        FD_SET(mWakeupRead, &rset);
        FD_SET(mListenFd, &rset);

        maxfd = mWakeupRead > mListenFd ? mWakeupRead : mListenFd;
        for (iter = mConnectors.begin(); iter != mConnectors.end(); ++iter) {
            ret = (*iter)->preSelect(&rset);
            if (ret > maxfd)
                maxfd = ret;
        }

        nfds = select(maxfd + 1, &rset, 0, 0, 0);
        if (-1 == nfds) {
            if (EINTR == errno)
                continue;
            return;
        }
        if (FD_ISSET(mWakeupRead, &rset)) {
            int nouse = -1;
            read(mWakeupRead, &nouse, sizeof(nouse));
            if (1 == nfds)
                continue;
        }
        if (FD_ISSET(mListenFd, &rset)) {
            struct sockaddr_un cliaddr;
            int confd = -1;
            confd = accept(mListenFd, (struct sockaddr*)&cliaddr, &len);
            if (confd > 0) {
                fcntl(confd, F_SETFL, O_NONBLOCK);
                addConnector(confd);
            }
            if (1 == nfds)
                continue;
        }

        for (iter = mConnectors.begin(); iter != mConnectors.end(); ++iter) {
            ret = (*iter)->postSelect(&rset);
            if (ret == RESULT_QUIT) {
                quitConns.push_back((*iter)->getConnID());
                if (!rQuit)
                    rQuit = true;
            }
        }
        if (rQuit) {
            int count = (int)quitConns.size();
            for (int i = 0; i < count; ++i) {
                delConnector(quitConns[i]);
                close(quitConns[i]);
            }
            quitConns.clear();
            rQuit = false;
        }
    }

    ALOGI("ServerListener thread run exit!\n");
}

ServerListener&
ServerListener::self()
{
    static ServerListener *gSL = 0;
    if (!gSL)
        gSL = new ServerListener();
    return *gSL;
}

}
