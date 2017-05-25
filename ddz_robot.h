#ifndef DDZ_ROBOT_H
#define DDZ_ROBOT_H

#include <cstring>
#include <cstdio>

#include "boost/asio.hpp"
#include "dooqu_service.h"

using namespace dooqu_service::service;


class ddz_robot : public game_client
{
    public:
        ddz_robot(io_service& ios);
        virtual ~ddz_robot();

        boost::system::error_code connect_to(const char* ip_address, int port);

        void fill(char* id, char* name, char* game_id);

        void start_receive();

        //向游戏发送登录信息
        void logon();

        void set_available(bool available)
        {
            this->available_ = available;
        }

        char* game_id()
        {
            return game_id_;
        }

        void set_error_code(int error_code)
        {
            this->error_code_ = error_code;
        }

        int64_t tick_count()
        {
            return this->tick_count_.elapsed();
        }

        void update_tick()
        {
            this->tick_count_.restart();
        }

        void reset()
        {
            boost::system::error_code err_code;
            this->t_socket.shutdown(boost::asio::socket_base::shutdown_both, err_code);
            this->t_socket.close(err_code);
        }

        void print_send_buffer()
        {
            __lock__(this->send_buffer_lock_, NULL);

            std::cout << this->id() <<":" << "buffer-size:" <<this->send_buffer_sequence_.size() << std::endl;
            std::cout << "recv:" << this->buffer << std::endl;
            std::cout << "buffer_pos" << this->buffer_pos << std::endl;

            this->buffer_pos = 0;
            this->p_buffer = &this->buffer[0];

//            for(int i = 0; i < this->send_buffer_sequence_.size(); i++)
//            {
//                buffer_stream* stream = this->send_buffer_sequence_.at(i);
//
//                for(int n = 0; n < stream->size(); n++)
//                {
//                    printf("%c", stream->at(n));
//                }
//
//                printf("=%d\n", stream->size());
//            }
        }



    protected:
        char game_id_[32];

        void show_pokers();

        dooqu_service::util::tick_count tick_count_;

    private:
};

#endif // DDZ_ROBOT_H
