#ifndef CRON_H
#define CRON_H 1

#include <Arduino.h>

#include <functional>
#include <time.h>

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

namespace cron {

static const long DAILY = 86400;
static const long HOURLY = 3600;

// A Job runs at a specific time.  If configured with a period, it
// will reschedule itself to run again in a future.
class Job {
 public:
 Job(const char* n, std::function<bool()> f, long period = 0, time_t initial = 0) :
    name(n), next(initial), cb(f), incr(period) {}

    // Return the number of seconds left before the next job runs.
    long timeLeft(time_t now) {
        return (long)difftime(next, now);
    }

    // Time of the next run.
    time_t nextRun() {
        return next;
    }

    // Set the time to an absolute time_t value.  If the timestamp is
    // < 30 days, it's considered relative and just sets a relative
    // timestamp to now.
    bool set(time_t to, time_t now) {
        if (to < 86400 * 30) {
            next = now + to;
        } else {
            next = to;
        }
        return true;
    }

    // Parse HH:MM:SS to the that hour, minute, and second today.  If
    // that time is in the past, (e.g., it's noon and you're setting
    // to 07:00:00), then the job will be ready to go.  If you instead
    // want it to be scheduled for tomorrow, you can call
    // rescheduleAfter(now) on this job after set() returns true.
    virtual bool set(String hms, time_t now) {
        if (hms.length() != 8 || hms[2] != ':' || hms[5] != ':') {
            return false;
        }

        char* p = (char*)hms.c_str();
        int h = strtol(p, &p, 10);
        if (h < 0 || h > 23) {
            return false;
        }
        int m = strtol(++p, &p, 10);
        if (m < 0 || m > 59) {
            return false;
        }
        // The input isn't null terminated and I don't care about seconds.
        int s = strtol(++p, &p, 10);

        struct tm t;
        localtime_r(&now, &t);
        t.tm_hour = h;
        t.tm_min = m;
        t.tm_sec = s;

        next = mktime(&t);
        DEBUG_MSG("Scheduled cron job %s for %d\n", name, next);
        return true;
    }

    // Cancel disables future executions of this job.  Use set() to reschedule.
    void cancel() {
        next = 0;
    }

    // Schedule the next run after the given time.  If this job has a
    // 0 increment (i.e., it's a oneshot job), it cancels itself.
    void rescheduleAfter(time_t now) {
        if (next <= now && incr == 0) {
            cancel();
            return;
        }

        while (next <= now) {
            next += incr;
        }
    }

    // Run jobs as of "now".  Return true if the job ran successfully.
    bool run(time_t now) {
        if (next == 0) {
            return false;
        }

        if (difftime(next, now) <= 0) {
            if (cb()) {
                rescheduleAfter(now);
                return true;
            }
        }

        return false;
    }

    inline bool operator()(time_t now) { return run(now); }

    long period() { return incr; }

    // This job's name.
    const char* name;

 protected:

    time_t next;
    std::function<bool()> cb;
    long incr;
};

// CronTab contains a bunch of periodic jobs and allows them to be run
// periodically.
class CronTab {
 public:

 CronTab(std::initializer_list<Job*> j) : jobs(j) {}

    void add(Job* j) { jobs.push_back(j); }

    int operator()(time_t now) {
        // If time has not been set yet, do not run jobs.  And if time
        // is set after cron jobs are initially scheduled, they're
        // probably wrong.
        if (now < 1537292757) {
            DEBUG_MSG("time is not synchronized, so cron jobs will not run");
            return 0;
        }

        int rv(0);
        for (auto j : jobs) {
            if (j->run(now)) {
                rv++;
            }
        }
        return rv;
    }

    Job* operator[](int n) {
        return jobs[n];
    }

    Job* operator[](const char* name) {
        for (auto j : jobs) {
            if (strcmp(j->name, name) == 0) {
                return j;
            }
        }
        return nullptr;
    }

 private:
    std::vector<Job*> jobs;
};

};  // namespace cron

#endif  // CRON_H
