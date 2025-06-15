// sintatico.h

#ifndef SINTATICO_H
#define SINTATICO_H

#include <stdio.h>
#include <stdlib.h>
#include "lexico.h" // Inclui as definições de Token e getNextToken

// Declaração das funções do analisador sintático
void programa();
void bloco();
void declaracao_const();
void lista_const();
void declaracao_var();
void lista_var();
void declaracao_proc();
void comando();
void lista_comando();
void expressao();
void sinal();
void termo();
void lista_termo();
void fator();
void lista_fator();
void condicao();
void relacao();

// Função para iniciar a análise sintática
void iniciarAnaliseSintatica(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

#endif // SINTATICO_H