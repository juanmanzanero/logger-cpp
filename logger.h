#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#define isatty _isatty
#endif

#include <iostream>
#include <array>
#include <sstream>
#include <iomanip>

constexpr size_t n_print_levels = 3;

class Color
{
 public:
    Color(const std::string& color_code) : _color_code(color_code) {}

    constexpr const std::string& get_code() const { return _color_code; }
 private:
    const std::string _color_code;
};

inline const Color green("32");
inline const Color red("31");
inline const Color reset("0");
inline const Color bold("1");
inline const Color blink("5");
inline const Color underline("4");

class Logger
{
 public:

    Logger(std::ostream& sOut, const size_t print_level) : _sOut(sOut), _print_level(print_level) 
    {
        if ( !(print_level < n_print_levels) )
            throw std::runtime_error("Print level is higher than the number of requested print levels");

        if ( &_sOut == &std::cout)
        {
            _is_tty = isatty(1);
        }
    }

    Logger& operator()(const size_t current_print_level);

    void print_all() const;

    Logger& operator<<(const Color& c);
 
    template<class T>
    Logger& operator<<(const T& t);

    Logger& operator<<(std::ostream&(*f)(std::ostream&));

    void spin_bar(const std::string& header);

    void stop_spinning_bar();

    void progress_bar(const std::string& header, const int curr, const int tot);

    void progress_bar(const std::string& header, double percentage);

    void stop_progress_bar();

    void set_print_level(const size_t print_level);

    static Logger out;
    static Logger err;

 private:
    std::ostream& _sOut;
    std::array<std::ostringstream,n_print_levels> _ss;

    size_t _print_level         = n_print_levels;
    size_t _current_print_level = n_print_levels;
    bool _is_tty                = false;

    // Spinning bar ---------------------------
    bool _spinning_bar_on = false;
    static constexpr std::array<char,4> _spinning_bar_chars = {'/','|','-','\\'};
    int _spinning_bar_counter = 0;

    // Progress bar ----------------------------
    bool _progress_bar_on = false;
    static constexpr std::array<const char*,9> _progress_bar_chars = {" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"};
    constexpr static int _progress_bar_width = 40;

};

inline Logger& out = Logger::out;
inline Logger& err = Logger::err;

#endif
