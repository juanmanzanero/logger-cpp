#include "logger.hpp" 
#include <chrono>
#include <thread>


int main()
{
    out(1) << bold;
    out(1) << blink;    

    out(2) << "My first message :)" << std::endl;

    out(2) << red;
    out(1) << "My second message :)" << std::endl;
    out(0) << reset;
    out(0) << "My third message :)" << std::endl;

    out(0).print_all();

    std::cout << "aaa" << '\r' << std::endl;

    for (size_t i = 0 ; i < 20; ++i )
    {
        out(0).spin_bar("spinning bar: ");
        std::chrono::milliseconds dura(100);
        std::this_thread::sleep_for( dura );

        out(0) << "aa" << std::endl;
    }
    out(0).stop_spinning_bar();

    for (size_t i = 0; i <= 50; ++i)
    {
        out(0).progress_bar("my bar: ", i,50);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        out(0) << " aaa " << std::endl;
    }

    out(0).stop_progress_bar();

    for (size_t i = 0; i <= 50; ++i)
    {
        out(0).progress_bar("my bar: ", (double)i/50.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        out(0) << " aaa " << std::endl;
    }

    out(0).stop_progress_bar();

}
