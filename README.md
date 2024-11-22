> Uriel Camí & Liam Garriga

# ZEOS - FiB UPC (2024-2025)

## Preguntes
- sys.c:108-113 (d'on treu les adreces?)
- sched.c:88 (task1 és fill de Idle o no te pare?)

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
- (1 point) Keyboard support stores keys in a circular buffer.
- (1 point) Functional getKey feature.
- (,5 points) Functional gotoXY feature.
- (1 points) Functional changeColor and clrscr features.
- (2 points) Functional create_thread and exit system_call.
- (2 point) Functional synchronization support
- (1 points) Functional memRegGet and memRegDel features.
- (1 point) Functional game using different implemented features.
- (,5 point) Remove the requirement of exit at the finalization of the thread.
- [Optional] (1 point) Challenge: Implement a user level slab allocator on top of
‘memRegGet’ regions.


### Notes
- fork nomes duplica el thread actual
- memregget: busca una regio del proces nova
  - memregdel: hem d'esborrar la regio, tenint nomes un punter (guardar info sobre la mida de la regio)

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

### E2
- [x] Adapt the task_struct definition.
- [x] Initialize a free queue.
- [x] Initialize a ready queue.
- [x] Implement the initial processes initialization.
- [x] Implement the task_switch function.
- [x] Implement the inner_task_switch function.
- [x] Implement the getpid system call.
- [x] Implement the fork system call.
  - [x] Basic implementation
  - [x] Add error handling
- [x] Implement process scheduling.
  - [x] Basic implementation
  - [x] Test more cases
- [x] Implement the exit system call.
- [x] Implement the block system call.
- [x] Implement the unblock system call
- [ ] Modify user.c explanation to account for changed behaviour