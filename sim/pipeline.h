#pragma once
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <random>
#include <bitset>
#include <cstdint>
#include "../sim/cpu.h"

// Forward declaration
extern int debug;
class pipelineSimulation;

class event {
  public:
      // Construct sets time of event.
    event (float t, std::string n, CpuSim* cI, pipelineSimulation* pS) : time (t), name(n), cpuInstance(cI), pipelineSim(pS) { }

      // Execute event by invoking this method.
    virtual void processEvent () = 0;
    CpuSim* cpuInstance;
    pipelineSimulation* pipelineSim;
    const uint32_t time;
    std::string name;
};

struct eventComparator {
  bool operator() (const event * left, const event * right) const {
    return left->time > right->time;
  }
};

class simulation {
  public:
    simulation () : clk (0), eventQueue () { }
    void run ();
    void scheduleEvent (event * newEvent) {
      eventQueue.push(newEvent);
    }
    uint32_t clk;
  protected:
    std::priority_queue<event*, std::vector<event *, std::allocator<event*> >, eventComparator> eventQueue;
};

class pipelineSimulation : public simulation {
    public:
        pipelineSimulation(CpuSim* cpu1, std::string simName) : simulation(), cpuInstance(cpu1), simName(simName){}
        bool notStalled();
        void start();
        void halt();
        void run ();
        void tick();
        void endCyclePrintOut();

        CpuSim * cpuInstance;
        std::string simName;
        int stallTime = 0;
        bool pipelineBusy = false;
        bool halted = false;
};

class fetchEvent : public event {
    public:
        fetchEvent(uint32_t t, CpuSim* cpuInstance, pipelineSimulation* pipelineSim) : event(t, "fetch", cpuInstance, pipelineSim){}
        void processEvent();
};

class decodeEvent : public event {
    public:
        decodeEvent(uint32_t t, CpuSim* cpuInstance, pipelineSimulation* pipelineSim) : event(t, "decode", cpuInstance, pipelineSim){}
        void processEvent();
};

class executeEvent : public event {
    public:
        executeEvent(uint32_t t, CpuSim* cpuInstance, pipelineSimulation* pipelineSim) : event(t, "execute", cpuInstance, pipelineSim){}
        void processEvent();
};

class storeEvent : public event {
    public:
        storeEvent(uint32_t t, CpuSim* cpuInstance, pipelineSimulation* pipelineSim) : event(t, "store", cpuInstance, pipelineSim){}
        void processEvent();
};

class hazardEvent : public event {
    public:
        hazardEvent(uint32_t t, CpuSim* cpuInstance, pipelineSimulation* pipelineSim) : event(t, "hazard", cpuInstance, pipelineSim) {}
        void processEvent();
};