#ifndef DDZ_GAME_INFO_H
#define DDZ_GAME_INFO_H

#include <set>
#include "dooqu_service.h"
#include "poker_info.h"
#include "ddz_robot.h"
#include "poker_finder.h"

using namespace dooqu_service::service;
using namespace dooqu_service::util;
using namespace dooqu_server::ddz;



class ddz_game_info : public game_info
{
    public:
        ddz_game_info();
        virtual ~ddz_game_info();

        void set_desk_id(int desk_id)
        {
            this->desk_id_ = desk_id;
        }
        int get_desk_id()
        {
            return this->desk_id_;
        }

        poker_list* get_pokers();

        void set_pos_index(int pos_index)
        {
            this->pos_index_ = pos_index;
        }

        int get_pos_index()
        {
            return this->pos_index_;
        }

        poker_info& curr_poker_info()
        {
            return this->curr_poker_info_;
        }

        void set_curr_poker_info(const poker_info& poker_info)
        {
            this->curr_poker_info_ = poker_info;
        }

        ddz_robot* curr_poker_shower()
        {
            return this->curr_poker_shower_;
        }
        void set_curr_poker_shower(ddz_robot* robot)
        {
            this->curr_poker_shower_ = robot;
        }

        int curr_poker_shower_index()
        {
            return this->curr_poker_shower_index_;
        }
        void set_curr_poker_shower_index(int index)
        {
            this->curr_poker_shower_index_ = index;
        }

        int curr_landlord()
        {
            return this->curr_landlord_index_;
        }
        void set_curr_landlord_index(int index)
        {
            this->curr_landlord_index_ = index;
        }

        async_task::task_timer* get_check_timer()
        {
            return this->check_timer_;
        }

        void set_check_timer(async_task::task_timer* timer)
        {
            this->check_timer_ = timer;
        }

        void set_game_started(bool started)
        {
            this->is_game_started_ = started;
        }

        bool is_game_started()
        {
            return this->is_game_started_;
        }

        void reset();

    protected:
        int desk_id_;
        poker_list pokers_;
        int pos_index_;
        poker_info curr_poker_info_;
        ddz_robot* curr_poker_shower_;
        int curr_poker_shower_index_;
        int curr_landlord_index_;
        async_task::task_timer* check_timer_;
        bool is_game_started_;

    private:
};

#endif // DDZ_GAME_INFO_H
