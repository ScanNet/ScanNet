/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
 
/* CUda UTility Library */

#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

// stop watch base class
#include <stopwatch_base.h>

// include OS specific policy
#ifdef _WIN32
# include <stopwatch_win.h>
typedef StopWatchWin  OSStopWatch;
#else
# include <stopwatch_linux.h>
typedef StopWatchLinux  OSStopWatch;
#endif

// concrete stop watch type
typedef StopWatchBase<OSStopWatch>  StopWatchC;

namespace StopWatch 
{
//! Create a stop watch
const unsigned int create();

//! Get a handle to the stop watch with the name \a name
StopWatchC& get( const unsigned int& name);

// Delete the stop watch with the name \a name
void destroy( const unsigned int& name);
} // end namespace, stopwatch

#endif // _STOPWATCH_H_

