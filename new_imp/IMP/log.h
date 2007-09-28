#include <iostream>
#include <fstream>

#define VERBOSE 0
#define TERSE 1

#define LOG_LEVEL VERBOSE

// The log file stream
extern std::ofstream imp_log;

// Functions for opening and closing the log file stream
void InitLog(char* log_fname);
void AppendLog(void);
void CloseLog(void);
void EnterMsg(char *str);

// Macros for writing to the log file:

// LogMsg uses log level to decide what to show.
// If given level is below current LOG_LEVEL, then nothing is written.
// For right now, "nothing" is sending an empty string to the standard
// error stream.
#define LogMsg(log_level, stream) AppendLog(); log_level >= LOG_LEVEL ? imp_log << stream << std::endl : std::cerr << ""; CloseLog();

// Warnings are always shown
#define WarningMsg(stream) AppendLog(); imp_log << "** WARNING **  " << stream << std::endl; CloseLog();

// Errors are always shown, and then the program is quit
#define ErrorMsg(stream) AppendLog(); imp_log << "** ERROR **  " << stream << std::endl; std::cout << "** ERROR **  " << stream << std::endl; CloseLog(); exit(0);
