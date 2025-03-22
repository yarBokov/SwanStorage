#include "storage/service.h"

#include "storage/transport/grpc_server.h"

namespace storage
{
    service::~service() = default;

    bool service::initialize()
    {
        m_grpc_server.start();
        return true;
    }

    void service::finalize()
    {
        m_grpc_server.stop();
    }
}