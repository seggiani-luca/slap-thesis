# 1. Dimostrare differenze tempo esec. letture stride e senza stride

Cosa ti servirà:

1. Primitiva get_time() per il tempo corrente (accurata):
```c
uint64_t get_time()
{
    struct timespec ts; // <time.h>
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec; // nanosecondi
}
```

2. Prepara 2 test: primo array linked-list, secondo array casuale
    linked-list: [ S, ..., 2S, ... ]
    casuale:     [ RANDOM ]

3. Esegui i test col codice che è nell'articolo

Cose carine:

1. Il programma dovrebbe prendere in argomento:
    - Dim. array (può essere fissa ma almeno S * ITERS);
    - S (Stride);
    - ITERS (Iterazioni).

2. Il programma dovrebbe restituire il tempo di esecuzione.

Vogliamo fare numerosi test, quindi servirà un programma "host" che lancia più
volte questo sottoprogramma e compila un CSV o qualcosa con tutti i tempi di
esecuzione, per farne un grafico.
Il grafico può essere preparato con MATLAB.

-> Nota! "We run Listing 1 on the P- and E-cores of the Apple M1, M2, and M3 
CPUs, using the `pthread_set_qos_class_self_np`".
