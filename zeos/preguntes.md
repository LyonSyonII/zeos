
# M5
- ??? No usa interficie demanada
    - Comprovar parametres
    - Exit de TID = 0 borra totes les pagines
    - Fork copia totes les pagines

# Semafors
- Que faig quan es destrueix un semafor i hi ha processos que poden esperar-se encara?
- Problema: Podria ser que es creessin suficients semafors per fer la volta i reutilitzar el destruit
```c
sem_t sem = semCreate(); // sems[2]
fork();
// ...
semWait(sem);
semDestroy(sem);
// ...
semCreate(); // casualment es sems[2] tambe (i fork encara s'ha d'esperar a l'original)
// ...
```

- En comptes d'enviar un sem_t
- Creo un struct sem_t (sistema) que tingui un id
- Retorno id del semafor camuflada com a punter


# Joc
Frogger
