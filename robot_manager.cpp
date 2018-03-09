#include "robot_manager.h"

robot_manager::robot_manager(io_service& ios) : work_(NULL), async_task(ios), is_running(false)
{
    this->poker_values.insert("A1");
    this->poker_values.insert("A2");
    this->poker_values.insert("A3");
    this->poker_values.insert("A4");

    this->poker_values.insert("B1");
    this->poker_values.insert("B2");
    this->poker_values.insert("B3");
    this->poker_values.insert("B4");

    this->poker_values.insert("C1");
    this->poker_values.insert("C2");
    this->poker_values.insert("C3");
    this->poker_values.insert("C4");

    this->poker_values.insert("D1");
    this->poker_values.insert("D2");
    this->poker_values.insert("D3");
    this->poker_values.insert("D4");

    this->poker_values.insert("E1");
    this->poker_values.insert("E2");
    this->poker_values.insert("E3");
    this->poker_values.insert("E4");

    this->poker_values.insert("F1");
    this->poker_values.insert("F2");
    this->poker_values.insert("F3");
    this->poker_values.insert("F4");

    this->poker_values.insert("G1");
    this->poker_values.insert("G2");
    this->poker_values.insert("G3");
    this->poker_values.insert("G4");


    this->poker_values.insert("H1");
    this->poker_values.insert("H2");
    this->poker_values.insert("H3");
    this->poker_values.insert("H4");

    this->poker_values.insert("I1");
    this->poker_values.insert("I2");
    this->poker_values.insert("I3");
    this->poker_values.insert("I4");

    this->poker_values.insert("J1");
    this->poker_values.insert("J2");
    this->poker_values.insert("J3");
    this->poker_values.insert("J4");

    this->poker_values.insert("K1");
    this->poker_values.insert("K2");
    this->poker_values.insert("K3");
    this->poker_values.insert("K4");

    this->poker_values.insert("L1");
    this->poker_values.insert("L2");
    this->poker_values.insert("L3");
    this->poker_values.insert("L4");

    this->poker_values.insert("M1");
    this->poker_values.insert("M2");
    this->poker_values.insert("M3");
    this->poker_values.insert("M4");

    this->poker_values.insert("N1");
    this->poker_values.insert("O1");
    this->regist_handle("LOG", make_handler(robot_manager::on_robot_logined));
    this->regist_handle("IDK", make_handler(robot_manager::on_robot_in_desk));
    this->regist_handle("LSD", make_handler(robot_manager::on_list_desk_client));
    this->regist_handle("RDY", make_handler(robot_manager::on_desk_ready));
    this->regist_handle("STT", make_handler(robot_manager::on_desk_game_started));
    this->regist_handle("BID", make_handler(robot_manager::on_desk_bid));
    this->regist_handle("LRD", make_handler(robot_manager::on_desk_landlord));
    //this->regist_handle("ERR", make_handler(robot_manager::on_robot_error));

    this->regist_handle("CAD", make_handler(robot_manager::on_desk_poker_show));
    this->regist_handle("STP", make_handler(robot_manager::on_desk_game_stoped));
    /*
    this->regist_handle("RFS", make_handler(robot_manager::on_desk_poker_refuse));
    this->regist_handle("NOT", make_handler(robot_manager::on_not_client_card));
    */
}

robot_manager::~robot_manager()
{
    this->unregist_all_handles();
    this->offline_all_robots();
    this->robots_.clear();
    std::cout << "~robot_manager" << std::endl;
}


void robot_manager::create_robot(io_service& ios, char* id, char* name, char* game_id)
{
    static int g_id = 0;
    ddz_robot* robot = new ddz_robot(ios);
    std::shared_ptr<ws_session<tcp_stream>> new_robot(robot);
    ddz_game_info* game_info = new ddz_game_info();
    robot->set_game_info(game_info);
    robot->fill(id, name, game_id);
    robot->index = g_id ++;
    this->robots_.insert(new_robot);
}


void robot_manager::online_all_robots()
{
    if(this->is_running == true || this->work_ != NULL)
        return;

    this->is_running = true;

    this->work_ = new boost::asio::io_service::work(this->task_io_service_);

    for(int i = 0; i < 5;i ++)
    {
        std::thread* t = new thread(std::bind(static_cast<size_t(io_service::*)()>(&io_service::run), &this->task_io_service_));
        this->threads_.insert(t);
        service_status::instance()->init(t->get_id());
    }

    std::vector<ddz_robot*> all_robots;
    robot_list::iterator e = this->robots_.begin();
    while (e != this->robots_.end())
    {
        std::shared_ptr<ws_session<tcp_stream>> ws_session_ptr = (*e);
        ws_session<tcp_stream>* ws_session = ws_session_ptr.get();
        ddz_robot* robot = dynamic_cast<ddz_robot*>(ws_session);
        robot->set_command_dispatcher(this);
        boost::system::error_code err;
        robot->socket().connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 8000), err);
        if(!err)
        {            
            std::cout << "robot:" << robot->id() << " connected." << std::endl;
            robot->set_available(true);
            robot->start_handshake();
        }
        else
        {
            std::cout << "robot:" << robot->id() << " failed, reason:" << err.message() << std::endl;
        }
        ++e;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));        
    }
}


void robot_manager::offline_all_robots()
{
    if(this->is_running == false)
        return;
    this->is_running = false;
    delete this->work_;
    this->work_ = NULL;
    this->cancel_all_task();
    {
        ___lock___(this->robot_list_mutex_, "robot_manager.offline_all_robots");
        for(robot_list::iterator e = this->robots_.begin(); e != this->robots_.end(); ++e)
        {
            ddz_robot* robot = (ddz_robot*)(*e).get();
            robot->reset();
        }
    }
    for(std::set<std::thread*>::iterator e = this->threads_.begin(); e != this->threads_.end(); ++e)
    {
        std::cout << "wait for thread exit..." << std::endl;
        (*e)->join();
    }
    this->task_io_service_.stop();
    printf("all robots offlined.\n");
}


void robot_manager::dispatch_bye(ws_client* client)
{
    ddz_robot* robot = (ddz_robot*)client;
    robot->set_available(false);
    robot->set_command_dispatcher(NULL);
    ___lock___(this->robot_list_mutex_, "robot_manager.dispatch_bye");
    this->robots_.erase(robot->shared_from_this());
}


void robot_manager::show_robot_pokers(ddz_robot* robot)
{
    poker_array pokers_finded;
    ddz_game_info* game_info = robot->get_game_info<ddz_game_info>();

    if (game_info->curr_poker_info().type != poker_info::ZERO)
    {
        poker_finder::find_big_pokers(game_info->curr_poker_info(), *game_info->get_pokers(), pokers_finded);
    }
    else
    {
        int r = dooqu_service::util::random(1, 10);

        switch (r)
        {
        case 1:
        case 2:
        case 3:

            for (int i = 3; i >= 2; i--)
            {
                poker_finder::find_three_chain_and_single(*game_info->get_pokers(), '0', i * 4, pokers_finded);
                if (pokers_finded.size() == i * 4)
                {
                    goto LABEL_DEFAULT;
                }
                else if (pokers_finded.size() != 0)
                {
                    assert(0);
                }
            }
            for (int i = 5; i >= 3; i--)
            {
                poker_finder::find_double_chain(*game_info->get_pokers(), '0', i * 2, pokers_finded);
                if (pokers_finded.size() == i * 2)
                {
                    goto LABEL_DEFAULT;
                }
                else if (pokers_finded.size() != 0)
                {
                    assert(0);
                }
            }
            for (int i = 7; i >= 5; i--)
            {
                poker_finder::find_single_chain(*game_info->get_pokers(), '0', i, pokers_finded);

                if (pokers_finded.size() == i)
                {
                    goto LABEL_DEFAULT;
                }
                else if (pokers_finded.size() != 0)
                {
                    assert(0);
                }
            }

            if (pokers_finded.size() != 0)
            {
                printf("%d", pokers_finded.size());
            }

            poker_finder::find_double(*game_info->get_pokers(), '0', pokers_finded);

            if (pokers_finded.size() != 2 && pokers_finded.size() != 0)
            {
                printf("%d", pokers_finded.size());
            }
            assert(pokers_finded.size() == 2 || pokers_finded.size() == 0);
            if (pokers_finded.size() == 2)
            {
                goto LABEL_DEFAULT;
            }

            poker_finder::find_single(*game_info->get_pokers(), '0', pokers_finded);
            if (pokers_finded.size() == 1)
            {
                goto LABEL_DEFAULT;
            }
            else
            {
                goto LABEL_SINGLE;
            }
            break;

        case 4:
        case 5:
        case 6:
            poker_finder::find_three_and_double(*game_info->get_pokers(), '0', pokers_finded);
            assert(pokers_finded.size() == 5 || pokers_finded.size() == 0);
            if (pokers_finded.size() == 5)
            {
                goto LABEL_DEFAULT;
            }
            else if (pokers_finded.size() != 0)
            {
                assert(0);
            }

            poker_finder::find_three_and_single(*game_info->get_pokers(), '0', pokers_finded);
            assert(pokers_finded.size() == 4 || pokers_finded.size() == 0);
            if (pokers_finded.size() == 4)
            {
                goto LABEL_DEFAULT;
            }
            else if (pokers_finded.size() != 0)
            {
                assert(0);
            }
            poker_finder::find_double(*game_info->get_pokers(), '0', pokers_finded);
            assert(pokers_finded.size() == 2 || pokers_finded.size() == 0);
            if (pokers_finded.size() == 2)
            {
                goto LABEL_DEFAULT;
            }
            else if (pokers_finded.size() != 0)
            {
                assert(0);
            }

            goto LABEL_SINGLE;
            break;


        case 7:
        case 8:
        case 9:
            poker_finder::find_four_and_two_single(*game_info->get_pokers(), '0', pokers_finded);
            assert(pokers_finded.size() == 6 || pokers_finded.size() == 0);
            if (pokers_finded.size() > 0)
            {
                goto LABEL_DEFAULT;
            }

            poker_finder::find_double(*game_info->get_pokers(), '0', pokers_finded);
            assert(pokers_finded.size() == 2 || pokers_finded.size() == 0);
            if (pokers_finded.size() == 2)
            {
                goto LABEL_DEFAULT;
            }

            goto LABEL_SINGLE;
            break;

        case 10:
LABEL_SINGLE :
            poker_finder::find_single(*game_info->get_pokers(), '0', pokers_finded);
            break;

        default:
LABEL_DEFAULT :

            if (pokers_finded.size() == 0)
            {
                poker_finder::find_single(*game_info->get_pokers(), '0', pokers_finded);
            }
            break;
        }
    }

    if (pokers_finded.size() > 0)
    {
        char buffer[128] = { 0 };

        int n = 0;
        for (int i = 0; i < pokers_finded.size(); i++)
        {
            n += sprintf(&buffer[n], " %s", pokers_finded[i]);
        }

        robot->write_frame(true, ws_framedata::opcode::TEXT, "CAD %s%c", buffer, NULL);
    }
    else
    {
        robot->write_frame(true, dooqu_service::basic::ws_framedata::opcode::TEXT, "CAD 0%c", NULL);
    }
}


void robot_manager::on_robot_logined(ddz_robot* robot, command* command)
{
    std::shared_ptr<ddz_robot> robot_ptr = robot->get_robot_ptr();
    this->queue_task([robot_ptr]()
    {
        robot_ptr->write_frame(true, dooqu_service::basic::ws_framedata::opcode::TEXT, "IDK%c", NULL);
    }, 
    dooqu_service::util::random(1000, 3000));
}


void robot_manager::on_robot_in_desk(ddz_robot* robot, command* command)
{
    std::shared_ptr<ddz_robot> robot_ptr = robot->get_robot_ptr();
    int desk_index = std::atoi(command->params(0));
    if (desk_index >= 0)
    {
        ddz_game_info* game_info = robot->get_game_info<ddz_game_info>();
        game_info->set_desk_id(desk_index);
        game_info->set_pos_index(std::atoi(command->params(1)));
        this->queue_task([robot_ptr]()
        {
            robot_ptr->write_frame(true, dooqu_service::basic::ws_framedata::opcode::TEXT,  "RDY%c", NULL);
        }, 
        dooqu_service::util::random(1000, 3030));
    }
    else
    {
        this->queue_task([robot_ptr]()
        {
            robot_ptr->write_frame(true, dooqu_service::basic::ws_framedata::opcode::TEXT,  "IDK%c", NULL);
        }, 
        dooqu_service::util::random(1000, 3030));
    }
}

void robot_manager::on_list_desk_client(ddz_robot* robot, command* command)
{
    //如果当前的lsd是当前的机器人，记住该机器人的位置索引
    if (std::strcmp(command->params(0), robot->id()) == 0)
    {
    }
}

void robot_manager::on_desk_ready(ddz_robot* robot, command* command)
{
    if (std::strcmp(command->params(0), robot->id()) == 0)
    {        
        ddz_game_info* game_info = robot->get_game_info<ddz_game_info>();
        std::shared_ptr<ddz_robot> robot_ptr = robot->get_robot_ptr();
        task_timer* check_timer = this->queue_task([robot_ptr, this, robot]()
        {
            if(robot_ptr->is_available())
            {
                this->check_find_other_desk(robot);
            }            
        }, 
        dooqu_service::util::random(20000, 40000), true);
        game_info->set_check_timer(check_timer);
    }
}


void robot_manager::on_desk_game_started(ddz_robot* robot, command* command)
{
    ddz_game_info* game_info = robot->get_game_info<ddz_game_info>();
    game_info->set_game_started(true);

    //检查空卓的定时器取消
    task_timer* timer = game_info->get_check_timer();

    if(timer != NULL)
    {
        this->cancel_task(timer);
        game_info->set_check_timer(NULL);
    }

     //清空牌面
    game_info->reset();

    for (int i = 0; i < command->param_size(); ++i)
    {
        poker_list::iterator finder = this->poker_values.find(command->params(i));
        if (finder != this->poker_values.end())
        {
            game_info->get_pokers()->insert(*finder);
        }
    }
    //有无大王或者炸弹
    dooqu_server::ddz::poker_array pokers_finded;
    dooqu_server::ddz::poker_finder::find_bomb_or_double_joker(*game_info->get_pokers(), pokers_finded);

    if(pokers_finded.size() > 0)
    {
        int r = random(0, 5);

        if (r < 3)
        {
            std::shared_ptr<ddz_robot> robot_ptr = robot->get_robot_ptr();
            this->queue_task([robot_ptr]()
            {
                robot_ptr->write_frame(true, dooqu_service::basic::ws_framedata::opcode::TEXT, "DLA%c", NULL);
            }, 
            dooqu_service::util::random(1000, 2000));
        }
    }
}


void robot_manager::on_desk_bid(ddz_robot* robot, command* command)
{
    ddz_game_info* game_info = robot->get_game_info<ddz_game_info>();
    int bid_index = std::atoi(command->params(0));
    if (game_info->get_pos_index() == bid_index)
    {
        std::shared_ptr<ddz_robot> robot_ptr = robot->get_robot_ptr();
        this->queue_task([robot_ptr]()
        {
            robot_ptr->write_frame(true, dooqu_service::basic::ws_framedata::opcode::TEXT, "BID %c%c", (dooqu_service::util::random(0, 5) == 0)? '0' : '1', NULL);
        },
        dooqu_service::util::random(1000, 2200));
    }
}


void robot_manager::on_desk_landlord(ddz_robot* robot, command* command)
{
    int pos_index = std::atoi(command->params(0));

    ddz_game_info* game_info = robot->get_game_info<ddz_game_info>();
    game_info->set_curr_landlord_index(pos_index);

    if (game_info->get_pos_index() == pos_index)
    {
        //底牌加到牌面列表中
        for (int i = 1; i < command->param_size(); i++)
        {
            poker_list::iterator finder = this->poker_values.find(command->params(i));

            if (finder != this->poker_values.end())
            {
                game_info->get_pokers()->insert(*finder);
            }
        }
        std::shared_ptr<ddz_robot> robot_ptr = robot->get_robot_ptr();
        this->queue_task([robot_ptr, this, robot]()
        {
            if(robot_ptr->is_available())
            {
                this->show_robot_pokers(robot);
            }            
        }, 
        random(2000, 3000));
    }
}


void robot_manager::on_desk_poker_show(ddz_robot* robot, command* command)
{
    int pos_index = std::atoi(command->params(0));
    //分析牌面信息 CAD 0 A1,A2 0,0
    poker_info pi;
    int cards_str_len = std::strlen(command->params(1));

    ddz_game_info* game_info = robot->get_game_info<ddz_game_info>();

    //如果当前用户不是过牌
    if (std::strcmp(command->params(1), "0") != 0)
    {
        char cards[20] = { 0 };
        int char_index = 0;
        //牌型值的索引位置规律是:0,3,6...，所以，依次取对应索引的char值，放在char数组中，用来分析牌型
        for (int i = 0; i < cards_str_len; i += 3, char_index++)
        {
            cards[char_index] = command->params(1)[i];
        }
        poker_parser::parse(cards, char_index, pi);
        //更新当前机器人的牌面信息
        if (game_info->get_pos_index() == pos_index)
        {
            int del_bef = game_info->get_pokers()->size();
            for (int i = 0; i < cards_str_len; i += 3)
            {
                command->params(1)[i + 2] = 0;
                const char* poker_val_del = &command->params(1)[i];
                game_info->get_pokers()->erase(poker_val_del);
                //printf("\n%s 删除牌面:%s", robot->robot_name(), poker_val_del);
            }
        }
    }

    //如果当前出牌的玩家是要牌了，那么更新当前的最新牌型和出牌者信息
    if (pi.type != poker_info::ZERO)
    {
        game_info->set_curr_poker_info(pi);
        game_info->set_curr_poker_shower_index(pos_index);
    }

    if (command->param_size() > 2)
    {
        char digit_buffer[2] = { 0 };
        sprintf(digit_buffer, "%c", command->params(2)[0]);

        //如果系统希望出牌的位置上是当前的机器人
        if (game_info->get_pos_index() == std::atoi(digit_buffer))
        {
            //如果下一个要出牌的机器人管的牌是自己的
            if (game_info->curr_poker_shower_index() == game_info->get_pos_index())
            {
                poker_info p;
                game_info->set_curr_poker_info(p);
            }

            //this->queue_task(std::bind(&robot_manager::show_robot_pokers, this, robot), random(1000, 3000));
            std::shared_ptr<ddz_robot> robot_ptr = robot->get_robot_ptr();
            this->queue_task([robot_ptr, this, robot]()
            {
                if(robot_ptr->is_available())
                {
                    this->show_robot_pokers(robot);
                }                
            }, 
            random(2000, 3000));
        }
    }
}

void robot_manager::on_desk_game_stoped(ddz_robot* robot, command* command)
{
    std::cout << robot->id() <<" game stop {" << command->params(0) << "}" << std::endl;
    //设定非游戏状态
    //robot->set_game_is_running(false);
    ddz_game_info* game_info = robot->get_game_info<ddz_game_info>();

    game_info->set_game_started(false);
    //清空手里的牌面
    game_info->reset();

    int r = random(0, 3);
    int sleepMilli = random(2000, 5000);

    std::shared_ptr<ddz_robot> robot_ptr = robot->get_robot_ptr();
    if(r == 1)
    {
        //如果r==1，那么换一桌
        this->queue_task([robot_ptr]()
        {
            robot_ptr->write_frame(true, dooqu_service::basic::ws_framedata::opcode::TEXT, "IDK%c", NULL);
        }, 
        sleepMilli);
    }
    else
    {
        //如果r != 1，那么直接准备下一局
        this->queue_task([robot_ptr]()
        {
            robot_ptr->write_frame(true, dooqu_service::basic::ws_framedata::opcode::TEXT, "RDY%c", NULL);
        }, 
        sleepMilli);
    }
}

