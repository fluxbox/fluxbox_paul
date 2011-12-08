/** TickTracker.cc file for the fluxbox compositor. */

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


#include "TickTracker.hh"

#include "Logging.hh"

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

// The accuracy of the timer (1.0 = 1 second).
const double EPSILON = 1e-6;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
TickTracker::TickTracker() {
    m_is_running = false;
    m_tick_size = 1000000;
    m_ticks_per_second = 1.0;
}

// Copy constructor.
TickTracker::TickTracker(const TickTracker &other) :
    m_is_running(other.m_is_running),
    m_startTime(other.m_startTime),
    m_tick_size(other.m_tick_size),
    m_ticks_per_second(other.m_ticks_per_second),
    m_observed_ticks(other.m_observed_ticks) {
}

// Assignment operator.
TickTracker &TickTracker::operator=(const TickTracker &other) {
    if (this != &other) {
        m_is_running = other.m_is_running;
        m_startTime = other.m_startTime;
        m_tick_size = other.m_tick_size;
        m_ticks_per_second = other.m_ticks_per_second;
        m_observed_ticks = other.m_observed_ticks;
    }
    return *this;
}

// Destructor.
TickTracker::~TickTracker() { }


//--- TIMER MANIPULATION -------------------------------------------------------

// Starts the timer.
void TickTracker::start() {
    if (gettimeofday(&m_startTime, NULL)) {
        throw TimeException("Cannot obtain the current time.");
    }
    m_observed_ticks = 0;
    m_is_running = true;
}

/** Stops the timer. */
void TickTracker::stop() {
    m_is_running = false;
}


//--- TIMER QUERIES ------------------------------------------------------------

// Returns the new number of elapsed ticks since last call of this function.
int TickTracker::newElapsedTicks() {
    int total_ticks = totalElapsedTicks();
    int new_ticks = total_ticks - m_observed_ticks;
    m_observed_ticks = total_ticks;

    if (new_ticks < 0) {
        return 0;
    } else {
        return new_ticks;
    }
}

// Returns the total number of elapsed ticks.
int TickTracker::totalElapsedTicks() {
    static timeval now;

    if (gettimeofday(&now, NULL)) {
        throw TimeException("Cannot obtain the current time.");
    }

    return tickDifference(now, m_startTime);
}


// Sets the size of a tick.
void TickTracker::setTickSize(int usec) {
    if (usec < 1) {
        throw TimeException("Invalid tick size.");
    }

    m_tick_size = usec;
    m_ticks_per_second = 1000000.0 / usec;
}


//--- INTERNAL FUNTIONS --------------------------------------------------------

// Returns the difference in time between two timevals.
// Function adapted from http://www.gnu.org/s/libc/manual/html_node/Elapsed-Time.html.
timeval TickTracker::timeDifference(timeval t1, timeval t2) {
    int n_sec;

    if (t1.tv_usec < t2.tv_usec) {
        n_sec = (t2.tv_usec - t1.tv_usec) / 1000000 + 1;
        t2.tv_usec -= 1000000 * n_sec;
        t2.tv_sec += n_sec;
    }
    if (t1.tv_usec - t2.tv_usec > 1000000) {
        n_sec = (t1.tv_usec - t2.tv_usec) / 1000000;
        t2.tv_usec += 1000000 * n_sec;
        t2.tv_sec -= n_sec;
    }

    timeval diff;
    diff.tv_sec = t1.tv_sec - t2.tv_sec;
    diff.tv_usec = t1.tv_usec - t2.tv_usec;
    return diff;
}

// Returns the difference between two timevals in ticks.
int TickTracker::tickDifference(const timeval &t1, const timeval &t2) {
    timeval diff = timeDifference(t1, t2);

    double raw_diff = diff.tv_sec * m_ticks_per_second + (double(diff.tv_usec) / m_tick_size);
    return int(raw_diff + EPSILON);
}
