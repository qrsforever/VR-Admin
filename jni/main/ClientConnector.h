#ifndef __ClientCOnnector__H_
#define __ClientCOnnector__H_

#include "Connector.h"
#include "XmlDatabase.h"

namespace VR {

class ClientConnector : public Connector {
public:
    ClientConnector(int connid, XmlDatabase& db);
    ~ClientConnector();

    virtual int preSelect(fd_set* rset);
    virtual int postSelect(fd_set* rset);

private:
    int processMessage(char* r, char* w);

private:
    XmlDatabase& mDB;
};

}

#endif
