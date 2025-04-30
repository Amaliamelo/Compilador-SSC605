#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Estrutura para representar tokens
typedef struct {
    char lexema[100];
    char token[100];
    int linha;
    int status;
} Token;

// Array de tokens (simulação de vetor dinâmico)
Token tokens[1000];
int tokenIndex = 0;

// Função para adicionar um token ao array
void adicionarToken(const char *lexema, const char *token, int linha, int status) {
    strcpy(tokens[tokenIndex].lexema, lexema);
    strcpy(tokens[tokenIndex].token, token);
    tokens[tokenIndex].linha = linha;
    tokens[tokenIndex].status = status;
    tokenIndex++;
}

// Função para validar identificadores
void automatoIdentificador(const char *palavra, int num_linha) {
    int s = 0; // Estado
    int i = 0;
    int flag = 1; // Controla o loop
    char c;

    while (flag) {
        c = palavra[i];
        switch (s) {
            // Estado 0
            case 0:
                if (isalpha(c)) {
                    s = 1;
                } else {
                    s = 3;
                }
                break;

            // Estado 1
            case 1:
                if (isalnum(c)) {
                    s = 1;
                    if (i == strlen(palavra) - 1) {
                        adicionarToken(palavra, "ident", num_linha, 1);
                        flag = 0;
                    }
                } else if (i == strlen(palavra)) {
                    adicionarToken(palavra, "ident", num_linha, 1);
                    flag = 0;
                } else {
                    s = 2;
                }
                break;

            // Estado 2
            case 2:
                adicionarToken(palavra, "Identificador com caracter inválido", num_linha, 0);
                flag = 0;
                break;

            // Estado 3
            case 3:
                adicionarToken(palavra, "Identificador não inicia com letra", num_linha, 0);
                flag = 0;
                break;
        }
        i++;
    }
}

// Função para validar números inteiros e reais
void automatoNumero(const char *aux, int num_linha) {
    int s = 0; // Estado inicial
    int tama_cont = 0; // Controle do tamanho
    int i;
    char c;

    for (i = 0; i < strlen(aux); i++) {
        c = aux[i];
        switch (s) {
            // Estado 0
            case 0:
                if (isdigit(c)) {
                    s = 2;
                    tama_cont++;
                    if (strlen(aux) == 1) { // Validar número inteiro de um único dígito
                        adicionarToken(aux, "num_inteiro", num_linha, 1);
                        return;
                    }
                } else if (c == '-' || c == '+') { // Aceita sinais no início
                    s = 1;
                } else {
                    s = 5;
                }
                break;

            // Estado 1
            case 1:
                if (isdigit(c)) {
                    tama_cont++;
                    s = 2;
                } else {
                    s = 4;
                }
                break;

            // Estado 2
            case 2:
                if (i == strlen(aux) - 1) { // Último caractere
                    if (isdigit(c)) {
                        adicionarToken(aux, "num_inteiro", num_linha, 1);
                    } else {
                        adicionarToken(aux, "Numero mal formado", num_linha, 0);
                    }
                    return;
                } else if (c == '.') {
                    s = 3;
                } else if (isdigit(c)) {
                    tama_cont++;
                } else {
                    s = 4;
                }
                break;

            // Estado 3
            case 3:
                if (i == strlen(aux) - 1) { // Último caractere
                    if (isdigit(c)) {
                        adicionarToken(aux, "num_real", num_linha, 1);
                    } else {
                        adicionarToken(aux, "Numero mal formado", num_linha, 0);
                    }
                    return;
                } else if (isdigit(c)) {
                    tama_cont++;
                } else {
                    s = 4;
                }
                break;

            // Estado 4
            case 4:
                adicionarToken(aux, "Numero mal formado", num_linha, 0);
                return;

            // Estado 5
            case 5:
                adicionarToken(aux, "Numero mal formado", num_linha, 0);
                return;
        }
    }
}

// Função para imprimir os tokens
void imprimirTokens() {
    for (int i = 0; i < tokenIndex; i++) {
        printf("Lexema: %s, Token: %s, Linha: %d, Status: %s\n",
               tokens[i].lexema,
               tokens[i].token,
               tokens[i].linha,
               tokens[i].status ? "Válido" : "Inválido");
    }
}

// Teste do automatoIdentificador
int main() {
    automatoIdentificador("var1", 1);
    //o numero ao lado da váriavel seria o numero da linha em que essa variável está
    automatoIdentificador("123abc", 2);
    automatoIdentificador("var_2", 3);
    automatoIdentificador("Abc123", 4);
    automatoNumero("123", 1);
    automatoNumero("-123", 2);
    automatoNumero("+3.14", 3);
    automatoNumero("3.", 4);
    automatoNumero(".45", 5);
    automatoNumero("abc", 6);

    imprimirTokens();
    return 0;
}