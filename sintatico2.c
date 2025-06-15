#include "sintatico.h"
#include <stdarg.h> // Necessário para va_list, va_start, va_arg, va_end

// Variáveis globais
FILE* global_TextFile;
FILE* global_TextSaida;
int* global_boolErro;
int* global_boolSpace;
int panicMode = 0;

// Funções auxiliares e de tratamento de erro (mantidas como antes)
void panic_mode_recover(TokenType expected_type_for_context, ...);

// A declaração de current_token precisa estar disponível aqui.
// Assumindo que current_token está definida em lexico.h e extern em sintatico.h
// Se não estiver, você precisará adicionar 'extern Token current_token;' em sintatico.h
// ou, se for um arquivo separado, 'Token current_token;' no topo de sintatico.c
// Se já estiver sendo importada via sintatico.h, não precisa declarar aqui.

void match(TokenType expected_type, const char* error_message, ...) {
    if (current_token.type == expected_type) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace);
    } else if (!panicMode){
        fprintf(global_TextSaida, "Erro sintático: %s. Esperava '%s', encontrou '%s' ('%s').\n",
                error_message, getTokenTypeName(expected_type), current_token.lexeme, getTokenTypeName(current_token.type));
        *global_boolErro = 1;

        va_list args;
        va_start(args, error_message);
        panic_mode_recover(expected_type, va_arg(args, TokenType), TOKEN_EOF);
        va_end(args);

        // Após a recuperação, se o token atual é o esperado, consuma-o.
        if (current_token.type == expected_type) {
            getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace);
        }
    } 
}

void panic_mode_recover(TokenType expected_type_for_context, ...) {
    va_list args;
    TokenType sync_token_type;

    TokenType global_sync_tokens[] = {
        TOKEN_BEGIN, TOKEN_VAR, TOKEN_CONST, TOKEN_PROCEDURE,
        TOKEN_IF, TOKEN_THEN, TOKEN_WHILE, TOKEN_DO, TOKEN_CALL, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF
    };
    int num_global_sync_tokens = sizeof(global_sync_tokens) / sizeof(TokenType);

    int found_sync = 0;

    while (current_token.type != TOKEN_EOF && !found_sync) {
        if (current_token.type == expected_type_for_context) { // Sincroniza com o token esperado no contexto
            found_sync = 1;
            break;
        }

        for (int i = 0; i < num_global_sync_tokens; ++i) { // Procura tokens de sincronização
            if (current_token.type == global_sync_tokens[i]) {
                found_sync = 1;
                fprintf(global_TextSaida, "L --> Achei um token de sincronizacao! '%s' ('%s').\n",
                current_token.lexeme, getTokenTypeName(current_token.type));
                panicMode = 1;

                break;
            }
        }

        if (!found_sync) { // Sincroniza com tokens específicos passados como argumento
            va_start(args, expected_type_for_context);
            while ((sync_token_type = va_arg(args, TokenType)) != TOKEN_EOF) {
                if (current_token.type == sync_token_type) {
                    found_sync = 1;
                    break;
                }
            }
            va_end(args);
        }

        if (!found_sync) {
            fprintf(global_TextSaida, "Ignorando token inesperado para recuperacao de erro: '%s' ('%s')\n",
                    current_token.lexeme, getTokenTypeName(current_token.type));
            getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace);
        }
    }
}


// --- Funções do Analisador Sintático para cada Não-Terminal (Atualizadas conforme a gramática da imagem) ---

// Protótipos para funções que se chamam mutuamente ou são chamadas posteriormente
void mais_cmd();
void declaracao();
void expressao();
void condicao();
void termo();
void fator();
void relacao();
void operador_unario();
void mais_termos();
void mais_fatores();
void declaracao_const();
void mais_const();
void declaracao_var();
void mais_var();
void declaracao_proc();
void comando();


// <programa> ::= <bloco> .
void programa() {
    bloco();
    match(TOKEN_PERIOD, "Faltando '.' no final do programa", TOKEN_EOF);
    if (*global_boolErro == 0) {
        fprintf(global_TextSaida, "Compilacao bem-sucedida!\n");
    } else {
        fprintf(global_TextSaida, "Compilacao falhou com erros sintaticos ou lexicos.\n");
    }
}

// <bloco> ::= <declaracao> <comando>
void bloco() {
    declaracao();
    comando();
}

// <declaracao> ::= <constante> <variavel> <procedimento>
void declaracao() {
    declaracao_const();
    declaracao_var();
    declaracao_proc();
}

// <constante> ::= CONST ident = numero <mais_const> ; | λ
void declaracao_const() {
    if (current_token.type == TOKEN_CONST) {
        match(TOKEN_CONST, "Esperava 'CONST'",
              TOKEN_IDENT,
              TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador apos 'CONST'",
              TOKEN_EQ,
              TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_EQ, "Esperava '=' apos o identificador na declaracao de constante",
              TOKEN_NUMERO,
              TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_NUMERO, "Esperava um numero apos '=' na declaracao de constante",
              TOKEN_SEMICOLON, TOKEN_COMMA,
              TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        mais_const();
        match(TOKEN_SEMICOLON, "Esperava ';' apos a declaracao de constante",
              TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
    }
}

// <mais_const> ::= , ident = numero <mais_const> | λ
void mais_const() {
    if (current_token.type == TOKEN_COMMA) {
        match(TOKEN_COMMA, "Esperava ',' na lista de constantes",
              TOKEN_IDENT,
              TOKEN_SEMICOLON, TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador apos ',' na lista de constantes",
              TOKEN_EQ,
              TOKEN_SEMICOLON, TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_EQ, "Esperava '=' apos o identificador na lista de constantes",
              TOKEN_NUMERO,
              TOKEN_SEMICOLON, TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_NUMERO, "Esperava um numero apos '=' na lista de constantes",
              TOKEN_SEMICOLON, TOKEN_COMMA,
              TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        mais_const();
    }
}

// <variavel> ::= VAR ident <mais_var> ; | λ
void declaracao_var() {
    if (current_token.type == TOKEN_VAR) {
        match(TOKEN_VAR, "Esperava 'VAR'",
              TOKEN_IDENT,
              TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador apos 'VAR'",
              TOKEN_SEMICOLON, TOKEN_COMMA,
              TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        mais_var();
        match(TOKEN_SEMICOLON, "Esperava ';' apos a declaracao de variavel",
              TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
    }
}

// <mais_var> ::= , ident <mais_var> | λ
void mais_var() {
    if (current_token.type == TOKEN_COMMA) {
        match(TOKEN_COMMA, "Esperava ',' na lista de variaveis",
              TOKEN_IDENT,
              TOKEN_SEMICOLON, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador apos ',' na lista de variaveis",
              TOKEN_SEMICOLON, TOKEN_COMMA,
              TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        mais_var();
    }
}

// <procedimento> ::= PROCEDURE ident ; <bloco> ; <procedimento> | λ
void declaracao_proc() {
    if (current_token.type == TOKEN_PROCEDURE) {
        match(TOKEN_PROCEDURE, "Esperava 'PROCEDURE'",
              TOKEN_IDENT,
              TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador apos 'PROCEDURE'",
              TOKEN_SEMICOLON,
              TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_SEMICOLON, "Esperava ';' apos o nome do procedimento",
              TOKEN_CONST, TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        bloco();
        match(TOKEN_SEMICOLON, "Esperava ';' apos o bloco do procedimento",
              TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        declaracao_proc();
    }
}

// <comando> ::= ident := <expressao>
//             | CALL ident
//             | BEGIN <comando> <mais_cmd> END
//             | IF <condicao> THEN <comando>
//             | WHILE <condicao> DO <comando>
//             | λ
void comando() {
    if (current_token.type == TOKEN_IDENT) {
        match(TOKEN_IDENT, "Esperava um identificador para atribuicao",
              TOKEN_ASSIGN,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_ASSIGN, "Esperava ':=' para atribuicao",
              TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_ODD,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        expressao();
    } else if (current_token.type == TOKEN_CALL) {
        match(TOKEN_CALL, "Esperava 'CALL'",
              TOKEN_IDENT,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador apos 'CALL'",
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
    } else if (current_token.type == TOKEN_BEGIN) {
        match(TOKEN_BEGIN, "Esperava 'BEGIN'",
              TOKEN_IDENT, TOKEN_CALL, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE,
              TOKEN_END, TOKEN_SEMICOLON, TOKEN_PERIOD, TOKEN_EOF);
        comando();
        mais_cmd();
        match(TOKEN_END, "Esperava 'END' apos a lista de comandos",
              TOKEN_SEMICOLON, TOKEN_PERIOD, TOKEN_EOF);
    } else if (current_token.type == TOKEN_IF) {
        match(TOKEN_IF, "Esperava 'IF'",
              TOKEN_ODD, TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_THEN, TOKEN_DO,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        condicao();
        match(TOKEN_THEN, "Esperava 'THEN' apos a condicao do IF",
              TOKEN_IDENT, TOKEN_CALL, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        comando();
    } else if (current_token.type == TOKEN_WHILE) {
        match(TOKEN_WHILE, "Esperava 'WHILE'",
              TOKEN_ODD, TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_THEN, TOKEN_DO,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        condicao();
        match(TOKEN_DO, "Esperava 'DO' apos a condicao do WHILE",
              TOKEN_IDENT, TOKEN_CALL, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        comando();
    } //else {
        // Produção lambda (λ) para comando:
       /* if (!(current_token.type == TOKEN_SEMICOLON || current_token.type == TOKEN_END ||
              current_token.type == TOKEN_PERIOD || current_token.type == TOKEN_EOF)) {
             fprintf(global_TextSaida, "Erro sintatico: Comando inesperado. Encontrou '%s' ('%s').\n",
                    current_token.lexeme, getTokenTypeName(current_token.type));
             *global_boolErro = 1;
             panic_mode_recover(current_token.type, TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);*/
     //   }
   // }
}

// <mais_cmd> ::= ; <comando> <mais_cmd> | λ
void mais_cmd() {
    if (current_token.type == TOKEN_SEMICOLON) {
        match(TOKEN_SEMICOLON, "Esperava ';' antes do proximo comando",
              TOKEN_IDENT, TOKEN_CALL, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE,
              TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        comando();
        mais_cmd();
    }
}

// <expressao> ::= <operador_unario> <termo> <mais_termos>
void expressao() {
    operador_unario();
    termo();
    mais_termos();
}

// <operador_unario> ::= - | + | λ
void operador_unario() {
    if (current_token.type == TOKEN_MINUS) {
        match(TOKEN_MINUS, "Esperava '-' para operador unario",
              TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // Usando TOKEN_LE, TOKEN_GE
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
    } else if (current_token.type == TOKEN_PLUS) {
        match(TOKEN_PLUS, "Esperava '+' para operador unario",
              TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // Usando TOKEN_LE, TOKEN_GE
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
    }
}

// <termo> ::= <fator> <mais_fatores>
void termo() {
    fator();
    mais_fatores();
}

// <mais_termos> ::= - <termo> <mais_termos> | + <termo> <mais_termos> | λ
void mais_termos() {
    if (current_token.type == TOKEN_MINUS || current_token.type == TOKEN_PLUS) {
        TokenType op_type = current_token.type;
        match(op_type, "Esperava '+' ou '-' em mais_termos",
              TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // Usando TOKEN_LE, TOKEN_GE
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        termo();
        mais_termos();
    }
}

// <fator> ::= ident | numero | ( <expressao> )
void fator() {
    if (current_token.type == TOKEN_IDENT) {
        match(TOKEN_IDENT, "Esperava um identificador",
              TOKEN_MULTIPLY, TOKEN_DIVIDE, // Usando TOKEN_MULTIPLY, TOKEN_DIVIDE
              TOKEN_PLUS, TOKEN_MINUS,
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // Usando TOKEN_LE, TOKEN_GE
              TOKEN_RPAREN,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
    } else if (current_token.type == TOKEN_NUMERO) {
        match(TOKEN_NUMERO, "Esperava um numero",
              TOKEN_MULTIPLY, TOKEN_DIVIDE, // Usando TOKEN_MULTIPLY, TOKEN_DIVIDE
              TOKEN_PLUS, TOKEN_MINUS,
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // Usando TOKEN_LE, TOKEN_GE
              TOKEN_RPAREN,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
    } else if (current_token.type == TOKEN_LPAREN) {
        match(TOKEN_LPAREN, "Esperava '('",
              TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        expressao();
        match(TOKEN_RPAREN, "Esperava ')'",
              TOKEN_MULTIPLY, TOKEN_DIVIDE, // Usando TOKEN_MULTIPLY, TOKEN_DIVIDE
              TOKEN_PLUS, TOKEN_MINUS,
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // Usando TOKEN_LE, TOKEN_GE
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
    } else {
        fprintf(global_TextSaida, "Erro sintatico: Fator esperado. Encontrou '%s' ('%s').\n",
                current_token.lexeme, getTokenTypeName(current_token.type));
        *global_boolErro = 1;
        panic_mode_recover(current_token.type,
                           TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_PLUS, TOKEN_MINUS, // Usando TOKEN_MULTIPLY, TOKEN_DIVIDE
                           TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // Usando TOKEN_LE, TOKEN_GE
                           TOKEN_RPAREN, TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
    }
}

// <mais_fatores> ::= * <fator> <mais_fatores> | / <fator> <mais_fatores> | λ
void mais_fatores() {
    if (current_token.type == TOKEN_MULTIPLY || current_token.type == TOKEN_DIVIDE) { // Usando TOKEN_MULTIPLY, TOKEN_DIVIDE
        TokenType op_type = current_token.type;
        match(op_type, "Esperava '*' ou '/' em mais_fatores",
              TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_PLUS, TOKEN_MINUS,
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // Usando TOKEN_LE, TOKEN_GE
              TOKEN_RPAREN,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        fator();
        mais_fatores();
    }
}

// <condicao> ::= ODD <expressao> | <expressao> <relacional> <expressao>
void condicao() {
    if (current_token.type == TOKEN_ODD) {
        match(TOKEN_ODD, "Esperava 'ODD'",
              TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_THEN, TOKEN_DO,
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
        expressao();
    } else {
        expressao();
        relacao();
        expressao();
    }
}

// <relacional> ::= = | <> | < | <= | > | >=
void relacao() {
    if (current_token.type == TOKEN_EQ) {
        match(TOKEN_EQ, "Esperava '='",
              TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_THEN, TOKEN_DO,
              TOKEN_SEMICOLON
              //, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF
              );
    } else if (current_token.type == TOKEN_NEQ) {
        match(TOKEN_NEQ, "Esperava '<>'",
              TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_THEN, TOKEN_DO,
              TOKEN_SEMICOLON
              //, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF
              );
    } else if (current_token.type == TOKEN_LT) {
        match(TOKEN_LT, "Esperava '<'",
              TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_THEN, TOKEN_DO,
              TOKEN_SEMICOLON
              //, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF
              );
    } else if (current_token.type == TOKEN_LE) { // Usando TOKEN_LE
        match(TOKEN_LE, "Esperava '<='",
              TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_THEN, TOKEN_DO,
              TOKEN_SEMICOLON
              //, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF
              );
    } else if (current_token.type == TOKEN_GT) {
        match(TOKEN_GT, "Esperava '>'",
              TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
              TOKEN_THEN, TOKEN_DO,
              TOKEN_SEMICOLON
              //, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF
              );
    } else if (current_token.type == TOKEN_GE) { // Usando TOKEN_GE
            match(TOKEN_GE, "Esperava '>='",
                TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
                TOKEN_THEN, TOKEN_DO,
                TOKEN_SEMICOLON
                //, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF
                );
    } else {
        fprintf(global_TextSaida, "Erro sintatico: Operador relacional esperado. Encontrou '%s' ('%s').\n",
                current_token.lexeme, getTokenTypeName(current_token.type));
        *global_boolErro = 1;
        // Modificado L
        panic_mode_recover(current_token.type,TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT,
        TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_THEN, TOKEN_DO, TOKEN_SEMICOLON);
    }
}

// Função para iniciar a análise sintática
void iniciarAnaliseSintatica(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
    global_TextFile = textFile;
    global_TextSaida = textSaida;
    global_boolErro = boolErro;
    global_boolSpace = boolSpace;

    getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace);

    programa();

    if (current_token.type != TOKEN_EOF) {
        fprintf(global_TextSaida, "Erro sintatico: Caracteres inesperados apos o final do programa. Encontrou '%s' ('%s').\n",
                current_token.lexeme, getTokenTypeName(current_token.type));
        *global_boolErro = 1;
    }
}