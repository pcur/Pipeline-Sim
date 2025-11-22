#include "../sim/pipeline.h"
#include "../sim/helpers.h"

bool pipelineSimulation::notStalled(){
    printDebug("Checking if pipeline is stalled: " + std::to_string(pipelineBusy), 2);
    if(pipelineBusy) return false;
    else return true;
}

void pipelineSimulation::halt(){
    printDebug("Halting pipeline simulation", 0);
    while(!eventQueue.empty()){
        event * nextEvent = eventQueue.top();
        eventQueue.pop();
        delete nextEvent;
    }
    printDebug("All events removed from queue.", 1);
    halted = true;
}

void pipelineSimulation::endCyclePrintOut(){
    // Debug output for each clock cycle
        if(debug == 1){
            std::stringstream ss;
            ss << simName << " Clock Cycle: " << (clk / 10) << " Fetch: " << cpuInstance->state.fetchState << " Decode: " << cpuInstance->state.decodeState << " Execute: " << cpuInstance->state.executeState << " Store: " << cpuInstance->state.storeState;
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
    cpuInstance->stallTime = 0;
    halted = false;
    pipelineBusy = false;
    printDebug("Starting pipeline simulation runtime loop", 2);
    scheduleEvent(new fetchEvent(clk + 9, this));
}

void pipelineSimulation::tick(){
    std::stringstream ss;
    ss << "\n========={ Simulation Tick: " << clk << " for cpu: " << simName << " }=========";
    printDebug(ss.str(), 1);
    if (clk % 10 == 0){
        endCyclePrintOut();
    }
    while(eventQueue.top()->time <= clk) {
            printDebug("Processing event scheduled for time: " + std::to_string(eventQueue.top()->time), 5);
            // Process all events scheduled for this clock cycle in order of time
            event * nextEvent = eventQueue.top();
            /*if(cpuInstance->pipelineBusy){
                if(nextEvent->name == "decode" || nextEvent->name == "execute"){
                    printDebug("Pipeline is busy, deferring event: " + nextEvent->name, 2);
                    break;
                }
            }*/
            nextEvent->processEvent();
            if(halted) break;
            //may need stall logic here
            eventQueue.pop();
            delete nextEvent;
    }
    if(halted || cpuInstance->shouldHalt){
        cpuInstance->state.fetchState = "HALTED";
        cpuInstance->state.decodeState = "HALTED";
        cpuInstance->state.executeState = "HALTED";
        cpuInstance->state.storeState = "HALTED";
        printDebug("Halting simulation", 1);
        std::stringstream ss;
        ss << "Simulation Tick: " << clk << " Fetch: " << cpuInstance->state.fetchState << " Decode: " << cpuInstance->state.decodeState;
        printDebug(ss.str(), 0);
        halt();
    }
    else{
        if(cpuInstance->stallTime > 0) {
            printDebug("Pipeline is stalled for " + std::to_string(cpuInstance->stallTime) + " more cycles", 1);
            cpuInstance->stallTime--;
            if(cpuInstance->stallTime > 0){
                cpuInstance->state.executeState = "STALLED FOR: " + std::to_string(cpuInstance->stallTime + 5) + " CYCLES";
            }
            else cpuInstance->state.executeState = cpuInstance->stalledState;
            //scheduleEvent(new executeEvent(clk + 1.1));
            if(cpuInstance->stallTime == 0) cpuInstance->pipelineBusy = 0;
        }
        else {
            if (clk % 10 == 9){ // only schedule fence once per cpu cycle{}
                scheduleEvent(new fetchEvent(clk + 10, this));
                printDebug("Pipeline is not stalled, scheduling new fetch event", 1);
            }
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
    pipelineSim->cpuInstance->fetch();
    printDebug("Scheduling decode event at tick " + std::to_string(pipelineSim->clk + 8 + pipelineSim->cpuInstance->totalStallTime), 4);
    pipelineSim->scheduleEvent(new decodeEvent(pipelineSim->clk + 8 + pipelineSim->cpuInstance->totalStallTime, pipelineSim));
}

void decodeEvent::processEvent(){
    printDebug("Processing decode event at time: " + std::to_string(pipelineSim->clk), 3);
    pipelineSim->cpuInstance->decode();
    printDebug("Scheduling execute event at tick " + std::to_string(pipelineSim->clk + 8 + pipelineSim->cpuInstance->totalStallTime), 4);
    pipelineSim->scheduleEvent(new executeEvent(pipelineSim->clk + 8 + pipelineSim->cpuInstance->totalStallTime, pipelineSim));
}

void executeEvent::processEvent(){
    printDebug("Processing execute event at time: " + std::to_string(pipelineSim->clk), 3);
    pipelineSim->cpuInstance->execute();
    printDebug("Scheduling store event at tick " + std::to_string(pipelineSim->clk + 8), 4);
    pipelineSim->scheduleEvent(new storeEvent(pipelineSim->clk + 8 + pipelineSim->cpuInstance->totalStallTime, pipelineSim));
}

void storeEvent::processEvent(){
    printDebug("Processing store event at time: " + std::to_string(pipelineSim->clk), 3);
    pipelineSim->cpuInstance->store();
    
    // Check if halt was signaled during store stage
    if(pipelineSim->cpuInstance->shouldHalt) {
        printDebug("Halt signal detected, stopping pipeline", 1);
        pipelineSim->halted = true;
        return;
    }
    
    printDebug("Scheduling hazard event at tick " + std::to_string(pipelineSim->clk + 8), 4);
    pipelineSim->scheduleEvent(new hazardEvent(pipelineSim->clk + 8, pipelineSim));
}

void hazardEvent::processEvent(){
    printDebug("Processing hazard event at time: " + std::to_string(pipelineSim->clk), 3);
    //add hazard detection logic here
    //empty now but can handle stalls and maybe noops if needed
}