#include "ddz_game_info.h"

ddz_game_info::ddz_game_info() : pos_index_(-1), check_timer_(NULL)
{
    //ctor
}

ddz_game_info::~ddz_game_info()
{
    if(this->check_timer_ != NULL)
    {

    }
}


poker_list* ddz_game_info::get_pokers()
{
    return &this->pokers_;
}


void ddz_game_info::reset()
{
    this->get_pokers()->clear();
    this->set_curr_poker_info(poker_info(poker_info::ZERO, '0', 0));
    this->set_curr_landlord_index(-1);
    this->set_curr_poker_shower(NULL);
    this->set_curr_poker_shower_index(-1);
}
