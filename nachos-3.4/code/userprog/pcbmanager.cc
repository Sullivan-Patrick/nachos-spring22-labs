#include "pcbmanager.h"
#include "system.h"


PCBManager::PCBManager(int maxProcesses) {

    bitmap = new BitMap(maxProcesses);
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
        if(pcbs[i] == NULL) {
            printf("pcb number %d from pcbs list is null\n", i);
        } else {
            printf("pcb at index %d, with id %d\n", i, pcbs[i]->pid);
        }
    }
    for(int i = 0; i < sizeof(pcbs); i++) {
        if(pcbs[i] != NULL) {
            if(pcbs[i]->pid == pcb->pid) {
                printf("Found the pcb to deallocate at index %d, with id %d\n", i, pcb->pid);
                index = i;
                found = true;
                break;
            }
        }
    }
    if (!found) {
        return -1;
    }

    // Acquire pcbManagerLock
    pcbManagerLock->Acquire();

    bitmap->Clear(pcb->pid);

    // Release pcbManagerLock
    pcbManagerLock->Release();

    delete pcbs[pcb->pid];
    // for(int i = 0; i < sizeof(pcbs); i++) {
    //     if(pcbs[i] == NULL) {
    //         printf("pcb number %d from pcbs list is null\n", i);
    //         break;
    //     } else {
    //         printf("pcb from pcb array, pcb id %d\n", pcbs[i]->pid);
    //     }
    // }

    pcbs[index] = NULL;

}

PCB* PCBManager::GetPCB(int pid) {
    if (pid < 0) {
        return NULL;
    }
    return pcbs[pid];
}