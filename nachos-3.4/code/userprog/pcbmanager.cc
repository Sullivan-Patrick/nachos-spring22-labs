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
    for(int i = 0; i < sizeof(pcbs); i++) {
        if(pcbs[i]->pid == pcb->pid) {
            found = true;
            break;
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

    pcbs[pcb->pid] = NULL;

}

PCB* PCBManager::GetPCB(int pid) {
    printf("pcbs array length %d", sizeof(pcbs));
    if(pcbs[pid] == NULL) {
        printf("pcb is null \n");
    }
    return pcbs[pid];
}