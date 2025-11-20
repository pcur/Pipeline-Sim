#include "../sim/pipeline.h"
#include "../sim/helpers.h"

bool pipelineSimulation::notStalled(){
    printDebug("Checking if pipeline is stalled: " + std::to_string(pipelineBusy), 2);
    if(pipelineBusy) return false;
    else return true;
}

void pipelineSimulation::halt(){
    printDebug("Halting pipeline simulation", 1);
    while(!eventQueue.empty()){
        event * nextEvent = eventQueue.top();
        eventQueue.pop();
        delete nextEvent;
    }
    printDebug("All events removed from queue.", 1);
    halted = true;
}

void pipelineSimulation::debugPrintout(){
    // Debug output for each clock cycle
        if(debug == 1){
            std::stringstream ss;
            ss << simName << " Clock Cycle: " << clk << " Fetch: " << state.fetchState << " Decode: " << state.decodeState << " Execute: " << state.executeState << " Store: " << state.storeState;
            printDebug(ss.str(), 0);
        }
        if(debug == 2){
            std::priority_queue<event*, std::vector<event *, std::allocator<event*> >, eventComparator> queueCopy = eventQueue;
            while(!queueCopy.empty()){
                event* ev = queueCopy.top();
                printDebug("Event: " + ev->name + ", time: " + std::to_string(ev->time), 0);
                queueCopy.pop();
            }
        }
}
//start new simulation with externally-driven clock
void pipelineSimulation::start(){
    printDebug("Starting pipeline simulation runtime loop", 2);
    scheduleEvent(new fetchEvent(clk + 9, cpuInstance, this));
}

void pipelineSimulation::tick(){
    std::stringstream ss;
    ss << "\n========={ Simulation Tick: " << clk << " for cpu: " << simName << " }=========";
    printDebug(ss.str(), 1);
    while(eventQueue.top()->time <= clk + 10) {
            printDebug("Processing event scheduled for time: " + std::to_string(eventQueue.top()->time), 4);
            // Process all events scheduled for this clock cycle in order of time
            event * nextEvent = eventQueue.top();
            nextEvent->processEvent();
            if(halted) break;
            //may need stall logic here
            eventQueue.pop();
            delete nextEvent;
    }
    if (clk % 10 == 0){
    debugPrintout();
    }
    if(halted){
        state.fetchState = "HALTED";
        state.decodeState = "HALTED";
        state.executeState = "HALTED";
        state.storeState = "HALTED";
        printDebug("Halting simulation", 1);
        std::stringstream ss;
        ss << "Simulation Tick: " << clk << " Fetch: " << state.fetchState << " Decode: " << state.decodeState;
        printDebug(ss.str(), 0);
        halt();
    }
    else{
        if(stallTime > 0) {
            pipelineBusy = 1;
            printDebug("Pipeline is stalled for " + std::to_string(stallTime) + " more cycles", 1);
        }
        if(notStalled()){
            if (clk % 10 == 9){ // only schedule fence once per cpu cycle{}
            scheduleEvent(new fetchEvent(clk + 9, cpuInstance, this));
            printDebug("Pipeline is not stalled, scheduling new fetch event", 1);
        }
        }
        else { 
            printDebug("Pipeline is stalled, not scheduling new fetch event", 1);
            stallTime--;
            state.executeState = "STALL";
            //scheduleEvent(new executeEvent(clk + 1.1));
            if(stallTime <= 0) pipelineBusy = 0;
        }
        clk++; //increment internal clock
    }
}

void pipelineSimulation::run(){
    start();
    while(!eventQueue.empty()){ //clock cycle loop
        tick();
    }
}

void fetchEvent::processEvent(){
    printDebug("Processing fetch event at time: " + std::to_string(pipelineSim->clk), 3);
    cpuInstance->fetch();
    pipelineSim->scheduleEvent(new decodeEvent(pipelineSim->clk + 17 + cpuInstance->stallTime, cpuInstance, pipelineSim));
}

void decodeEvent::processEvent(){
    printDebug("Processing decode event at time: " + std::to_string(pipelineSim->clk), 3);
    cpuInstance->decode();
    pipelineSim->scheduleEvent(new executeEvent(pipelineSim->clk + 15 + cpuInstance->stallTime, cpuInstance, pipelineSim));
}

void executeEvent::processEvent(){
    printDebug("Processing execute event at time: " + std::to_string(pipelineSim->clk), 3);
    cpuInstance->execute();
    pipelineSim->scheduleEvent(new storeEvent(pipelineSim->clk + 13 + cpuInstance->stallTime, cpuInstance, pipelineSim));
}

void storeEvent::processEvent(){
    printDebug("Processing store event at time: " + std::to_string(pipelineSim->clk), 3);
    cpuInstance->store();
    pipelineSim->scheduleEvent(new hazardEvent(pipelineSim->clk + 11, cpuInstance, pipelineSim));
}

void hazardEvent::processEvent(){
    printDebug("Processing hazard event at time: " + std::to_string(pipelineSim->clk), 3);
    //add hazard detection logic here
    //empty now but can handle stalls and maybe noops if needed
}