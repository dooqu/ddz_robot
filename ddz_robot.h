#ifndef DDZ_ROBOT_H
#define DDZ_ROBOT_H


#include <cstring>
#include <cstdio>

#include "boost/asio.hpp"
#include "dooqu_service.h"

using namespace dooqu_service::service;


class ddz_robot : public dooqu_service::service::game_session<tcp_stream>
{
    public:
        ddz_robot(io_service& ios);
        virtual ~ddz_robot();

        void fill(char* id, char* name, char* game_id);

        void connect_server(const char* ip_address, int port, std::function<void(void)> callback);

        char* game_id()
        {
            return game_id_;
        }

        void reset()
        {
            boost::system::error_code err_code;
            this->async_close();
        }

        std::shared_ptr<ddz_robot> get_robot_ptr()
        {
            return std::dynamic_pointer_cast<ddz_robot>(this->shared_from_this());
        }

        void start_ws()
        {
            this->start_handshake();
        }

        void start_handshake();

    protected:
        char game_id_[32];

        void show_pokers();

        

        void start_receive_handshake_resp();

        boost::asio::streambuf buf_;

    private:
};

#endif // DDZ_ROBOT_H
