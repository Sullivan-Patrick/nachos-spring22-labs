#include "pcb.h"


PCB::PCB(int id) {

    pid = id;
    parent = NULL;
    children = new List();
    thread = NULL;
    exitStatus = -9999;

}



PCB::~PCB() {

    delete children;

}



void PCB::AddChild(PCB* pcb) {

    children->Append(pcb);


}


int PCB::RemoveChild(PCB* pcb){

    return children->RemoveItem(pcb);

}


bool PCB::HasExited() {
    return exitStatus == -9999 ? false : true;
}


void decspn(int arg) {
    PCB* pcb = (PCB*)arg;
    printf("pcb with id %d, has exited", pcb->pid);
    if (pcb->HasExited()) {
        printf("pcb with id %d, has exited", pcb->pid);
        pcbManager->DeallocatePCB(pcb);
    } else {
        pcb->parent = NULL;
    }
}


void PCB::DeleteExitedChildrenSetParentNull() {
    if(!children->IsEmpty()) {
        printf("Children list not empty. \n");
        children->Mapcar(decspn);
    }
}