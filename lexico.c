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

// Tabela de palavras reservadas
const char *tabela_reservados[] = {
    "program", "var", "integer", "real", "begin", "end", "while", "read", "write",
    "const", "procedure", "else", "then", "if", "do", "to", "for"
};

// Vetor de operadores
const char *operadores[] = {
    ";", ":", "+", "-", "*", "/", "(", ")", "=", ",", ">", "<", "."
};

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
    FILE *arquivoSaida = fopen("saida.txt", "w");
    if (arquivoSaida == NULL) {
        printf("Erro ao criar o arquivo de saída: %s\n", "saida.txt");
        return;
    }

    fprintf(arquivoSaida, "Lexema\t\tToken\t\tLinha\tStatus\n");
    fprintf(arquivoSaida, "-----------------------------------------------\n");

    for (int i = 0; i < tokenIndex; i++) {
        fprintf(arquivoSaida, "Lexema: %s, Token: %s, Linha: %d, Status: %s\n",
               tokens[i].lexema,
               tokens[i].token,
               tokens[i].linha,
               tokens[i].status ? "Válido" : "Inválido");
    }
    fclose(arquivoSaida);

}

// Função para verificar se o token é uma palavra
int ehPalavra(const char *token) {
    for (int i = 0; token[i] != '\0'; i++) {
        if (!isalpha(token[i])) { // Se não for letra
            return 0;
        }
    }
    return 1;
}

// Função para verificar se o token é um número
int ehNumero(const char *token) {
    for (int i = 0; token[i] != '\0'; i++) {
        if (!isdigit(token[i])) { // Se não for dígito
            return 0;
        }
    }
    return 1;
}
int ehPalavraReservada(const char *lexema) {
    for (int i = 0; i < sizeof(tabela_reservados) / sizeof(tabela_reservados[0]); i++) {
        if (strcmp(lexema, tabela_reservados[i]) == 0) {
            return 1; // Palavra reservada encontrada
        }
    }
    return 0; // Não é palavra reservada
}

// Função para verificar se o token é um símbolo
int ehOperador(const char *lexema) {
    for (int i = 0; i < sizeof(operadores) / sizeof(operadores[0]); i++) {
        if (strcmp(lexema, operadores[i]) == 0) {
            return 1; // Operador encontrado
        }
    }
    return 0; // Não é operador
}