#ifndef LOGGERCPP_LOGGER_H
#define LOGGERCPP_LOGGER_H

#ifndef _MSC_VER
#include <unistd.h>
#else

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stringapiset.h>
#include <io.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#define isatty _isatty

#endif

#include <iostream>
#include <array>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>

#ifdef _MSC_VER
#define sprintf sprintf_s
#endif

namespace logger {

#ifdef _MSC_VER
inline std::wstring string_to_wstring(const std::string& input_string)
{
    std::size_t reqLength = ::MultiByteToWideChar(CP_UTF8, 0, input_string.c_str(), (int)input_string.length(), 0, 0);

    // construct new string of required length
    std::wstring input_wstring(reqLength, L'\0');

    // convert old string to new string
    ::MultiByteToWideChar(CP_UTF8, 0, input_string.c_str(), (int)input_string.length(), &input_wstring[0], (int)input_wstring.length());

    return input_wstring;
}
#endif


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

    struct Configuration
    {
        bool logmode = false;
        bool is_tty = false;
        std::size_t progress_bar_width = 40u;

#ifdef _MSC_VER
        bool print_to_vs_console = false;
#endif

    };



    Logger(std::ostream& console_stream) : _console_stream(console_stream)
    {
        //
        // initializes the logger with a default print
        // level of 0 and no string streams
        //

        if (&_console_stream == &std::cout) {
            _configuration.is_tty = isatty(1);
        }
    }


    void set_maximum_print_level_string_streams(int max_print_level)
    {
        _string_streams.resize(max_print_level + 1);
    }


    void set_console_print_level(std::size_t console_print_level)
    {
        _state.console_print_level = console_print_level;
    }


    Configuration& configuration() { return _configuration; }


    Logger& operator()(const std::size_t new_input_print_level)
    {
        _state.input_print_level = new_input_print_level;

        return *this;
    }


    void print_all() const
    {
        for (std::size_t i = 0; i < _string_streams.size(); ++i) {
            std::cout << "Print level i = " << i << std::endl;
            std::cout << _string_streams[i].str();
            std::cout << "------------------- " << std::endl;
        }
    }


    Logger& operator<<(const Color& c)
    {
        if (_configuration.is_tty && (_state.console_print_level >= _state.input_print_level))
        {
            _console_stream << "\033[" << c.get_code() << "m";
        }

        return *this;
    }


    template<class T>
    Logger& operator<<(const T& t)
    {
        if (_state.input_print_level <= _state.console_print_level || (_state.input_print_level < _string_streams.size())) {

            // we will print to somewhere.
            // if logmode, write the current date/time to a string
            const auto datestr = get_log_entry_text();

            if (_state.input_print_level <= _state.console_print_level) {

                // write to console

                // stop spinning/progress bars if they are running
                if (_configuration.is_tty) {
                    if (_state.spinning_bar_on) {
                        stop_spinning_bar();
                    }

                    if (_state.progress_bar_on) {
                        stop_progress_bar();
                    }
                }


#ifdef _MSC_VER
                if (_configuration.print_to_vs_console) {

                    // instead of writing to cout/cerr, we write to
                    // the Visual Studio Output
                    std::ostringstream s;
                    s << datestr << t;
                    const auto s_wstr = string_to_wstring(s.str());
                    OutputDebugStringW(s_wstr.c_str());

                }
                else {
#endif
                    _console_stream << datestr << t;
#ifdef _MSC_VER
                }
#endif

            }


            // write to string streams
            for (std::size_t i = _state.input_print_level; i < _string_streams.size(); ++i) {
                _string_streams[i] << datestr << t;
            }

        }
        return *this;
    }


    Logger& operator<<(std::ostream& (*f)(std::ostream&))
    {
        if (_state.input_print_level <= _state.console_print_level || (_state.input_print_level < _string_streams.size())) {

            // we will print to somewhere
            const auto datestr = get_log_entry_text();

            if (_state.input_print_level <= _state.console_print_level) {

                // print to console

                // stop spinning/progress bars if they are running
                if (_configuration.is_tty) {
                    if (_state.spinning_bar_on) {
                        stop_spinning_bar();
                    }

                    if (_state.progress_bar_on) {
                        stop_progress_bar();
                    }
                }

#ifdef _MSC_VER
                if (_configuration.print_to_vs_console) {

                    if (f == std::endl<char, std::char_traits<char>>) {
                        const auto wstr = string_to_wstring(datestr + "\n");
                        OutputDebugStringW(wstr.c_str());
                    }

                }
                else {
#endif
                    _console_stream << datestr;
                    (*f)(_console_stream);

#ifdef _MSC_VER
                }
#endif
            }

            for (std::size_t i = _state.input_print_level; i < _string_streams.size(); ++i) {
                _string_streams[i] << datestr;
                (*f)(_string_streams[i]);
            }

            _state.is_first_message = true;
        }

        return *this;
    }


    void spin_bar(const std::string& header)
    {
        if (_configuration.is_tty && (_state.console_print_level >= _state.input_print_level)) {
            _state.spinning_bar_on = true;
            _console_stream << "\33[2K\r";
            _console_stream << header << _spinning_bar_chars[_state.spinning_bar_counter];
            _state.spinning_bar_counter++;
            _state.spinning_bar_counter = (_state.spinning_bar_counter) % 4;
            _console_stream << std::flush;
        }
    }


    void stop_spinning_bar()
    {
        if (_configuration.is_tty && _state.spinning_bar_on) {

            _console_stream << "\33[2K\r";
            _state.spinning_bar_on = false;
        }
    }


    void progress_bar(const std::string& header, const int curr, const int tot)
    {
        if (_configuration.is_tty && (_state.console_print_level >= _state.input_print_level)) {

            if (_state.spinning_bar_on) {
                stop_spinning_bar();
            }

            _state.progress_bar_on = true;
            _console_stream << "\015";
            _console_stream << header;

            double fills = (static_cast<double>(curr) / static_cast<double>(tot) * _configuration.progress_bar_width);
            int ifills = (int)fills;

            for (int i = 0; i < ifills; i++) {
                _console_stream << _progress_bar_chars[8];
            }

            if (curr != tot) {
                _console_stream << _progress_bar_chars[static_cast<int>((8.0) * (fills - ifills))];
            }

            for (int i = 0; i < _configuration.progress_bar_width - ifills - 1; i++) {
                _console_stream << "▏";
            }

            _console_stream << "▏";

            _console_stream << " (" << curr << "/" << tot << ")";
            _console_stream << std::flush;
        }
    }


    void progress_bar(const std::string& header, double percentage)
    {
        if (_configuration.is_tty && (_state.console_print_level >= _state.input_print_level)) {

            percentage = std::min(double{ 1.0 }, percentage);

            if (_state.spinning_bar_on) stop_spinning_bar();

            _state.progress_bar_on = true;
            _console_stream << "\015";
            _console_stream << header;
            constexpr int tot = 1000;
            int curr = static_cast<int>(percentage * tot);

            double fills = (static_cast<double>(curr) / static_cast<double>(tot) * _configuration.progress_bar_width);
            int ifills = (int)fills;

            for (int i = 0; i < ifills; i++) {
                _console_stream << _progress_bar_chars[8];
            }

            if (curr != tot) {
                _console_stream << _progress_bar_chars[static_cast<int>((8.0) * (fills - ifills))];
            }

            for (int i = 0; i < _configuration.progress_bar_width - ifills - 1; i++) {
                _console_stream << "▏";
            }

            _console_stream << "▏";

            char buffer[10];
            sprintf(buffer, "%*.1f", 5, percentage * 100);

            _console_stream << " (" << buffer << "%" << ")";
            _console_stream << std::flush;
        }
    }


    void stop_progress_bar()
    {
        if (_configuration.is_tty && _state.progress_bar_on) {

            _console_stream << "\33[2K\r";
            _state.progress_bar_on = false;
        }
    }


    void clear()
    {
        for (auto& ss : _string_streams) {

            ss.str("");
            ss.clear();
        }


        _string_streams.clear();
        _string_streams.shrink_to_fit();
    }

    static Logger out;
    static Logger err;

private:

    std::string get_log_entry_text()
    {
        //
        // Prints current date/time in [YYYY-MM-DDTHH:mm:ss] format
        // In windows it also shows the current thread Id
        //

        if (_configuration.logmode && _state.is_first_message) {

            auto time = std::time(nullptr);
            std::ostringstream sout;

#ifdef _MSC_VER
            struct tm buf;
            gmtime_s(&buf, &time);
            char threadstr[10];
            sprintf_s(threadstr, "%05d", GetCurrentThreadId());
            sout << "[(" << threadstr << ")" << std::put_time(&buf, "%FT%T") << "]";
#else
            sout << "[" << std::put_time(std::gmtime(&time), "%FT%T%z") << "]";
#endif

            _state.is_first_message = false;
            return sout.str();
        }
        else {
            return {};
        }

    }

    std::ostream& _console_stream;
    std::vector<std::ostringstream> _string_streams;

    Configuration _configuration;

    struct
    {
        std::size_t console_print_level = 0u;
        std::size_t input_print_level = 0u;
        bool is_first_message = true;
        bool spinning_bar_on = false;
        int spinning_bar_counter = 0;
        bool progress_bar_on = false;
    } _state;


    // Spinning bar ---------------------------
    static constexpr std::array<char, 4u> _spinning_bar_chars = { '/','|','-','\\' };

    // Progress bar ----------------------------
    static constexpr std::array<const char*, 9u> _progress_bar_chars = { " ", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█" };

};

inline Logger Logger::out(std::cout);
inline Logger Logger::err(std::cerr);

inline Logger& out = Logger::out;
inline Logger& err = Logger::err;


} // end namespace loggercpp



#endif
