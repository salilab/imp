#include <stdio.h>
#include "IMP/log.h"


std::ofstream imp_log;
char log_fname[256];


bool log_initialized = false;

// fail safe C routines
// seem to be more reliable than the C++ streams

/**
  Initialize the C-based log. Always writes to log2.txt.
 */
void InitLog2(void)
{
  FILE *fp;

  fp = fopen("log2.txt", "w");
  fputs ("Starting log ... \n", fp);
  fclose(fp);
}

/**
  Enter C string into the C-based log. Add a newline after the string.

  \param[in] str The C string to be entered into the log.
 */
void EnterMsg(char *str)
{
  FILE *fp;

  fp = fopen("log2.txt", "a");
  fputs(str, fp);
  fputs("\n", fp);
  fclose(fp);
}

// C++ stream-based log routines

/**
  Initialize the C++-based log. Writes to the given log file.

  \param[in] the_log_fname File name of the C++-based log.
*/
void InitLog(char *the_log_fname)
{
  if (!log_initialized) {
    log_initialized = true;

    strncpy(log_fname, the_log_fname, 255);
    imp_log.open(log_fname, std::ios_base::out);
    imp_log.close();

    // InitLog2();
  }
}


/**
  Open the C++-based log output stream for additional writing. Actual writing
  is done through macros defined in log.h.
 */
void AppendLog(void)
{
  imp_log.open(log_fname, std::ios_base::app);
}


/**
  Close the C++-based log output stream.
 */
void CloseLog(void)
{
  if (imp_log.is_open()) {
    imp_log.close();
  }
}
