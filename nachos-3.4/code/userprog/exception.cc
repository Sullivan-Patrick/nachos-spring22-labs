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

    int pid = 99;

    printf("System Call: [%d] invoked [Exit]\n", pid);
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

    // 2. Restore the page table for child
    // currentThread->space->RestoreState()

    // PCReg == machine->ReadRegister(PCReg)
    // print message for child creation (pid,  PCReg, currentThread->space->GetNumPages())

    // machine->Run();

}

int doFork(int functionAddr) {

    // 1. Check if sufficient memory exists to create new process
    // currentThread->space->GetNumPages() <= mm->GetFreePageCount()
    // if check fails, return -1

    // 2. SaveUserState for the parent thread
    // currentThread->SaveUserState();

    // 3. Create a new address space for child by copying parent address space
    // Parent: currentThread->space
    // childAddrSpace: new AddrSpace(currentThread->space)

    // 4. Create a new thread for the child and set its addrSpace
    // childThread = new Thread("childThread")
    // child->space = childAddSpace;

    // 5. Create a PCB for the child and connect it all up
    // pcb: pcbManager->AllocatePCB();
    // pcb->thread = childThread
    // set parent for child pcb
    // add child for parent pcb

    // 6. Set up machine registers for child and save it to child thread
    // PCReg: functionAddr
    // PrevPCReg: functionAddr-4
    // NextPCReg: functionAddr+4
    // childThread->SaveUserState();

    // 7. Call thread->fork on Child
    // childThread->Fork(childFunction, pcb->pid)

    // 8. Restore register state of parent user-level process
    // currentThread->RestoreUserState()

    // 9. return pcb->pid;

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
        int ret = doFork(machine->ReadRegister(4));
        machine->WriteRegister(2, ret);
        incrementPC();
    } else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
