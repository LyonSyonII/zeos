> Uriel Camí & Liam Garriga

# ZEOS - FiB UPC (2024-2025)

## Directory Structure

- [SO2] bootsect.S: boot sector definition
- [SO2] build.c: build process
- clock.c: Clock interrupt management
- [SO2] devices.c: sys_write_console
- entry.S: Interrupt/Syscall (HW/SW) + Exception handlers + SAVE_ALL/RESTORE_ALL macros
- hardware.c: enable hardware interrupt mask
- interrupt.c: set_idt + print_page_fault
- [SO2] io.c: Console output
- keyboard.c: Keyboard interrupt management + char_map
- libc_sys.S: libc syscall wrappers
- libc.c: utils + syscall wrapper API + errno
- [SO2] list.c: linked list implementation
- [SO2] mm.c: Memory definition and initialization
- [SO2] sched.c: Stub Scheduler implementation
- sys_call_table.S: Syscall id definition
- sys.c: Syscall routine implementations
- system.c: Kernel main (boot, init, launch user code)
- user.c: User code
- utils.c: copy_from_XXX, access_ok, itox, ...

## TO-DO

- [ ] Adapt the task_struct definition.
- [ ] Initialize a free queue.
- [ ] Initialize a ready queue.
- [ ] Implement the initial processes initialization.
- [ ] Implement the task_switch function.
- [ ] Implement the inner_task_switch function.
- [ ] Implement the getpid system call.
- [ ] Implement the fork system call.
- [ ] Implement process scheduling.
- [ ] Implement the exit system call.
- [ ] Implement the block system call.
- [ ] Implement the unblock system call

### Notes
- modificar task struct (pcb)
	- 10 pcbs (task_struct tasks[10])
- freequeue (pcbs lliures)
- readyqueue (processos en estat de ready)
- inicialitzar processos
	- init: proces d'usuari 
		- init_mm (inicialitza la mmu ...)
			- cr3 (Apunta a DIR, no TP)
			- tants DIR com processos (i TP)
				- Per tant 10 DIR i 10 TP
		- set_user_pages()
	- idle: proces a executar quan no n'hi ha cap a la readyqueue
	- init -> idle -> init
	- init -> init

- scheduler
	- cada cert temps canviem de proces (task_switch)
	- no ho hem de fer a la interrupcio de rellotge
		- millor fer-ho a la interrupcio de teclat per debugar
- task_switch(new)

### E1
- [x] Complete Zeos code.
    - [x] Implement the macro RESTORE_ALL.
    - [x] Implement the macro EOI.
- [x] Implement the keyboard management.
    - [x] Implement the keyboard service routine.
    - [x] Implement the keyboard handler.
    - [x] Initialize the IDT with the keyboard handler
    - [x] Enable the interrupt.
- [x] Implement the write system call.
    - Hem d'utilitzar `sysenter` (fer el que fa `int` manualment)
      Excepte canvi d'usuari a sistema i IDT
      Hem de guardar la direccio del handler global a registres de MSR a l'inici del sistema `wrmsr/rdmsr`
      Perdem la posicio de la pila d'usuari `%ebp` i posicio de codi (etiqueta), les guardem a la pila
    - Copiem parametres a EDX, ECX, EBX
      Si en tenim massa, guardem a la pila i un dels parametres sera la direccio de la pila on es troben
- [x] Implement the clock management.
- [x] Implement the gettime system call.
- [x] Implement the page fault exception management.
