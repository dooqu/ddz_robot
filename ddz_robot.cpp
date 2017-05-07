#include "ddz_robot.h"

ddz_robot::ddz_robot(io_service& ios) : game_client(ios), game_id_({0})
{

}

ddz_robot::~ddz_robot()
{
    if(this->game_info_ != NULL)
        delete this->game_info_;

    this->game_info_ = NULL;
}

boost::system::error_code ddz_robot::connect_to(const char* ip_address, int port)
{
    boost::system::error_code result_code;

    this->socket().connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip_address), port), result_code);
    return result_code;
}


void ddz_robot::start_receive()
{
    this->read_from_client();
}


void ddz_robot::logon()
{
    this->write("LOG %s %s%c", this->game_id(), this->id(), NULL);
}

void ddz_robot::fill(char* id, char* name, char* game_id)
{
    this->game_client::fill(id, name, NULL);

    int len = min(strlen(game_id), sizeof(game_id_) - 1);
    std::strncpy(game_id_, game_id, len);
}


