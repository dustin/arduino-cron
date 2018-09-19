#include <time.h>

#include <Cron.h>

cron::CronTab crontab {
                       // This job will run every second.
                       new cron::Job("job1",
                                     []() { return Serial.println("Running job 1!"); },
                                     1),
                       // This job will run once a day at the time you configure.
                       new cron::Job("job2",
                                     []() { return Serial.println("Running job 2!"); },
                                     cron::DAILY),
};

void setup() {
    // You must have your time set correctly before proceeding.
    // 1.  Connect to the network.
    // 2.  Call configTime() properly (with the correct timezone info if you want to use HH:MM:SS)

    time_t now(time(NULL));
    crontab[0]->set(3, now);                // Start the first job in 3 seconds.
    crontab["job2"]->set("13:14:15", now);  // Schedule the second job in the afternoon.

    // If you want to make sure that second job runs only in the
    // future (e.g., in case it's already past 13:14:15), you can
    // reschedule it:
    crontab["job2"]->rescheduleAfter(now);
}

void loop() {
    crontab();  // run all the cron jobs
}
