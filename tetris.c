#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

// =======================
//    Tetris Stack – C
//    Fila (circular) + Pilha (linear)
// =======================

#define MAX_FILA  5
#define MAX_PILHA 3

// ---- Tipos de peça clássicos do Tetris
static const char TIPOS[] = {'I','O','T','S','Z','J','L'};
#define QTD_TIPOS (int)(sizeof(TIPOS)/sizeof(TIPOS[0]))

// ---------- Estruturas ----------
typedef struct {
    char tipo;  // 'I','O','T','S','Z','J','L'
    int  id;    // sequencial
} Peca;

typedef struct {
    Peca itens[MAX_FILA];
    int inicio;  // aponta para o índice do primeiro elemento
    int fim;     // aponta para a próxima posição livre para inserir
    int total;   // qtd de elementos na fila
} Fila;

typedef struct {
    Peca itens[MAX_PILHA];
    int topo;    // índice do topo ( -1 quando vazia )
} Pilha;

// ---------- Gerador de Peças ----------
Peca gerarPeca(void) {
    static int proxId = 1;
    Peca p;
    p.tipo = TIPOS[rand() % QTD_TIPOS];
    p.id   = proxId++;
    return p;
}

// ---------- Fila (circular) ----------
void inicializarFila(Fila *f) {
    f->inicio = 0;
    f->fim    = 0;
    f->total  = 0;
}
int filaVazia(const Fila *f) { return f->total == 0; }
int filaCheia (const Fila *f) { return f->total == MAX_FILA; }

int enqueue(Fila *f, Peca p) {
    if (filaCheia(f)) return 0;
    f->itens[f->fim] = p;
    f->fim = (f->fim + 1) % MAX_FILA;  // índice circular
    f->total++;
    return 1;
}

int dequeue(Fila *f, Peca *out) {
    if (filaVazia(f)) return 0;
    if (out) *out = f->itens[f->inicio];
    f->inicio = (f->inicio + 1) % MAX_FILA; // índice circular
    f->total--;
    return 1;
}

// Acessa o i-ésimo elemento a partir da frente (0 = frente)
Peca* filaAt(Fila *f, int i) {
    if (i < 0 || i >= f->total) return NULL;
    int idx = (f->inicio + i) % MAX_FILA;
    return &f->itens[idx];
}

// Mantém a fila sempre com 5 peças (repondo com gerarPeca())
void reabastecerFila(Fila *f) {
    while (!filaCheia(f)) {
        enqueue(f, gerarPeca());
    }
}

// ---------- Pilha (linear) ----------
void inicializarPilha(Pilha *p) {
    p->topo = -1;
}
int pilhaVazia(const Pilha *p) { return p->topo == -1; }
int pilhaCheia (const Pilha *p) { return p->topo == MAX_PILHA - 1; }

int push(Pilha *p, Peca x) {
    if (pilhaCheia(p)) return 0;
    p->itens[++(p->topo)] = x;
    return 1;
}

int pop(Pilha *p, Peca *out) {
    if (pilhaVazia(p)) return 0;
    if (out) *out = p->itens[p->topo];
    p->topo--;
    return 1;
}

Peca* topoPilha(Pilha *p) {
    if (pilhaVazia(p)) return NULL;
    return &p->itens[p->topo];
}

// ---------- Exibição ----------
void mostrarFila(const Fila *f) {
    printf("\nFila (frente -> tras) [%d/%d]: ", f->total, MAX_FILA);
    if (f->total == 0) { printf("(vazia)\n"); return; }
    for (int i = 0; i < f->total; i++) {
        int idx = (f->inicio + i) % MAX_FILA;
        printf("[%c#%d]%s", f->itens[idx].tipo, f->itens[idx].id, (i == f->total-1 ? "" : " -> "));
    }
    printf("\n");
}

void mostrarPilha(const Pilha *p) {
    printf("Pilha (base .. topo) [%d/%d]: ", p->topo + 1, MAX_PILHA);
    if (p->topo == -1) { printf("(vazia)\n"); return; }
    for (int i = 0; i <= p->topo; i++) {
        printf("[%c#%d]%s", p->itens[i].tipo, p->itens[i].id, (i == p->topo ? "" : " , "));
    }
    if (p->topo >= 0) printf("  <topo=[%c#%d]>", p->itens[p->topo].tipo, p->itens[p->topo].id);
    printf("\n");
}

void mostrarEstado(const Fila *f, const Pilha *p) {
    mostrarFila(f);
    mostrarPilha(p);
    printf("\n");
}

// 4 - Trocar a peça da frente da fila com o topo da pilha
int trocarFrenteComTopo(Fila *f, Pilha *p) {
    if (filaVazia(f)) {
        printf("Falha: fila vazia.\n");
        return 0;
    }
    if (pilhaVazia(p)) {
        printf("Falha: pilha vazia.\n");
        return 0;
    }
    int idxFrente = f->inicio;           // acesso direto no array circular
    int idxTopo   = p->topo;              // topo na pilha
    Peca tmp = f->itens[idxFrente];
    f->itens[idxFrente] = p->itens[idxTopo];
    p->itens[idxTopo]   = tmp;
    return 1;
}

// 5 - Trocar os 3 primeiros da fila com as 3 peças da pilha
int trocarTres(Fila *f, Pilha *p) {
    if (p->topo + 1 != 3) {
        printf("Falha: a pilha precisa ter exatamente 3 pecas.\n");
        return 0;
    }
    if (f->total < 3) {
        printf("Falha: a fila precisa ter pelo menos 3 pecas.\n");
        return 0;
    }
    // três primeiros da fila: posições 0,1,2 a partir de 'inicio'
    for (int i = 0; i < 3; i++) {
        int idxFila  = (f->inicio + i) % MAX_FILA;
        int idxPilha = i; // base..topo = 0..2
        Peca tmp = f->itens[idxFila];
        f->itens[idxFila] = p->itens[idxPilha];
        p->itens[idxPilha] = tmp;
    }
    return 1;
}

// ---------- Utilitários ----------
void pausaEnter(void) {
    printf("\nPressione ENTER para continuar...");
    int c;
    // Consumir até newline
    while ((c = getchar()) != '\n' && c != EOF) { /* descarta */ }
    // Espera um ENTER
    getchar();
}

void limparEntrada(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) { /* limpa buffer */ }
}

// ---------- Principal ----------
int main(void) {
    srand((unsigned)time(NULL));

    Fila fila;
    Pilha pilha;
    inicializarFila(&fila);
    inicializarPilha(&pilha);

    // Preenche a fila com 5 peças iniciais
    reabastecerFila(&fila);

    int opcao;

    do {
        printf("\n=============================\n");
        printf("  Desafio Tetris Stack (C)\n");
        printf("=============================\n");
        mostrarEstado(&fila, &pilha);
        printf("Menu:\n");
        printf(" 1 - Jogar peca (remove da frente da fila)\n");
        printf(" 2 - Enviar peca da fila para a reserva (pilha)\n");
        printf(" 3 - Usar peca da reserva (pop da pilha)\n");
        printf(" 4 - Trocar peca da frente com topo da pilha\n");
        printf(" 5 - Trocar 3 primeiros da fila com os 3 da pilha\n");
        printf(" 0 - Sair\n");
        printf("Escolha: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida.\n");
            limparEntrada();
            continue;
        }
        limparEntrada();

        printf("\n");
        switch (opcao) {
            case 1: { // Jogar peca (dequeue) e repor com nova
                Peca jogada;
                if (!dequeue(&fila, &jogada)) {
                    printf("Nao ha peca para jogar: fila vazia.\n");
                } else {
                    printf("Jogou peca: [%c#%d]\n", jogada.tipo, jogada.id);
                    // Repor para manter 5
                    reabastecerFila(&fila);
                }
                break;
            }
            case 2: { // Enviar da fila para pilha (push) e repor na fila
                if (pilhaCheia(&pilha)) {
                    printf("Reserva cheia (pilha cheia). Nao foi possivel enviar.\n");
                    break;
                }
                Peca frente;
                if (!dequeue(&fila, &frente)) {
                    printf("Fila vazia. Nada para enviar.\n");
                    break;
                }
                push(&pilha, frente);
                printf("Enviou [%c#%d] da fila para a reserva.\n", frente.tipo, frente.id);
                // Repor para manter 5
                reabastecerFila(&fila);
                break;
            }
            case 3: { // Usar peca da reserva (pop)
                Peca usada;
                if (!pop(&pilha, &usada)) {
                    printf("Reserva vazia. Nada para usar.\n");
                } else {
                    printf("Usou peca da reserva: [%c#%d]\n", usada.tipo, usada.id);
                }
                // Fila já está com 5 (não mexe), então nada a reabastecer aqui.
                break;
            }
            case 4: { // Trocar frente da fila com topo da pilha
                if (trocarFrenteComTopo(&fila, &pilha)) {
                    printf("Troca realizada: frente da fila <-> topo da pilha.\n");
                }
                break;
            }
            case 5: { // Trocar 3 primeiros da fila com os 3 da pilha
                if (trocarTres(&fila, &pilha)) {
                    printf("Troca de 3 realizada entre fila (3 primeiros) e pilha (3 itens).\n");
                }
                break;
            }
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida.\n");
        }

        if (opcao != 0) {
            mostrarEstado(&fila, &pilha);
            // pausaEnter(); // descomente se quiser pausar a cada ação
        }

    } while (opcao != 0);

    return 0;
}
