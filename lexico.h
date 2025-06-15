// lexico.h

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

// Tipos de Tokens para o analisador sintático
typedef enum {
    TOKEN_EOF = 0, // Fim de arquivo
    TOKEN_ERROR,   // Erro léxico
    // Palavras reservadas
    TOKEN_CONST,
    TOKEN_VAR,
    TOKEN_PROCEDURE,
    TOKEN_CALL,
    TOKEN_BEGIN,
    TOKEN_END,
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_ODD,
    // Identificadores e números
    TOKEN_IDENT,
    TOKEN_NUMERO,
    // Símbolos e Operadores
    TOKEN_ASSIGN,       // :=
    TOKEN_EQ,           // =
    TOKEN_NEQ,          // <>
    TOKEN_LT,           // <
    TOKEN_LE,           // <=
    TOKEN_GT,           // >
    TOKEN_GE,           // >=
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_MULTIPLY,     // *
    TOKEN_DIVIDE,       // /
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_COMMA,        // ,
    TOKEN_SEMICOLON,    // ;
    TOKEN_PERIOD,       // .
    TOKEN_COLON         // :  (Embora não apareça diretamente na gramática, é bom ter se relacional tiver ':')
} TokenType;


// Estrutura que representa símbolos e seus respectivos nomes (denominadores) 
typedef struct {
    char simbolo[3];
    char denominador[MAX_DEN];
    TokenType token_type; // <-- ADICIONE ESTA LINHA AQUI
} SSimb;

// Lista de palavras reservadas da linguagem 
extern char palReservadas[20][20];

// Estrutura do Token
typedef struct {
    TokenType type;
    char lexeme[MAX_IDENT_LEN + 1];
    // int line; // Opcional: para melhor relatório de erros
    // int column; // Opcional: para melhor relatório de erros
} Token;

// Variável global para o token atual
extern Token current_token;


// Funções do Analisador Léxico (as suas já existentes, mas que precisarão ser modificadas)
// Elas agora não printarão diretamente, mas preencherão a struct Token.
// A função `getNextToken` será responsável por coordenar a chamada dessas funções.
int comentario(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace);

// Nova função para obter o próximo token e o seu tipo em string
void getNextToken(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace, int panicMode);
const char* getTokenTypeName(TokenType type);


#endif // LEXICO_H