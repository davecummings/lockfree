//
//  monitor.cpp
//  LockFree Mac
//
//  Created by Dave Cummings on 5/6/14.
//  Copyright (c) 2014 Dave Cummings. All rights reserved.
//

#include <iostream>
#include <vector>

#include "monitor.h"

Monitor* Monitor::_self = NULL;

void Monitor::start(bool monitorPower)
{
#if !defined (__APPLE__)
    if (monitorPower) {
        std::cerr << "Power monitoring is not supported on this device." << std::endl;
    }
    monitorPower = false;
#endif
    _running = true;
    _reading = monitorPower;
    _start = CycleTimer::currentSeconds();
    
    MonitorLoopArgs* args = (MonitorLoopArgs*)malloc(sizeof(MonitorLoopArgs));
    args->runningPtr = &_running;
    args->granularity = _granularity;
    args->CPUSamples = &_CPUSamples;
    args->DDRSamples = &_DDRSamples;
    args->SSDSamples = &_SSDSamples;
    args->TimeSamples = &_TimeSamples;
    
    if (_reading) {
        pthread_create(&_thread, NULL, monitor_loop, (void*)args);
    }
}

void Monitor::start()
{
    start(false);
}

void Monitor::stop()
{
    _running = false;
    _end = CycleTimer::currentSeconds();
    
    if (_reading) {
        pthread_join(_thread, NULL);
    }
    
    _elapsed = _end - _start;
    
    _CPUPower = sumWattage(_CPUSamples, _TimeSamples);
    _CPUSamples.clear();
    _DDRPower = sumWattage(_DDRSamples, _TimeSamples);
    _DDRSamples.clear();
    _SSDPower = sumWattage(_SSDSamples, _TimeSamples);
    _SSDSamples.clear();
    _TimeSamples.clear();
    
}

bool Monitor::canMonitorPower()
{
#if defined (__APPLE__)
    return SMCGetCPUPower >= 0 && SMCGetDDRPower >= 0 && SMCGetSSDPower >= 0;
#endif
    return false;
    
}

double Monitor::getElapsedSeconds()
{
    if (_running) {
        return 0.0;
    }
    
    return _elapsed;
}

double Monitor::getCPUPowerUsage()
{
    if (_running) {
        return 0.0;
    }
    
    return _CPUPower;
}

double Monitor::getDDRPowerUsage()
{
    if (_running) {
        return 0.0;
    }
    
    return _DDRPower;
}

double Monitor::getSSDPowerUsage()
{
    if (_running) {
        return 0.0;
    }
    
    return _SSDPower;
}

double Monitor::getPowerUsage()
{
    return _CPUPower + _DDRPower + _SSDPower;
}

bool Monitor::isRunning()
{
    return _running;
}

int Monitor::getSampleGranularity()
{
    return _granularity;
}

void Monitor::setSampleGranularity(int millis)
{
    if (millis < 0) {
        millis = 0;
    }
    
    _granularity = millis;
}