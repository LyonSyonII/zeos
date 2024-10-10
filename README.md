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