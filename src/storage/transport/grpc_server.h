#ifndef __STORAGE_GRPC_SERVER_H
#define __STORAGE_GRPC_SERVER_H

namespace storage::transport
{
    class grpc_server
    {
        public:
            void start();
            void stop();
    };
}

#endif