# SLAP

Lavoro relativo alla vulnerabilità SLAP su Apple Silicon M2

## Scaletta

Piano di azione:

1. Dimostrare differenze tempo esec. letture stride e senza stride;
2. Dimostrare che il LAP predice attraverso il side channel (cache);
    -> Mail lettieri
3. Realizzare una primitiva di lettura arbitraria?
4. Fare qualcosa di ganzo (loro fanno Gmail, te puoi fare altro)
    4.1. Crei un sandbox e dimostri che puoi romperlo;
    4.2. Rompi qualcosa di vero? Potresti rompere l'ASLR e mappare le pagine 
         dello spazio di indirizz.
    4.3. Rompi Gmail come loro? Poco divertente ma ok.
