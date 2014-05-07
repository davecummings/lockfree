#ifndef MONITOR
#define MONITOR

#include <iostream>
#include <vector>
#include <thread>

#if defined(__APPLE__)
#include "smc.h"
#endif
#include "cycle_timer.h"

class Monitor
{
public:
    void start();
    void start(bool monitorPower);
    void stop();
    bool canMonitorPower();
    double getElapsedSeconds();
    double getCPUPowerUsage();
    double getDDRPowerUsage();
    double getSSDPowerUsage();
    double getPowerUsage();
    bool isRunning();
    int getSampleGranularity();
    void setSampleGranularity(int millis);
    
    static Monitor* getInstance()
    {
        if (_self == NULL) {
            _self = new Monitor;
        }
        
        return _self;
    }
private:
    Monitor() {
        _granularity = DEFAULT_GRANULARITY;
        _running = false;
        _reading = false;
    };
    ~Monitor() {};
    static Monitor* _self;
    std::vector<double> _CPUSamples;
    std::vector<double> _DDRSamples;
    std::vector<double> _SSDSamples;
    std::vector<double> _TimeSamples;
    double _CPUPower;
    double _DDRPower;
    double _SSDPower;
    double _start;
    double _end;
    double _elapsed;
    pthread_t _thread;
    bool _running;
    bool _reading;
    int _granularity;
    const static int DEFAULT_GRANULARITY = 100;
    
    // returns power consumption in mWh (milliwatt-hours)
    double sumWattage(std::vector<double>& samples, std::vector<double>& timeSamples)
    {
        double wattage = 0.0;
        
        std::vector<double>::iterator time_it = timeSamples.begin();
        for (std::vector<double>::iterator it = samples.begin() ; it != samples.end(); ++it) {
            wattage += (*it) * (*time_it);
            ++time_it;
        }
        
        wattage /= 3.6;
        
        return wattage;
    }
    
    typedef struct monitor_loop_args {
        std::vector<double>* CPUSamples;
        std::vector<double>* DDRSamples;
        std::vector<double>* SSDSamples;
        std::vector<double>* TimeSamples;
        int granularity;
        bool* runningPtr;
    } MonitorLoopArgs;
    
    static void* monitor_loop(void* args)
    {
        double then = CycleTimer::currentSeconds();
        MonitorLoopArgs* loopArgs = (MonitorLoopArgs*)(args);
        
        struct timespec interval;
        interval.tv_sec = loopArgs->granularity / 1000;
        interval.tv_nsec = 1000000 * (loopArgs->granularity % 1000);
        
        SMCOpen();
        
        do {
            double now = CycleTimer::currentSeconds();
            loopArgs->TimeSamples->push_back(now-then);
            loopArgs->CPUSamples->push_back(SMCGetCPUPower());
            loopArgs->DDRSamples->push_back(SMCGetDDRPower());
            loopArgs->SSDSamples->push_back(SMCGetSSDPower());
            then = now;
            nanosleep(&interval, NULL);
        } while (*(loopArgs->runningPtr));
        
        SMCClose();
        
        free(args);
        
        return NULL;
    }
    
    Monitor(Monitor const&);        // Don't Implement
    void operator=(Monitor const&); // Don't implement
};

#endif
