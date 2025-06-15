#ifndef LEXICO_H
#define LEXICO_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_DEN 30
#define LEN_ERRO_LEXICO 50

#define ENCONTRADO 5
#define NAO_ENCONTRADO 2
#define FIM_DE_ARQUIVO 3

#define MAX_IDENT_LEN 100

// Estrutura que representa símbolos e seus respectivos nomes (denominadores)
typedef struct {
    char simbolo[3];
    char denominador[MAX_DEN];
} SSimb;

// Função para reconhecer operadores relacionais: <>, <=, >=, =, >, < e dois-pontos
int relacional(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

// Função para reconhecer o símbolo de atribuição :=
int atribuicao(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

// Função para reconhecer os operadores aritméticos '+' e '-'
int operadorMaisMenos(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

// Função para reconhecer operadores de pontuação: vírgula, ponto e vírgula, ponto, dois-pontos
int operadorPontuacao(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

// Função para reconhecer operadores de divisão e multiplicação: '/' e '*'
int operadorDivMult(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

// Função para reconhecer parênteses direitos: )
int ParentesesDireito(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

// Função para reconhecer parênteses esquerdos: (
int ParentesesEsquerdo(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

#endif // LEXICO_H