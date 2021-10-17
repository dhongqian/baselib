#include "ManagerServerCallback.h"
#include <iostream>
#include <netwrapper/IClient.h>
#include "baselib/BufferUtility.h"

namespace hq {

ManagerServerCallback::ManagerServerCallback(){
}

ManagerServerCallback::~ManagerServerCallback() {
}

void ManagerServerCallback::handleAcceptor(std::shared_ptr<IClient> client_ptr) {

    if(nullptr != client_ptr) {
        std::cout << " accept " << client_ptr->localHostInfo().getIPString() << " : " << client_ptr->remoteHostInfo().getIPString() << std::endl;
    }

    uint64_t client_id = reinterpret_cast<uint64_t>(client_ptr.get());
    client_map_[client_id] = client_ptr;

    BufferUtilityPtr buffer_ptr = std::make_shared<BufferUtility>(1500);
    if(nullptr == buffer_ptr) return;

    std::ostringstream oss;
    oss << " send data: " << GetTickCount();
    memcpy(buffer_ptr->getBuffer(), oss.str().c_str(), oss.str().length());
    buffer_ptr->setDataLen(oss.str().length());
    client_ptr->send(buffer_ptr);
}

void ManagerServerCallback::handleError(const int error_code) {

}

}
