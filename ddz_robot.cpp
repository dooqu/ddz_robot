#include "ddz_robot.h"

ddz_robot::ddz_robot(io_service& ios) : game_session<tcp_stream>(ios), game_id_({0})
{
}

ddz_robot::~ddz_robot()
{
    if(this->game_info_ != NULL)
        delete this->game_info_;

    this->game_info_ = NULL;
}


void ddz_robot::start_handshake()
{
    char* buffer = "GET / HTTP/1.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nHost: service.wechat.dooqu.com\r\nSec-WebSocket-Origin: https://service.wechat.dooqu.com\r\nSec-WebSocket-Key: 2SCVXUeP9cTjV+0mWB8J6A==\r\nSec-WebSocket-Version: 13\r\n\r\n";
    this->write(buffer);
    this->receive_handshake_resp();
}


void ddz_robot::receive_handshake_resp()
{
    std::shared_ptr<ws_session<tcp_stream>> self = this->shared_from_this();
    
    boost::asio::async_read_until(this->socket(), buf_, "\r\n\r\n",
        [self, this](const boost::system::error_code& error, std::size_t size)
        {
            if(!error)
            {
                this->write_frame(true, dooqu_service::basic::ws_framedata::opcode::TEXT, "LOG %s %s%c", this->game_id(), this->id(), NULL);
                this->read_from_client();
            }
            else
            {
                std::cout << "receive handshake error." << std::endl;
            }
        } 
    );
}


void ddz_robot::fill(char* id, char* name, char* game_id)
{
    this->game_session<tcp_stream>::fill(id, name, NULL);
    int len = min(strlen(game_id), sizeof(game_id_) - 1);
    std::strncpy(game_id_, game_id, len);
}


