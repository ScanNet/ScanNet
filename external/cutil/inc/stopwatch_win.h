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

#ifndef _STOPWATCH_WIN_H_
#define _STOPWATCH_WIN_H_

// includes, system
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max

//! Windows specific implementation of StopWatch
class StopWatchWin
{
protected:

    //! Constructor, default
    StopWatchWin();

    // Destructor
    ~StopWatchWin();

public:

    //! Start time measurement
    inline void start();

    //! Stop time measurement
    inline void stop();

    //! Reset time counters to zero
    inline void reset();

    //! Time in msec. after start. If the stop watch is still running (i.e. there
    //! was no call to stop()) then the elapsed time is returned, otherwise the
    //! time between the last start() and stop call is returned
    inline const float getTime() const;

    //! Mean time to date based on the number of times the stopwatch has been 
    //! _stopped_ (ie finished sessions) and the current total time
    inline const float getAverageTime() const;

private:

    // member variables

    //! Start of measurement
    LARGE_INTEGER  start_time;
    //! End of measurement
    LARGE_INTEGER  end_time;

    //! Time difference between the last start and stop
    float  diff_time;

    //! TOTAL time difference between starts and stops
    float  total_time;

    //! flag if the stop watch is running
    bool running;

    //! Number of times clock has been started
    //! and stopped to allow averaging
    int clock_sessions;

    //! tick frequency
    static double  freq;

    //! flag if the frequency has been set
    static  bool  freq_set;
};

// functions, inlined

////////////////////////////////////////////////////////////////////////////////
//! Start time measurement
////////////////////////////////////////////////////////////////////////////////
inline void
StopWatchWin::start() 
{
    QueryPerformanceCounter((LARGE_INTEGER*) &start_time);
    running = true;
}

////////////////////////////////////////////////////////////////////////////////
//! Stop time measurement and increment add to the current diff_time summation
//! variable. Also increment the number of times this clock has been run.
////////////////////////////////////////////////////////////////////////////////
inline void
StopWatchWin::stop() 
{
    QueryPerformanceCounter((LARGE_INTEGER*) &end_time);
    diff_time = (float) 
        (((double) end_time.QuadPart - (double) start_time.QuadPart) / freq);

    total_time += diff_time;
    clock_sessions++;
    running = false;
}

////////////////////////////////////////////////////////////////////////////////
//! Reset the timer to 0. Does not change the timer running state but does 
//! recapture this point in time as the current start time if it is running.
////////////////////////////////////////////////////////////////////////////////
inline void
StopWatchWin::reset() 
{
    diff_time = 0;
    total_time = 0;
    clock_sessions = 0;
    if( running )
        QueryPerformanceCounter((LARGE_INTEGER*) &start_time);
}


////////////////////////////////////////////////////////////////////////////////
//! Time in msec. after start. If the stop watch is still running (i.e. there
//! was no call to stop()) then the elapsed time is returned added to the 
//! current diff_time sum, otherwise the current summed time difference alone
//! is returned.
////////////////////////////////////////////////////////////////////////////////
inline const float 
StopWatchWin::getTime() const 
{
    // Return the TOTAL time to date
    float retval = total_time;
    if(running) 
    {
        LARGE_INTEGER temp;
        QueryPerformanceCounter((LARGE_INTEGER*) &temp);
        retval += (float) 
            (((double) (temp.QuadPart - start_time.QuadPart)) / freq);
    }

    return retval;
}

////////////////////////////////////////////////////////////////////////////////
//! Time in msec. for a single run based on the total number of COMPLETED runs
//! and the total time.
////////////////////////////////////////////////////////////////////////////////
inline const float 
StopWatchWin::getAverageTime() const
{
	return (clock_sessions > 0) ? (total_time/clock_sessions) : 0.0f;
}

#endif // _STOPWATCH_WIN_H_

