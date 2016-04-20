#ifndef COMMON_H__
#define COMMON_H__
#include <iostream>
#include <time.h>

//================================================================
// Printing
//================================================================

// http://en.wikipedia.org/wiki/ANSI_escape_code
//http://stackoverflow.com/questions/5947742/how-to-change-the-output-color-of-echo-in-linux
const std::string boldblacktty("\033[1;30m");   // tell tty to switch to bold black
const std::string lightredtty("\033[1;31m");   // tell tty to switch to bold red
const std::string lightgreentty("\033[1;32m");   // tell tty to switch to bright green
const std::string redtty("\033[0;31m");
const std::string greentty("\033[0;32m");
const std::string bluetty("\033[34m");   // tell tty to switch to blue
const std::string lightbluetty("\033[1;34m");
const std::string lightcyantty("\033[1;36m");
const std::string cyantty("\033[0;36m");
const std::string purpletty("\033[0;35m");
const std::string lightpurpletty("\033[1;35m");
const std::string orangetty("\033[0;33m");
const std::string yellowtty("\033[1;33m");
const std::string magentatty("\033[1;35m");   // tell tty to switch to bright magenta
const std::string yellowbgtty("\033[1;43m");   // tell tty to switch to bright yellow background
const std::string underlinetty("\033[4m");   // tell tty to switch to underline
const std::string deftty("\033[0m");      // tell tty to switch back to default color

timespec diff(timespec,timespec); //Difference between timespecs
long long int getMilliseconds(timespec); //Get Milliseconds out of a timespec
long long int getNanoseconds(timespec);


#endif //COMMON_H__
