#include "../sim/pipeline.h"

uint32_t pc;
uint32_t int_reg_bank[32];
float   float_reg_bank[32];

bool pipelineSimulation::notStalled(){
    if(pipelineBusy) return false;
    else return true;
}

void pipelineSimulation::halt(){
    while(!eventQueue.empty()){
        event * nextEvent = eventQueue.top();
        eventQueue.pop();
        delete nextEvent;
    }
    pc = 5;
    halted = true;
}

void pipelineSimulation::run(){

    scheduleEvent(new fetchEvent(clk, cpuInstance, this));

    while(!eventQueue.empty()){ //clock cycle loop
        while(eventQueue.top()->time <= clk + 1) {
            event * nextEvent = eventQueue.top();
            nextEvent->processEvent();
            if(halted) break;
            eventQueue.pop();
            delete nextEvent;
        }
        if(debug == 1){
            printf("Clock Cycle: %04.1f", clk);
            std::cout << " Fetch: " << state.fetchState;
            std::cout << " Decode: " << state.decodeState << " Execute: " << state.executeState;
            std::cout << " Store: " << state.storeState << " Value in x1: " << cpuInstance->x1 << std::endl;
        }
        if(debug == 2){
            std::priority_queue<event*, std::vector<event *, std::allocator<event*> >, eventComparator> queueCopy = eventQueue;
            while(!queueCopy.empty()){
                event* ev = queueCopy.top();
                std::cout << "Event: " << ev->name << ", time: " << ev->time << std::endl;
                queueCopy.pop();
            }
        }
        if(clk == 4) halted = 1;
        if(halted){
            state.fetchState = "HALTED";
            state.decodeState = "HALTED";
            state.executeState = "HALTED";
            state.storeState = "HALTED";
            printf("Halting simulation\n");
            printf("Clock Cycle: %04.1f", clk);
            std::cout << " Fetch: " << state.fetchState;
            std::cout << " Decode: " << state.decodeState << " Execute: " << state.executeState;
            std::cout << " Store: " << state.storeState << " Value in x1: " << cpuInstance->x1 << std::endl;
            return;
        }
        else{
            if(stallTime > 0) pipelineBusy = 1;
            if(notStalled()){
                scheduleEvent(new fetchEvent(clk + 1.4, cpuInstance, this));
            }
            else { 
                stallTime--;
                state.executeState = "STALL";
                //scheduleEvent(new executeEvent(clk + 1.1));
                if(stallTime <= 0) pipelineBusy = 0;
            }
            clk++;
        }
    }
}

void fetchEvent::processEvent(){
    cpuInstance->fetch();
    pipelineSim->scheduleEvent(new decodeEvent(pipelineSim->clk + 1.3 + cpuInstance->stallTime, cpuInstance, pipelineSim));
}

void decodeEvent::processEvent(){
    cpuInstance->decode();
    pipelineSim->scheduleEvent(new executeEvent(pipelineSim->clk + 1.2 + cpuInstance->stallTime, cpuInstance, pipelineSim));
}

void executeEvent::processEvent(){
    cpuInstance->execute();
    pipelineSim->scheduleEvent(new storeEvent(pipelineSim->clk + 1.1, cpuInstance, pipelineSim));
}
void storeEvent::processEvent(){
    cpuInstance->store();
}
