/**
 * Alguem
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum {NJ, J1, J2, J3, J4, MAXJ};

#define SAO_IGUAIS(a, b) (a[0] == b[0] && a[1] == b[1])

typedef struct no_peca {
    char n[2], jogador;
    struct no_peca *prev;
} no_peca;

typedef struct dominoh {
    no_peca *ultima_peca;
    char n_jogador, n_pecas;
    char joga_agora;
} dominoh;

no_peca *cria_peca(dominoh *d)
{
    if (!d->ultima_peca) {
        d->ultima_peca = (no_peca *) malloc(sizeof(no_peca));
        d->ultima_peca->prev = NULL;
    }
    else {
        no_peca *aux = (no_peca *) malloc(sizeof(no_peca));
        aux->prev = d->ultima_peca;
        d->ultima_peca = aux;
    }

    d->n_pecas++;
    return d->ultima_peca;
}

int critica_peca(dominoh *d, char *peca)
{
    if (peca[0] < '0' || peca[0] > '6') return 0;
    if (peca[1] < '0' || peca[1] > '6') return 0;
    if (d->ultima_peca) {
        no_peca *aux = d->ultima_peca;
        while(aux) {
            if (SAO_IGUAIS(peca, aux->n))
                return 0;
            aux = aux->prev;
        }
    }
    return 1;
}

void distribui_pecas(dominoh *d)
{
    int pra_cada = d->n_pecas / d->n_jogador;
    int c = 0;

    no_peca *aux = d->ultima_peca;
    if (d->n_jogador == 2) {
        for (; aux && c < pra_cada; c++) {
            aux->jogador = J1;
            aux = aux->prev;
            aux->jogador = J2;
            aux = aux->prev;
        }
    }
    else {
        for (c = 0; aux && c < pra_cada; c++) {
            aux->jogador = J1;
            aux = aux->prev;
            aux->jogador = J2;
            aux = aux->prev;
            aux->jogador = J3;
            aux = aux->prev;
            aux->jogador = J4;
            aux = aux->prev;
        }
    }

    while (aux) {
        aux->jogador = NJ;
        aux = aux->prev;
    }

    srand(time(NULL));
    c = rand() % (2 * pra_cada);

    aux = d->ultima_peca;
    no_peca *aux66;
    for (; c; c--) aux = aux->prev;
    aux66 = aux;
    while (aux66->prev) aux66 = aux66->prev;
    int f = aux66->jogador;
    d->joga_agora = aux66->jogador = aux->jogador;
    aux->jogador = f;
}

int entrada(dominoh *d)
{
    char peca[3] = "66";
    do
        scanf("%d", &d->n_jogador);
    while(d->n_jogador != 2 && d->n_jogador != 4);

    if (critica_peca(d, peca)) {
        no_peca *p = cria_peca(d);
        p->n[0] = peca[0];
        p->n[1] = peca[1];
    }
    do {
        scanf("%s", peca);
        if (critica_peca(d, peca)) {
            no_peca *p = cria_peca(d);
            p->n[0] = peca[0];
            p->n[1] = peca[1];
        }
    } while (getchar() != '\n' || d->n_pecas < d->n_jogador);

    no_peca *aux = d->ultima_peca;
    while (aux) {
        printf("%c%c ", aux->n[0], aux->n[1]);
        aux = aux->prev;
    }
    printf("\n");

    return 0;
}

int checa_jogo(dominoh *d, char *hh, char *peca)
{
    if (!hh[0]) {
        if (peca[0] < peca[1]) {
            hh[0] = peca[0];
            hh[1] = peca[1];
        }
        else {
            hh[1] = peca[0];
            hh[0] = peca[1];
        }
    }
    else {
        if (hh[0] == peca[0]) hh[0] = peca[1];
        else if (hh[1] == peca[0]) hh[1] = peca[1];
        else if (hh[0] == peca[1]) hh[0] = peca[0];
        else if (hh[1] == peca[1]) hh[1] = peca[0];
    }

    printf("%c %c\n", hh[0], hh[1]);

    return 0;
}

int checa_vencedor(dominoh *d)
{

    return 0;
}

void joga(dominoh *d)
{
    int count = d->joga_agora;
    char peca[3], hh[2];
    char max_jogador = (d->n_jogador == 2) ? J2 : J4;
    hh[0] = '\0';
    for (;;)
    {
        scanf("%s", peca);
        checa_jogo(d, hh, peca);
        checa_vencedor(d);
        if (peca[0] == 'f') break;
        count++;
        if (count > max_jogador) count = J1;
    }
}

int main()
{
    dominoh doh;
    doh.ultima_peca = NULL;
    doh.n_pecas = 0;
    entrada(&doh);
    distribui_pecas(&doh);
    joga(&doh);

    return 0;
}
