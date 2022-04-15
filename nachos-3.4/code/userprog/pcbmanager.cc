#include "pcbmanager.h"
#include "system.h"


PCBManager::PCBManager(int maxProcesses) {

    bitmap = new BitMap(maxProcesses);
    printf("bitmap built with %d bits\n", maxProcesses);
    pcbs = new PCB*[maxProcesses];

    for(int i = 0; i < maxProcesses; i++) {
        pcbs[i] = NULL;
    }

}


PCBManager::~PCBManager() {

    delete bitmap;

    delete pcbs;

}


PCB* PCBManager::AllocatePCB() {

    // Acquire pcbManagerLock
    pcbManagerLock->Acquire();

    int pid = bitmap->Find();
    printf("next pid for the fork call %d\n", pid);

    // Release pcbManagerLock
    pcbManagerLock->Release();

    ASSERT(pid != -1);

    pcbs[pid] = new PCB(pid);

    // return newly allocated PCB
    return pcbs[pid];

}


int PCBManager::DeallocatePCB(PCB* pcb) {

    // Check is pcb is valid -- check pcbs for pcb->pid
    bool found = false;
    int index = 0;
    for(int i = 0; i < sizeof(pcbs); i++) {
        if(pcbs[i] != NULL) {
            if(pcbs[i]->pid == pcb->pid) {
                index = i;
                printf("pcb with pid %d to deallocate was found at index %d\n", pcb->pid, i);
                found = true;
                break;
            } else {
                printf("pcb on pcbs array with pid %d\n", pcbs[i]->pid, i);
            }
        }
    }
    if (!found) {
        printf("pcb with pid %d to deallocate was not found in pcbs\n", pcb->pid);
        return -1;
    }

    // Acquire pcbManagerLock
    pcbManagerLock->Acquire();

    bitmap->Clear(index);

    // Release pcbManagerLock
    pcbManagerLock->Release();

    if (pcb->pid < sizeof(pcbs)) {
        delete pcbs[index];
    }

    pcbs[index] = NULL;

}

PCB* PCBManager::GetPCB(int pid) {
    if (pid < 0) {
        return NULL;
    }
    return pcbs[pid];
}