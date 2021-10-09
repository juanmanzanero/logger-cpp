#include "logger.hpp"
#include <chrono>
#include <thread>


int main()
{
    out(0).set_print_level(2);
    out(1) << bold;
    out(1) << blink;    

    out(2) << "print level 2" << std::endl;

    out(2) << reset << green;
    out(1) << "print level 1" << std::endl;
    out(0) << reset;
    out(0) << "print level 0" << std::endl;


    for (size_t i = 0 ; i < 20; ++i )
    {
        out(0).spin_bar("spinning bar: ");
        std::chrono::milliseconds dura(50);
        std::this_thread::sleep_for( dura );
    }
    out(0).stop_spinning_bar();

    for (size_t i = 0; i <= 50; ++i)
    {
        out(0).progress_bar("my bar: ", i,50);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

    }

    out(0).stop_progress_bar();

    out(0) << std::endl;
    for (size_t i = 0; i <= 50; ++i)
    {
        out(0).progress_bar("my bar: ", (double)i/50.0);
        if ( i % 10 == 0 )
            out(2) << "i = " << i << " completed" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

    }

    out(0).stop_progress_bar();

    std::cout << " SAVED LOGS ---------------------- " << std::endl;
    out(0).print_all();
}
