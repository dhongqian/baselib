#include "ServerCallback.h"

namespace hq {

ServerCallback::ServerCallback(){
}

ServerCallback::~ServerCallback() {
}

void ServerCallback::handleAcceptor(std::shared_ptr<IClient> client_ptr) {
}

void ServerCallback::handleError(const int error_code) {

}

}
