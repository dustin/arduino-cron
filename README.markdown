# Cron

Cron allows you to perform some work periodically based on wall time.
Want to turn on a light every morning at `6:53`?  Now you can!

# Basic Usage

The cron system can't work quite correctly before time is set up, but
you can define your jobs.  See [the basic
example](examples/basic/basic.ino) for how to create some jobs and
ensure they run.

Note that this doesn't run the jobs "in the background."  i.e., you
get to decide when your jobs run and what they may interfere with.
For most of my cases, I'm interacting with MQTT services, so I want to
make sure I'm connected to the network and my MQTT server before
running jobs.  This isn't absolutely critical as a job can "fail" and
they will still be marked as ready and retried, but I like having
really clear interleaves.

# Time Management

This library does require that you're on the network and have your
time sync'd with `localtime` working correctly.   e.g., I do something
like this in US/Pacific:

```cpp
void setup() {
  setupWifi();
  setupTime();
}

ESP8266WiFiMulti wifiMulti; // or equivalent

void setupWifi() {
  wifiMulti.addAP("Some AP", "Some Password");
  wifiMulti.addAP("Alternate AP", "Alternate Password");

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(1000);
  }
}

void setupTime() {
  configTime(0 /* tz */, 0 /* dst */, "pool.ntp.org");
  while (time(nullptr) < 1535920965) {
    delay(10);
  }
  setenv("TZ", "PST8PDT7,M3.2.0/02:00:00,M11.1.0/02:00:00", 1);
  tzset();
}
```

At this point, your device should know the time and continue to know
the time.
