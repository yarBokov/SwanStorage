#ifndef __STORAGE_SERVICE_H
#define __STORAGE_SERVICE_H

#include "libs/common/systemd_base_unit.h"

namespace storage
{
    class service final : public common::systemd_base_unit
    {
        public:
            ~service() override;

        private:
            bool initialize() override;
            void finalize() override;

        private:
            transport::grpc_server m_grpc_server;
    };
}

#endif