#include <iostream>
#include <thread>
#include <functional>
#include "boost/asio.hpp"
#include "robot_manager.h"

using namespace std;

int main()
{
    srand(time(NULL));

    service_status::create_new();
    service_status::instance()->init(std::this_thread::get_id());

    {
        cin.get();
        boost::asio::io_service ios;

        robot_manager manager(ios);

        char room_id[10] = {0};
        char robot_id[10] = {0};

        int g_robot_id = 0;

        for(int room_index = 0; room_index < 3; ++room_index)
        {
            for(int i = 0; i < 297; i++)
            {
                sprintf(robot_id, "robot_%d", g_robot_id++);
                sprintf(room_id, "ddz_%d", room_index);
                manager.create_robot(ios, robot_id, robot_id, room_id);
            }
        }
        std::cout << "all robot created" << std::endl;
        cin.get();
        manager.online_all_robots();
        std::cout << "press any key to stop..." << std::endl;
        cin.get();

        manager.offline_all_robots();

        ios.stop();
    }

    return 0;
}
