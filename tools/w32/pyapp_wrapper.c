/**
 *  \file pyapp_wrapper.c     \brief Windows Python application wrapper.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

/*
   Python applications on Unix/Linux/Mac are typically named without a .py
   extension (e.g. 'myapp' not 'myapp.py') and rely on a #!/usr/bin/python
   line at the start of the file to tell the OS that they are Python files.
   This doesn't work on Windows though, since Windows relies on the file
   extension. So on Windows we have to call this app 'myapp.py'.

   For the sake of consistency users on Windows would expect to run the app
   by typing 'myapp'. This can work if they set the PATHEXT environment variable
   to include .py. However, to make things work on an unmodified Windows box,
   we provide the wrapper below. Compile it with
      cl pyapp_wrapper.c shell32.lib
   and then copy the resulting pyapp_wrapper.exe to myapp.exe. Then when
   the user runs 'myapp' it runs 'myapp.exe' which should transparently
   redirect to 'myapp.py'.
 */

#include <stdio.h>
#include <windows.h>

/* Find where the parameters start in the command line (skip past the
   executable name) */
static char *find_param_start(char *cmdline)
{
  BOOL in_quote = FALSE, in_space = FALSE;
  for (; *cmdline; cmdline++) {
    /* Ignore spaces that are quoted */
    if (*cmdline == ' ' && !in_quote) {
      in_space = TRUE;
    } else if (*cmdline == '"') {
      in_quote = !in_quote;
    }
    /* Return the first nonspace that follows a space */
    if (in_space && *cmdline != ' ') {
      break;
    }
  }
  return cmdline;
}

/* Given the name of this executable (myapp.exe) return the corresponding
   Python application (myapp.py) */
static char *get_script_name(char *exename)
{
  int l;
  char *s;
  l = strlen(exename);
  if (l > 4 && exename[l - 4] == '.') {
    /* Replace .exe extension with .py */
    s = strdup(exename);
    l -= 4;
  } else {
    /* Add .py extension if the user ran us as "myapp" */
    s = malloc(l + 4);
    strcpy(s, exename);
  }
  s[l] = '.';
  s[l + 1] = 'p';
  s[l + 2] = 'y';
  s[l + 3] = '\0';
  return s;
}

int main(int argc, char *argv[]) {
  SHELLEXECUTEINFO si;
  BOOL bResult;
  char *param, *pyscript;
  pyscript = get_script_name(argv[0]);
  param = strdup(GetCommandLine());

  ZeroMemory(&si, sizeof(SHELLEXECUTEINFO));
  si.cbSize = sizeof(SHELLEXECUTEINFO);
  /* Wait for the spawned process to finish, so that any output goes to the
     console *before* the next command prompt */
  si.fMask = SEE_MASK_NO_CONSOLE | SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS;
  si.lpFile = pyscript;
  si.lpParameters = find_param_start(param);
  si.nShow = SW_SHOWNA;
  bResult = ShellExecuteEx(&si);
  free(param);
  free(pyscript);
   
  if (bResult) {
    if (si.hProcess) {
      WaitForSingleObject(si.hProcess, INFINITE);
      CloseHandle(si.hProcess);
    }
    return 0;
  } else {
    fprintf(stderr, "Failed to start process, code %d\n", GetLastError());
    return 1;
  }
}
