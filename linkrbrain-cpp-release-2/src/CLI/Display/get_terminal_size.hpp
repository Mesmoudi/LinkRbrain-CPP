#ifndef LINKRBRAIN2019__SRC__CLI__DISPLAY__GET_TERMINAL_SIZE_HPP
#define LINKRBRAIN2019__SRC__CLI__DISPLAY__GET_TERMINAL_SIZE_HPP


#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define VC_EXTRALEAN
    #include <Windows.h>
#elif defined(__linux__)
    #include <sys/ioctl.h>
#endif // operating system


namespace CLI::Display {

    static const int get_terminal_width() {
        #if defined(_WIN32)
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
            return csbi.dwSize.X;
        #elif defined(__linux__)
            struct winsize w;
            ioctl(fileno(stdout), TIOCGWINSZ, &w);
            return w.ws_col;
        #endif // operating system
    }

    static const int get_terminal_height() {
        #if defined(_WIN32)
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
            return csbi.dwSize.Y;
        #elif defined(__linux__)
            struct winsize w;
            ioctl(fileno(stdout), TIOCGWINSZ, &w);
            return w.ws_row;
        #endif // operating system
    }

}

#endif // LINKRBRAIN2019__SRC__CLI__DISPLAY__GET_TERMINAL_SIZE_HPP
