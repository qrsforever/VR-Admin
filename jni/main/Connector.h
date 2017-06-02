#ifndef __Connector__H_
#define __Connector__H_

#include <sys/select.h>

#define RESULT_ERR   -1
#define RESULT_OK    0
#define RESULT_QUIT  1

namespace VR {

class Connector {
public:
    Connector(int id) : mConnID(id) { };
    virtual ~Connector() { };
    int getConnID() const { return mConnID; }
    virtual int preSelect(fd_set* rset) = 0;
    virtual int postSelect(fd_set* rset) = 0;
protected:
    int mConnID;
};

}

#endif
