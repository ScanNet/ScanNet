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

#ifndef _STOPWATCH_BASE_H_
#define _STOPWATCH_BASE_H_

// includes, system
#include <vector>

//! Simple stop watch
template<class OSPolicy>
class StopWatchBase : public OSPolicy 
{
public:

    // generic, specialized type
    typedef StopWatchBase<OSPolicy>   SelfType;
    // generic, specialized type pointer
    typedef StopWatchBase<OSPolicy>*  SelfTypePtr;

    //! global index for all stop watches
    static  std::vector< SelfTypePtr > swatches;

public:

    //! Constructor, default
    StopWatchBase();

    // Destructor
    ~StopWatchBase();

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

    //! Constructor, copy (not implemented)
    StopWatchBase( const StopWatchBase&);

    //! Assignment operator (not implemented)
    StopWatchBase& operator=( const StopWatchBase&);
};

// include, implementation
#include <stopwatch_base.inl>


#endif // _STOPWATCH_BASE_H_

