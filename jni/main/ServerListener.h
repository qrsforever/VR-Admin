#ifndef __ServerListener__H_
#define __ServerListener__H_

#include "Thread.h"
#include "Connector.h"
#include "XmlDatabase.h"

#include <list>

namespace VR {

class ServerListener : public Thread {
public:
    ServerListener();
    ~ServerListener();

    static ServerListener& self();

    int addConnector(int confd);
    int delConnector(int confd);

protected:
    void run();

private:
    int _LoopRefresh();

private:
    int mWakeupRead;
    int mWakeupWrite;
    int mListenFd;
    std::list<Connector*> mConnectors;
    XmlDatabase mXmlData;
};

}


#endif
