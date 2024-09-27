- [x] Complete Zeos code.
    - [x] Implement the macro RESTORE_ALL.
    - [x] Implement the macro EOI.
- [] Implement the keyboard management.
    - [] Implement the keyboard service routine.
    - [] Implement the keyboard handler.
    - [] Initialize the IDT with the keyboard handler
    - [] Enable the interrupt.
- [] Implement the write system call.
    - Hem d'utilitzar `sysenter` (fer el que fa `int` manualment)
      Excepte canvi d'usuari a sistema i IDT
      Hem de guardar la direccio del handler global a registres de MSR a l'inici del sistema `wrmsr/rdmsr`
      Perdem la posicio de la pila d'usuari `%ebp` i posicio de codi (etiqueta), les guardem a la pila
    - Copiem parametres a EDX, ECX, EBX
      Si en tenim massa, guardem a la pila i un dels parametres sera la direccio de la pila on es troben
- [] Implement the clock management.
- [] Implement the gettime system call.
- [] Implement the page fault exception management.