/*
 * Copyright 1993-2011 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
 
/* Stopwatch Timing Functions (this is the inlined version compared to the CUTIL version) */

#ifndef STOPWATCH_FUNCTIONS_H
#define STOPWATCH_FUNCTIONS_H

// includes, system
#include <vector>

// includes, project
#include <exception.h>

// Definition of the StopWatch Interface, this is used if we don't want to use the CUT functions
// But rather in a self contained class interface
class StopWatchInterface
{
public:
	StopWatchInterface() {};
	virtual	~StopWatchInterface() {};

public:
    //! Start time measurement
    virtual void start() = 0;

    //! Stop time measurement
    virtual void stop() = 0;

    //! Reset time counters to zero
    virtual void reset() = 0;

    //! Time in msec. after start. If the stop watch is still running (i.e. there
    //! was no call to stop()) then the elapsed time is returned, otherwise the
    //! time between the last start() and stop call is returned
    virtual float getTime() = 0;

    //! Mean time to date based on the number of times the stopwatch has been 
    //! _stopped_ (ie finished sessions) and the current total time
    virtual float getAverageTime() = 0;
};


//////////////////////////////////////////////////////////////////
// Begin Stopwatch timer class definitions for all OS platforms //
//////////////////////////////////////////////////////////////////
#ifdef _WIN32
    // includes, system
    #define WINDOWS_LEAN_AND_MEAN
    #include <windows.h>
    #undef min
    #undef max

    //! Windows specific implementation of StopWatch
	class StopWatchWin : public StopWatchInterface
    {
    public:
        //! Constructor, default
        StopWatchWin() :
            start_time(),   end_time(),
            diff_time(0.0f), total_time(0.0f),
            running( false ), clock_sessions(0), freq_set(false)
        {
            if( ! freq_set) {
                // helper variable
                LARGE_INTEGER temp;

                // get the tick frequency from the OS
                QueryPerformanceFrequency((LARGE_INTEGER*) &temp);

                // convert to type in which it is needed
                freq = ((double) temp.QuadPart) / 1000.0;

                // rememeber query
                freq_set = true;
            }
        };

        // Destructor
        ~StopWatchWin() { };

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
        inline float getTime();

        //! Mean time to date based on the number of times the stopwatch has been 
        //! _stopped_ (ie finished sessions) and the current total time
        inline float getAverageTime();

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
        double  freq;

        //! flag if the frequency has been set
        bool  freq_set;
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
    inline float 
    StopWatchWin::getTime()
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
    inline float 
    StopWatchWin::getAverageTime()
    {
	    return (clock_sessions > 0) ? (total_time/clock_sessions) : 0.0f;
    }
#else
    // Declarations for Stopwatch on Linux and Mac OSX
    // includes, system
    #include <ctime>
    #include <sys/time.h>

    //! Windows specific implementation of StopWatch
	class StopWatchLinux : public StopWatchInterface
    {
    public:
        //! Constructor, default
        StopWatchLinux() : 
          start_time(), diff_time( 0.0), total_time(0.0),
          running( false ), clock_sessions(0)
        { };

        // Destructor
        virtual ~StopWatchLinux()
        { };

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
        inline float getTime();

        //! Mean time to date based on the number of times the stopwatch has been 
        //! _stopped_ (ie finished sessions) and the current total time
        inline float getAverageTime();

    private:

        // helper functions
      
        //! Get difference between start time and current time
        inline float getDiffTime();

    private:

        // member variables

        //! Start of measurement
        struct timeval  start_time;

        //! Time difference between the last start and stop
        float  diff_time;

        //! TOTAL time difference between starts and stops
        float  total_time;

        //! flag if the stop watch is running
        bool running;

        //! Number of times clock has been started
        //! and stopped to allow averaging
        int clock_sessions;
    };

    // functions, inlined

    ////////////////////////////////////////////////////////////////////////////////
    //! Start time measurement
    ////////////////////////////////////////////////////////////////////////////////
    inline void
    StopWatchLinux::start() {
      gettimeofday( &start_time, 0);
      running = true;
    }

    ////////////////////////////////////////////////////////////////////////////////
    //! Stop time measurement and increment add to the current diff_time summation
    //! variable. Also increment the number of times this clock has been run.
    ////////////////////////////////////////////////////////////////////////////////
    inline void
    StopWatchLinux::stop() {
      diff_time = getDiffTime();
      total_time += diff_time;
      running = false;
      clock_sessions++;
    }

    ////////////////////////////////////////////////////////////////////////////////
    //! Reset the timer to 0. Does not change the timer running state but does 
    //! recapture this point in time as the current start time if it is running.
    ////////////////////////////////////////////////////////////////////////////////
    inline void
    StopWatchLinux::reset() 
    {
      diff_time = 0;
      total_time = 0;
      clock_sessions = 0;
      if( running )
        gettimeofday( &start_time, 0);
    }

    ////////////////////////////////////////////////////////////////////////////////
    //! Time in msec. after start. If the stop watch is still running (i.e. there
    //! was no call to stop()) then the elapsed time is returned added to the 
    //! current diff_time sum, otherwise the current summed time difference alone
    //! is returned.
    ////////////////////////////////////////////////////////////////////////////////
    inline float 
    StopWatchLinux::getTime()
    {
        // Return the TOTAL time to date
        float retval = total_time;
        if( running) {
            retval += getDiffTime();
        }

        return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////
    //! Time in msec. for a single run based on the total number of COMPLETED runs
    //! and the total time.
    ////////////////////////////////////////////////////////////////////////////////
    inline float 
    StopWatchLinux::getAverageTime()
    {
        return (clock_sessions > 0) ? (total_time/clock_sessions) : 0.0f;
    }
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    inline float
    StopWatchLinux::getDiffTime()
    {
      struct timeval t_time;
      gettimeofday( &t_time, 0);

      // time difference in milli-seconds
      return  (float) (1000.0 * ( t_time.tv_sec - start_time.tv_sec) 
                    + (0.001 * (t_time.tv_usec - start_time.tv_usec)) );
    }
#endif // _WIN32

#endif
