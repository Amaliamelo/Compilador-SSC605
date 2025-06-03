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

// Lista de palavras reservadas da linguagem
extern char palReservadas[20][20];

// Função para reconhecer e tratar comentários iniciados com '{' e encerrados com '}'
int comentario(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

// Função para reconhecer números inteiros válidos
int numero(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

// Função para reconhecer identificadores (variáveis) ou palavras reservadas
int identificador(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

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