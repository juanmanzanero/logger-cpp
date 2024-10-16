#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include "logger.h" 

#include <algorithm>


inline Logger Logger::out(std::cout,n_print_levels-1);
inline Logger Logger::err(std::cerr,n_print_levels-1);

inline Logger& Logger::operator()(const size_t current_print_level)
{
    _current_print_level = current_print_level;

    return *this;
}


inline Logger& Logger::operator<<(const Color& c)
{
    if ( _is_tty && (_print_level >= _current_print_level) )
    {
        _sOut << "\033[" << c.get_code() << "m";
    }

    return *this;
}


template<class T>
inline Logger& Logger::operator<<(const T& t)
{
    if (_current_print_level < n_print_levels) {
        std::string logdate;
        if (_logdate && _is_first_message) {
            auto time = std::time(nullptr);
            std::ostringstream sout;
#ifdef _WIN32
            struct tm buf;
            gmtime_s(&buf, &time);
            char threadstr[10];
            sprintf_s(threadstr, "%05d", GetCurrentThreadId());
            sout << "[(" << threadstr << ")" << std::put_time(&buf, "%FT%T") << "]";
#else
            sout << std::put_time(std::gmtime(&time), "%FT%T%z");
#endif
            logdate = sout.str();
            _is_first_message = false;
        }

        if (_print_level >= _current_print_level)
        {
            if (_is_tty && _spinning_bar_on)
            {
                stop_spinning_bar();
            }

            if (_is_tty && _progress_bar_on)
            {
                stop_progress_bar();
            }

            _sOut << logdate << t;
        }

        for (size_t i = _current_print_level; i < n_print_levels; ++i)
        {
            _ss[i] << logdate << t;
        }
    }
    return *this;
}


inline Logger& Logger::operator<<(std::ostream&(*f)(std::ostream&))
{
    if (_current_print_level < n_print_levels) {

        if (_print_level >= _current_print_level)
        {

            if (_is_tty && _spinning_bar_on)
            {
                stop_spinning_bar();
            }

            if (_is_tty && _progress_bar_on)
            {
                stop_progress_bar();
            }

            (*f)(_sOut);
        }

        for (size_t i = _current_print_level; i < n_print_levels; ++i)
        {
            (*f)(_ss[i]);
        }

        _is_first_message = true;
    }
    
    return *this;
}


inline void Logger::print_all() const
{
    for (size_t i = 0; i < n_print_levels; ++i)
    {
        std::cout << "Print level i = " << i << std::endl;
        std::cout << _ss[i].str();
        std::cout << "------------------- " << std::endl;
    }
}


inline void Logger::spin_bar(const std::string& header)
{
    if ( _is_tty && (_print_level >= _current_print_level) )
    {
        _spinning_bar_on = true;
        _sOut << "\33[2K\r" ;
        _sOut << header << _spinning_bar_chars[_spinning_bar_counter];
        _spinning_bar_counter++;
        _spinning_bar_counter = (_spinning_bar_counter) % 4;
        _sOut << std::flush;
    }
}


inline void Logger::stop_spinning_bar()
{
    if ( _is_tty && _spinning_bar_on)
    {
        _sOut << "\33[2K\r" ;
        _spinning_bar_on = false;
    }
}


inline void Logger::progress_bar(const std::string& header, const int curr, const int tot)
{
    if ( _is_tty && (_print_level >= _current_print_level) )
    {
        if ( _spinning_bar_on ) stop_spinning_bar();

        _progress_bar_on = true;
        _sOut << "\015";
        _sOut << header ;

        double fills = ((double)curr / tot * _progress_bar_width);
        int ifills = (int)fills;

        for (int i = 0; i < ifills; i++) 
        {
            _sOut << _progress_bar_chars[8];
        }

        if (curr != tot)
        {
            _sOut << _progress_bar_chars[(int)(8.0)*(fills-ifills)];
        }

        for (int i = 0; i < _progress_bar_width-ifills-1; i++) 
        {
            _sOut << "▏";
        }

        _sOut << "▏";

        _sOut << " (" << curr << "/" << tot <<")";
        _sOut << std::flush;
    }
}

inline void Logger::progress_bar(const std::string& header, double percentage)
{
    if ( _is_tty && (_print_level >= _current_print_level) )
    {
        percentage = min(double{ 1.0 }, percentage);

        if ( _spinning_bar_on ) stop_spinning_bar();

        _progress_bar_on = true;
        _sOut << "\015";
        _sOut << header ;
        constexpr int tot = 1000;
        int curr = (int)(percentage*tot);

        double fills = ((double)curr / tot * _progress_bar_width);
        int ifills = (int)fills;

        for (int i = 0; i < ifills; i++) 
        {
            _sOut << _progress_bar_chars[8];
        }

        if (curr != tot)
        {
            _sOut << _progress_bar_chars[(int)(8.0)*(fills-ifills)];
        }

        for (int i = 0; i < _progress_bar_width-ifills-1; i++) 
        {
            _sOut << "▏";
        }

        _sOut << "▏";

        char buffer[10];
        sprintf_s(buffer,"%*.1f",5,percentage*100);

        _sOut << " (" << buffer << "%" <<")";
        _sOut << std::flush;
    }
}


inline void Logger::stop_progress_bar()
{
    if ( _is_tty && _progress_bar_on)
    {
        _sOut << "\33[2K\r" ;
        _progress_bar_on = false;
    }
}


inline void Logger::set_print_level(const size_t print_level)
{
    if ( print_level < n_print_levels )
    {
        _print_level = print_level;
    }
    else
    {
        throw std::runtime_error("Print level is out of bounds");
    }
}

#endif
