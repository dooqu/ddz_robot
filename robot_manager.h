#ifndef ROBOT_MANAGER_H
#define ROBOT_MANAGER_H


#include <set>
#include <thread>
#include "dooqu_service.h"
#include "ddz_robot.h"
#include "ddz_game_info.h"
#include "poker_finder.h"
#include "poker_parser.h"
#include "poker_info.h"

using namespace dooqu_service::service;
using namespace dooqu_server::ddz;
using namespace dooqu_service::util;

typedef std::set<ddz_robot*> robot_list;

class robot_manager : public command_dispatcher, public async_task
{
    public:
        robot_manager(io_service& ios);
        virtual ~robot_manager();

        void create_robot(io_service& ios, char* id, char* name, char* game_id);
        void online_all_robots();
        void offline_all_robots();

    protected:
        robot_list robots_;
        boost::asio::io_service::work* work_;
        std::set<std::thread*> threads_;
        void dispatch_bye(game_client*);
        void show_robot_pokers(ddz_robot* robot);
        poker_list poker_values;
        void update_robots();
        bool is_running;

        //
        void on_robot_logined(ddz_robot* robot, command* command);
        //
        void on_robot_in_desk(ddz_robot* robot, command* command);


        void on_desk_ready(ddz_robot* robot, command* command);
        //
        void on_desk_game_started(ddz_robot* robot, command* command);

        void on_list_desk_client(ddz_robot* robot, command* command);


        void on_desk_bid(ddz_robot* robot, command* command);
        void on_desk_landlord(ddz_robot* robot, command* command);


        void on_desk_poker_show(ddz_robot* robot, command* command);
        void on_desk_game_stoped(ddz_robot* robot, command* command);
        void on_robot_error(ddz_robot* robot, command* command);

        void check_find_other_desk(ddz_robot* robot)
        {
            ddz_game_info* game_info = robot->get_game_info<ddz_game_info>();

            if(game_info == NULL)
                return;

            if(robot->available() && game_info->is_game_started() == false)
            {
                robot->write("IDK%c", NULL);
                std::cout << robot->id() << "检查超时，发送idk" << std::endl;
            }
            game_info->set_check_timer(NULL);
        }

        /*
        virtual void on_desk_poker_refuse(ddz_robot* robot, command* command);

        virtual void on_not_client_card(ddz_robot* robot, command* command);
        //virtual void robot_manager::on_robot_error(ddz_robot* robot, command* command)
        */

};

#endif // ROBOT_MANAGER_H
