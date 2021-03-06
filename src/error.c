/**
 * error.c -- error handling
 * Copyright (C) 2009-2014 by Gerardo Orellana <goaccess@prosoftcorp.com>
 * GoAccess - An Ncurses apache weblog analyzer & interactive viewer
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * A copy of the GNU General Public License is attached to this
 * source distribution for its full text.
 *
 * Visit http://goaccess.prosoftcorp.com for new releases.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#if defined(__GLIBC__)
#include <execinfo.h>
#endif
#include <sys/types.h>
#include <unistd.h>

#include "error.h"
#include "parser.h"

static FILE *log_file;
static GLog *log_data;
static FILE *log_invalid;

/* Open a debug file whose name is specified in the given path. */
void
dbg_log_open (const char *path)
{
  if (path != NULL) {
    log_file = fopen (path, "w");
    if (log_file == NULL)
      return;
  }
}

/* Close the debug file. */
void
dbg_log_close (void)
{
  if (log_file != NULL)
    fclose (log_file);
}

/* Open the invalid requests log file whose name is specified in the
 * given path. */
void
invalid_log_open (const char *path)
{
  if (path != NULL) {
    log_invalid = fopen (path, "w");
    if (log_invalid == NULL)
      return;
  }
}

/* Close the invalid requests log file. */
void
invalid_log_close (void)
{
  if (log_invalid != NULL)
    fclose (log_invalid);
}

/* Set current overall parsed log data. */
void
set_signal_data (void *p)
{
  log_data = p;
}

#if defined(__GLIBC__)
/* Dump to the standard output the values of the overall parsed log
 * data. */
static void
dump_struct (FILE * fp)
{
  int pid = getpid ();
  if (!log_data)
    return;

  fprintf (fp, "==%d== VALUES AT CRASH POINT\n", pid);
  fprintf (fp, "==%d==\n", pid);
  fprintf (fp, "==%d== Line number: %u\n", pid, log_data->processed);
  fprintf (fp, "==%d== Offset: %u\n", pid, log_data->offset);
  fprintf (fp, "==%d== Invalid data: %u\n", pid, log_data->invalid);
  fprintf (fp, "==%d== Piping: %d\n", pid, log_data->piping);
  fprintf (fp, "==%d== Response size: %llu bytes\n", pid, log_data->resp_size);
  fprintf (fp, "==%d==\n", pid);
}

/* Custom SIGSEGV handler. */
void
sigsegv_handler (int sig)
{
  char **messages;
  FILE *fp = stderr;
  int pid = getpid ();
  size_t size, i;
  void *trace_stack[TRACE_SIZE];

  (void) endwin ();
  fprintf (fp, "\n==%d== GoAccess %s crashed by Signal %d\n", pid, GO_VERSION,
           sig);
  fprintf (fp, "==%d==\n", pid);

  dump_struct (fp);

  size = backtrace (trace_stack, TRACE_SIZE);
  messages = backtrace_symbols (trace_stack, size);

  fprintf (fp, "==%d== STACK TRACE:\n", pid);
  fprintf (fp, "==%d==\n", pid);

  for (i = 0; i < size; i++)
    fprintf (fp, "==%d== %zu %s\n", pid, i, messages[i]);

  fprintf (fp, "==%d==\n", pid);
  fprintf (fp, "==%d== Please report it by opening an issue on GitHub:\n", pid);
  fprintf (fp, "==%d== https://github.com/allinurl/goaccess/issues\n\n", pid);
  exit (EXIT_FAILURE);
}
#endif

#pragma GCC diagnostic ignored "-Wformat-nonliteral"
/* Write formatted debug log data to the logfile. */
void
dbg_fprintf (const char *fmt, ...)
{
  va_list args;

  if (!log_file)
    return;

  va_start (args, fmt);
  vfprintf (log_file, fmt, args);
  fflush (log_file);
  va_end (args);
}

/* Write formatted invalid requests log data to the logfile. */
void
invalid_fprintf (const char *fmt, ...)
{
  va_list args;

  if (!log_invalid)
    return;

  va_start (args, fmt);
  vfprintf (log_invalid, fmt, args);
  fflush (log_invalid);
  va_end (args);
}

#pragma GCC diagnostic warning "-Wformat-nonliteral"
