/* The VMD plugin macros are written assuming that the plugin is built as
   a single compilation unit (on Windows, each #include of vmdplugin.h will
   define the DllMain function, so it can only be included once). Work around
   this by including all the .cpp files into this one.
 */
#include "Data.cpp"
#include "Data_read.cpp"
#include "init.cpp"


