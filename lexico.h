#ifndef LEXICO_H
#define LEXICO_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Definições de constantes para o analisador léxico
#define MAX_DEN 30
#define LEN_ERRO_LEXICO 50
#define ENCONTRADO 5
#define NAO_ENCONTRADO 2
#define FIM_DE_ARQUIVO 3
#define MAX_IDENT_LEN 100

// Tipos de tokens reconhecidos pelo analisador léxico
typedef enum {
    TOKEN_IDENT,
    TOKEN_NUMERO_INT, // Para números inteiros
    TOKEN_NUMERO_REAL, // Para números reais
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
    TOKEN_READ,         // Novo: read
    TOKEN_WRITE,        // Novo: write
    TOKEN_FOR,          // Novo: for
    TOKEN_TO,           // Novo: to
    TOKEN_REAL,         // Novo: real
    TOKEN_INTEGER,      // Novo: integer
    TOKEN_PROGRAM,      // Novo: program
    TOKEN_ELSE,         // NOVO: else
    TOKEN_IGUAL,          // =
    TOKEN_DIFERENTE,      // <>
    TOKEN_MENOR,          // <
    TOKEN_MENOR_IGUAL,    // <=
    TOKEN_MAIOR,          // >
    TOKEN_MAIOR_IGUAL,    // >=
    TOKEN_ATRIBUICAO,     // :=
    TOKEN_MAIS,           // +
    TOKEN_MENOS,          // -
    TOKEN_MULTIPLICACAO,  // *
    TOKEN_DIVISAO,        // /
    TOKEN_PARENTESES_ESQ, // (
    TOKEN_PARENTESES_DIR, // )
    TOKEN_VIRGULA,        // ,
    TOKEN_PONTO_E_VIRGULA,// ;
    TOKEN_PONTO,          // .
    TOKEN_DOIS_PONTOS,    // : // Usado para tipos, e não mais para atribuicao sozinha
    TOKEN_COMENTARIO,     // { ... }
    TOKEN_EOF,            // Fim de arquivo
    TOKEN_ERRO_LEXICO     // Erro léxico (caractere não reconhecido ou token malformado)
} TipoToken;

// Estrutura que representa um token
typedef struct {
    TipoToken tipo;
    char lexema[MAX_IDENT_LEN + 1]; // O texto do token
    int valor_numerico_int;         // Para tokens numéricos inteiros
    double valor_numerico_real;     // Para tokens numéricos reais
} Token;

// Variáveis globais para o analisador léxico (acessíveis externamente)
extern FILE* g_textFile;
extern FILE* g_textSaida;
extern Token g_currentToken; // O token atual lido pelo lexer, usado pelo parser

// Função para inicializar o analisador léxico com os arquivos de entrada e saída
void inicializarLexico(FILE* inputFile, FILE* outputFile);

// Função principal do analisador léxico que lê e retorna o próximo token
// Atualiza a variável global g_currentToken
void proximoToken();

// Funções auxiliares do analisador léxico (prefixadas com '_' para indicar uso interno)
// Cada função tenta reconhecer um tipo específico de token e preenche a estrutura Token.
// Retorna ENCONTRADO, NAO_ENCONTRADO ou FIM_DE_ARQUIVO.
int _comentario(Token* token);
int _numero(Token* token); // Vai tratar int e real
int _identificador(Token* token);
int _relacional(Token* token);
int _atribuicao(Token* token);
int _operadorMaisMenos(Token* token);
int _operadorPontuacao(Token* token);
int _operadorDivMult(Token* token);
int _ParentesesDireito(Token* token);
int _ParentesesEsquerdo(Token* token);

#endif // LEXICO_H
