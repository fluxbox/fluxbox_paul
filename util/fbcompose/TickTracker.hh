/** TickTracker.hh file for the fluxbox compositor. */

// Copyright (c) 2011 Gediminas Liktaras (gliktaras at gmail dot com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#ifndef FBCOMPOSITOR_TIMER_HH
#define FBCOMPOSITOR_TIMER_HH

#include "config.h"

#include "Exceptions.hh"

#include <sys/time.h>


namespace FbCompositor {

    class TimeException;
    class TickTracker;


    /**
     * A timer class.
     *
     * This class provides a simpler and more flexible interface to deal with
     * time. It was added, since it is not possible to make continuous time
     * measurements with FbTk::FbTickTracker.
     */
    class TickTracker {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        TickTracker();

        /** Copy constructor. */
        TickTracker(const TickTracker &other);

        /** Assignment operator. */
        TickTracker &operator=(const TickTracker &other);

        /** Destructor. */
        ~TickTracker();


        //--- TIMER MANIPULATION -----------------------------------------------

        /** Starts the timer. */
        void start();

        /** Stops the timer. */
        void stop();


        //--- TIMER QUERIES ----------------------------------------------------

        /** \returns whether the timer is running. */
        bool isRunning() const;

        /** \returns the tick size. */
        int tickSize() const;


        /** \returns the new number of elapsed ticks since last call of this function. */
        int newElapsedTicks();

        /** \returns the total number of elapsed ticks. */
        int totalElapsedTicks();


        /** Sets the size of a tick. */
        void setTickSize(int usec);


    private:
        //--- INTERNAL FUNTIONS ------------------------------------------------

        /** \returns the difference in time between two timevals. */
        timeval timeDifference(timeval t1, timeval t2);

        /** \returns the difference between two timevals in ticks. */
        int tickDifference(const timeval &t1, const timeval &t2);


        //--- PRIVATE VARIABLES ------------------------------------------------

        /** Whether the timer is running or not. */
        bool m_is_running;

        /** Time the timer was started. */
        timeval m_startTime;


        /** Size of the timer ticks. */
        int m_tick_size;

        /** The number of ticks per second. */
        double m_ticks_per_second;


        /** The number of observed ticks with newElapsedTicks(). */
        int m_observed_ticks;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns whether the timer is running.
    inline bool TickTracker::isRunning() const {
        return m_is_running;
    }

    // Returns the tick size.
    inline int TickTracker::tickSize() const {
        return m_tick_size;
    }
}


#endif  // FBCOMPOSITOR_TIMER_HH
