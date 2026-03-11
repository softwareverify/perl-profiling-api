/*    profilingapi.h
 *
 *    Copyright (C) 2011 by Stephen Kellett, Software Verification Ltd, (www.softwareverify.com)
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* This file contains definitions for the profiling API
   to enable third party tools (open source and commercial) to easily 
   instrument a Perl application.
 */

#ifndef _PROFILING_API_H
#define _PROFILING_API_H

#include "perl.h"

typedef enum _perlProfilerEvent
{
	PPE_NONE,						// no profiler event
	PPE_IGNORE,						// used internally, used to signify that should be ignored
	PPE_LINE,						// line visit event
	PPE_CALL,						// function call event
	PPE_RETURN,						// function return event
} PERL_PROFILER_EVENT;

typedef void (*PERL_PROFILER_CALLBACK)(PERL_PROFILER_EVENT	event,
									   const char			*fileName,
									   const int			lineNumber,
									   const char			*functionName,
									   const char			*packageName,
									   void					*userData);

// functions for use by external users of perl.exe (that want to call functions in perlNNN.dll)

EXT
void Perl_set_do_profiling(int	enable);

EXT
int Perl_get_do_profiling();

EXT
void Perl_set_coverage_callback(PERL_PROFILER_CALLBACK	callback,
								void					*userData);

EXT
void Perl_set_profiling_callback(PERL_PROFILER_CALLBACK	callback,
								 void					*userData);

// private function for use by Perl Interpreter

int doProfilingCallback(struct	op		*currentOP, 
						PerlInterpreter	*my_perl);

#endif	// _PROFILING_API_H
