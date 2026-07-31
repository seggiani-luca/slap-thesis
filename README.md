# SLAP

Lavoro relativo alla vulnerabilità SLAP su Apple Silicon M2.
Ciò che si vuole fare è:

1. (`exec_time`) Dimostrare le differenze in tempo di esecuzione fra accessi ad
   un array con struttura predicibile e casuale, per provare che il LAP esiste;
2. Dimostrare che il LAP predice e scoprire le predizioni attraverso un side 
   channel (che sarà la cache);
3. Realizzare una primitiva di lettura di memoria arbitraria attraverso il side
   channel.
4. Riportare il tutto ad un caso reale, che potrò essere: 
    - Creare un sandbox e dimostrare che può essere rotto dalla vulnerabilità;
    - Rompere qualcosa di vero, ad esempio l'ASLR, mappaando le pagine dello 
      spazio di indirizzamento;
    - Rompere un servizio come Gmail, come riportato nell'articolo originale.
