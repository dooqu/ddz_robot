#include <iostream>
#include <thread>
#include <functional>
#include "boost/asio.hpp"
#include "robot_manager.h"


using namespace std;



int main()
{
    srand(time(NULL));

    {
        boost::asio::io_service ios;

        robot_manager manager(ios);

        char room_id[10] = {0};
        char robot_id[10] = {0};

        int g_robot_id = 0;
        int g_room_id = 0;

        for(int room_index = 0; room_index < 60; ++room_index)
        {
            for(int i = 0; i < 300; i++)
            {
                sprintf(robot_id, "robot_%d", g_room_id++);
                sprintf(room_id, "ddz_%d", room_index);
                manager.create_robot(ios, robot_id, robot_id, room_id);
            }
        }

        std::cout << "all robot created" << std::endl;

        manager.online_all_robots();

        cin.get();
    }

    cin.get();

    return 0;
}
