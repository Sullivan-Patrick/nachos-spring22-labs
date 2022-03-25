#ifndef PCB_H
#define PCB_H

#include "list.h"
#include <../threads/thread.h>

class PCB {

    public:
        PCB(int id);
        ~PCB();
        int pid;
        PCB* parent;
        List* children;
        Thread* thread;
        int exitStatus;

    private:
        void AddChild(PCB* pcb);
        int RemoveChild(PCB* pcb);
        bool HasExited();
        void DeleteExitedChildrenSetParentNull();

};

#endif // PCB_H