// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "machine.h"
#include "addrspace.h"
#include "../threads/thread.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------


void doExit(int status) {

    int pid = currentThread->space->pcb->pid;

    printf("System Call: [%d] invoked [Exit]\n", pid);

    currentThread->space->pcb->exitStatus = status;

    // Manage PCB memory As a parent process
    PCB* pcb = currentThread->space->pcb;

    // Delete exited children and set parent null for non-exited ones
    pcb->DeleteExitedChildrenSetParentNull();

    // Manage PCB memory As a child process
    if(pcb->parent == NULL) {
        printf("pcb with pid %d, was deallocated\n", pcb->pid);
        pcbManager->DeallocatePCB(pcb);
    } else {
        printf("pcb with pid %d, was not deallocated\n", pcb->pid);
    }
    printf ("Process [%d] exits with [%d]\n", pid, status);

    delete currentThread->space;
    currentThread->Finish();

}

void incrementPC() {
    int oldPCReg = machine->ReadRegister(PCReg);

    machine->WriteRegister(PrevPCReg, oldPCReg);
    machine->WriteRegister(PCReg, oldPCReg + 4);
    machine->WriteRegister(NextPCReg, oldPCReg + 8);
}


void childFunction(int pid) {

    // 1. Restore the state of registers
    // currentThread->RestoreUserState()
    currentThread->RestoreUserState();

    // 2. Restore the page table for child
    // currentThread->space->RestoreState()
    currentThread->space->RestoreState();

    // PCReg == machine->ReadRegister(PCReg)
    int newPCReg = machine->ReadRegister(PCReg);
    // print message for child creation (pid,  PCReg, currentThread->space->GetNumPages())
    printf("Process [%d] Fork: start at address [%d] with [%d] pages memory\n", pid, newPCReg, currentThread->space->GetNumPages());

    machine->Run();
}

int doFork(int functionAddr) {

    // 1. Check if sufficient memory exists to create new process
    // currentThread->space->GetNumPages() <= mm->GetFreePageCount()
    // if check fails, return -1
    if (currentThread->space->GetNumPages() >= mm->GetFreePageCount()) {
        printf("Not Enough Memory for Child Process \n");
        return -1;
    }

    // 2. SaveUserState for the parent thread
    // currentThread->SaveUserState();
    currentThread->SaveUserState();

    // 3. Create a new address space for child by copying parent address space
    // Parent: currentThread->space
    AddrSpace* childAddrSpace = new AddrSpace(currentThread->space);


    // 4. Create a new thread for the child and set its addrSpace
    // childThread = new Thread("childThread")
    // child->space = childAddSpace;
    Thread *childThread = new Thread("childThread");
    childThread->space = childAddrSpace;

    // 5. Create a PCB for the child and connect it all up
    // pcb: pcbManager->AllocatePCB();
    // pcb->thread = childThread
    // set parent for child pcb
    // add child for parent pcb
    PCB *childPCB = pcbManager->AllocatePCB();
    childPCB->thread = childThread;
    childPCB->parent = currentThread->space->pcb;
    if(currentThread->space->pcb == NULL) {
        printf("Current thread pcb is null \n");
    } else {
        // printf("Adding new child to current thread pcb\n");
        // printf("child pcb id %d", childPCB->pid);
        currentThread->space->pcb->AddChild(childPCB);
    }
    childThread->space->pcb = childPCB;

    // 6. Set up machine registers for child and save it to child thread
    // PCReg: functionAddr
    // PrevPCReg: functionAddr-4
    // NextPCReg: functionAddr+4
    // childThread->SaveUserState();
    machine->WriteRegister(PCReg, functionAddr);
    // printf("This is the address of the function to fork %d \n", functionAddr);
    machine->WriteRegister(PrevPCReg, functionAddr - 4);
    machine->WriteRegister(NextPCReg, functionAddr + 4);
    childThread->SaveUserState();


    // 7. Call thread->fork on Child
    // childThread->Fork(childFunction, pcb->pid)
    childThread->Fork(childFunction, currentThread->space->pcb->pid);

    // 8. Restore register state of parent user-level process
    // currentThread->RestoreUserState()
    // printf("Restore old state after childThread completion \n");
    currentThread->RestoreUserState();

    // 9. return pcb->pid;
    return childPCB->pid;
}

int doExec(char* filename) {

    // Use progtest.cc:StartProcess() as a guide

    // 1. Open the file and check validity
    // OpenFile *executable = fileSystem->Open(filename);
    // AddrSpace *space;

    // if (executable == NULL) {
    //     printf("Unable to open file %s\n", filename);
    //     return -1;
    // }
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return -1;
    }

    // 2. Create new address space
    // space = new AddrSpace(executable);
    pcbManager->DeallocatePCB(currentThread->space->pcb);
    delete currentThread->space;
    space = new AddrSpace(executable);

    // 3. Check if Addrspace creation was successful
    // if(space->valid != true) {
    // printf("Could not create AddrSpace\n");
    //     return -1;
    // }
    if(space->valid != true) {
        printf("Could not create AddrSpace\n");
        return -1;
    }

    // Steps 4 and 5 may not be necessary!!

    // 4. Create a new PCB for the new addrspace
    // ?. Can you reuse existing pcb?
    // PCB* pcb = pcbManager->AllocatePCB();
    // Initialize parent
    // pcb->parent = currentThread->space->pcb->parent;
    // space->pcb = pcb;
    // PCB *newPcb = pcbManager->AllocatePCB();
    // newPcb->parent = currentThread->space->pcb->parent;
    // space->pcb = newPcb;

    // 5. Set the thread for the new pcb
    // pcb->thread = currentThread;
    //newPcb->thread = currentThread;

    // 6. Delete current address space
    // delete currentThread->space;
    //delete currentThread->space;

    // 7. Set the addrspace for currentThread
    // currentThread->space = space;
    currentThread->space = space;

    // 8.     delete executable;			// close file
    delete executable;

    // 9. Initialize registers for new addrspace
    //  space->InitRegisters();		// set the initial register values
    space->InitRegisters();

    // 10. Initialize the page table
    // space->RestoreState();		// load page table register
    space->RestoreState();

    // 11. Run the machine now that all is set up
    // machine->Run();			// jump to the user progam
    // ASSERT(FALSE); // Execution nevere reaches here
    int pcReg = machine->ReadRegister(PCReg);

    machine->Run();
    ASSERT(FALSE);

    return 0;
}


int doJoin(int pid) {

    // 1. Check if this is a valid pid and return -1 if not
    // PCB* joinPCB = pcbManager->GetPCB(pid);
    // if (pcb == NULL) return -1;
    if(pid < 0) {
        return -1;
    }

    PCB* joinPCB = pcbManager->GetPCB(pid);
    if (joinPCB == NULL) {
        return -1;
    }

    // 2. Check if pid is a child of current process
    // PCB* pcb = currentThread->space->pcb;
    // if (pcb != joinPCB->parent) return -1;
    PCB* pcb = currentThread->space->pcb;
    if (pcb != joinPCB->parent) return -1;

    // 3. Yield until joinPCB has not exited
    // while(!joinPCB->hasExited) currentThread->Yield();
    while(!joinPCB->HasExited()) currentThread->Yield();

    // 4. Store status and delete joinPCB
    // int status = joinPCB->exitStatus;
    // delete joinPCB;
    int status = joinPCB->exitStatus;
    delete joinPCB;

    // 5. return status;
    return status;

}

void doYield() {
    // printf("Inside of Yield \n");
    currentThread->Yield();
}


int doKill(int pid) {

    // 1. Check if the pid is valid and if not, return -1
    // PCB* joinPCB = pcbManager->GetPCB(pid);
    // if (pcb == NULL) return -1;
    PCB* joinPCB = pcbManager->GetPCB(pid);
    if (joinPCB == NULL) return -1;

    // 2. IF pid is self, then just exit the process
    // if (pcb == currentThread->space->pcb) {
    //         doExit(0);
    //         return 0;
    // }
    if (joinPCB == currentThread->space->pcb) {
        doExit(0);
        return 0;
    }

    // 3. Valid kill, pid exists and not self, do cleanup similar to Exit
    // However, change references from currentThread to the target thread
    // pcb->thread is the target thread
    joinPCB->exitStatus = 0;
    joinPCB->DeleteExitedChildrenSetParentNull();
    if(joinPCB->parent == NULL) pcbManager->DeallocatePCB(joinPCB);
    delete joinPCB->thread->space;

    // Finish and Sleep function direct code
    (void) interrupt->SetLevel(IntOff);
    threadToBeDestroyed = joinPCB->thread;
    joinPCB->thread->setStatus(BLOCKED);
    List* readyList = scheduler->getReadyList();
    readyList->RemoveItem(joinPCB->thread);
    (void) interrupt->SetLevel(IntOn);

    printf("Process [%d] killed process [%d]\n", currentThread->space->pcb->pid, joinPCB->pid);

    // 4. return 0 for success!
    return 0;
}


// perform MMU translation to access physical memory
char* readString(int virtualAddr) {
    int i = 0;
    char* str = new char[256];
    unsigned int physicalAddr = currentThread->space->Translate(virtualAddr);

    // Need to get one byte at a time since the string may straddle multiple non-contiguous pages
    bcopy(&(machine->mainMemory[physicalAddr]),&str[i],1);
    while(str[i] != '\0' && i != 256-1)
    {
        virtualAddr++;
        i++;
        physicalAddr = currentThread->space->Translate(virtualAddr);
        bcopy(&(machine->mainMemory[physicalAddr]),&str[i],1);
    }
    if(i != 256-1 && str[i] != '\0')
    {
        str[i] = '\0';
    }

    return str;
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
    } else  if ((which == SyscallException) && (type == SC_Exit)) {
        // Implement Exit system call
        doExit(machine->ReadRegister(4));
    } else if ((which == SyscallException) && (type == SC_Fork)) {
        printf("System Call: [%d] invoked [fork]\n", currentThread->space->pcb->pid);
        int ret = doFork(machine->ReadRegister(4));
        machine->WriteRegister(2, ret);
        incrementPC();
    } else if ((which == SyscallException) && (type == SC_Yield)) {
        printf("System Call: [%d] invoked [yield]\n", currentThread->space->pcb->pid);
        doYield();
        incrementPC();
    } else if ((which == SyscallException) && (type == SC_Exec)) {
        printf("System Call: [%d] invoked [exec]\n", currentThread->space->pcb->pid);
        int virtAddr = machine->ReadRegister(4);
        char* fileName = readString(virtAddr);
        printf("Exec Program: [%d] loading [%s]\n", currentThread->space->pcb->pid, fileName);
        int ret = doExec(fileName);
        machine->WriteRegister(2, ret);
        incrementPC();
    } else if ((which == SyscallException) && (type == SC_Join)) {
        printf("System Call: %d invoked [join]\n", currentThread->space->pcb->pid);
        int ret = doJoin(machine->ReadRegister(4));
        machine->WriteRegister(2, ret);
        incrementPC();
    } else if ((which == SyscallException) && (type == SC_Kill)) {
        int ret = doKill(machine->ReadRegister(4));
        machine->WriteRegister(2, ret);
        incrementPC();
    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}

