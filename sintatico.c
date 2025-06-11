// sintatico.c

#include "sintatico.h"
#include <stdarg.h> // Para varargs, útil para o modo pânico

// Variáveis globais para o parser (inicializadas em iniciarAnaliseSintatica)
FILE* global_TextFile;
FILE* global_TextSaida;
int* global_boolErro;
int* global_boolSpace;
// current_token é externa, definida em lexico.c e declarada em lexico.h

// --- Declarações de funções auxiliares e de tratamento de erro ---
// Necessário para que match() possa chamar panic_mode_recover()
void panic_mode_recover(TokenType expected_type, ...);

// --- Funções do Analisador Sintático para cada Não-Terminal (protótipos) ---
// (Já declaradas em sintatico.h, mas podem ser repetidas aqui como forward declarations
// para clareza se as implementações estiverem fora de ordem)
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


// --- Funções Auxiliares e de Tratamento de Erro ---

// Função para avançar o token se o tipo esperado for encontrado.
// Caso contrário, reporta um erro sintático e tenta recuperar.
void match(TokenType expected_type, const char* error_message, ...) {
    if (current_token.type == expected_type) {
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace);
    } else {
        fprintf(global_TextSaida, "Erro sintático: %s. Esperava '%s', encontrou '%s' ('%s').\n",
                error_message, getTokenTypeName(expected_type), current_token.lexeme, getTokenTypeName(current_token.type));
        *global_boolErro = 1; // Marca que houve erro sintático

        // Modo Pânico: Tenta sincronizar.
        // Coleta os tokens de sincronização passados como argumentos variáveis
        va_list args;
        va_start(args, error_message); // args começa após error_message
        // panic_mode_recover deve ser chamada com a lista de tokens de sincronização, terminando com TOKEN_EOF.
        panic_mode_recover(expected_type, va_arg(args, TokenType), TOKEN_EOF); // O primeiro va_arg será o primeiro sync token
        va_end(args);

        // Após a recuperação, se o token atual é o esperado, consuma-o.
        // Isso lida com o caso em que o erro foi apenas um "token extra" que foi pulado.
        if (current_token.type == expected_type) {
            getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace);
        }
    }
}

// Conjuntos de sincronização para o modo pânico
// A função panic_mode_recover agora recebe uma lista de tokens de sincronização via varargs.
// O último token na lista deve ser TOKEN_EOF para indicar o fim.
void panic_mode_recover(TokenType expected_type_for_context, ...) { // Renomeado para evitar conflito com match
    va_list args;
    TokenType sync_token_type;

    // Tokens de sincronização globais (sempre válidos para recuperação)
    // Estes são tokens que podem iniciar uma nova estrutura principal no PL/0.
    TokenType global_sync_tokens[] = {
        TOKEN_SEMICOLON, TOKEN_BEGIN, TOKEN_VAR, TOKEN_CONST, TOKEN_PROCEDURE,
        TOKEN_IF, TOKEN_THEN, TOKEN_WHILE, TOKEN_DO, TOKEN_CALL, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF
    };
    int num_global_sync_tokens = sizeof(global_sync_tokens) / sizeof(TokenType);

    int found_sync = 0;

    while (current_token.type != TOKEN_EOF && !found_sync) {
        // 1. Verifica se o token atual é o tipo esperado para o contexto da chamada `match`
        if (current_token.type == expected_type_for_context) {
            found_sync = 1;
            break;
        }

        // 2. Verifica os tokens de sincronização globais
        for (int i = 0; i < num_global_sync_tokens; ++i) {
            if (current_token.type == global_sync_tokens[i]) {
                found_sync = 1;
                break;
            }
        }

        // 3. Verifica os tokens de sincronização passados como argumento (específicos da regra)
        if (!found_sync) {
            va_start(args, expected_type_for_context); // Inicia args após o primeiro parâmetro (expected_type_for_context)
            while ((sync_token_type = va_arg(args, TokenType)) != TOKEN_EOF) { // TOKEN_EOF é o sentinela
                if (current_token.type == sync_token_type) {
                    found_sync = 1;
                    break;
                }
            }
            va_end(args);
        }

        // Se ainda não encontrou um token de sincronização, avança para o próximo token.
        if (!found_sync) {
            fprintf(global_TextSaida, "Ignorando token inesperado para recuperação de erro: '%s' ('%s')\n",
                    current_token.lexeme, getTokenTypeName(current_token.type));
            getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace);
        }
    }
    // Ao sair deste loop, current_token é um token de sincronização (ou EOF).
    // A função match() ou a regra gramatical chamadora decidirá se o consome ou não.
}

// --- Funções do Analisador Sintático para cada Não-Terminal ---

// <programa> ::= <bloco> .
void programa() {
    bloco();
    match(TOKEN_PERIOD, "Faltando '.' no final do programa", TOKEN_EOF); // FOLLOW(programa) = {EOF}
    if (*global_boolErro == 0) {
        fprintf(global_TextSaida, "Compilação bem-sucedida!\n");
    } else {
        fprintf(global_TextSaida, "Compilação falhou com erros sintáticos ou léxicos.\n");
    }
}

// <bloco> ::= <declaracao_const> <declaracao_var> <declaracao_proc> <comando>
void bloco() {
    declaracao_const();
    declaracao_var();
    declaracao_proc();
    comando();
}

// <declaracao_const> ::= CONST ident = numero ; <lista_const> | λ
void declaracao_const() {
    if (current_token.type == TOKEN_CONST) {
        match(TOKEN_CONST, "Esperava 'CONST'", TOKEN_IDENT, TOKEN_EOF); // FOLLOW(CONST) = {IDENT, VAR, PROCEDURE, BEGIN, IF, WHILE, CALL, EOF}
        match(TOKEN_IDENT, "Esperava um identificador após 'CONST'", TOKEN_EQ, TOKEN_EOF); // FOLLOW(IDENT) = {=, ,}
        match(TOKEN_EQ, "Esperava '=' após o identificador na declaração de constante", TOKEN_NUMERO, TOKEN_EOF);
        match(TOKEN_NUMERO, "Esperava um número após '=' na declaração de constante", TOKEN_SEMICOLON, TOKEN_EOF);
        match(TOKEN_SEMICOLON, "Esperava ';' após a declaração de constante", TOKEN_COMMA, TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL, TOKEN_EOF);
        lista_const();
    }
    // Produção lambda se não começar com CONST
}

// <lista_const> ::= , ident = numero ; <lista_const> | λ
void lista_const() {
    if (current_token.type == TOKEN_COMMA) {
        match(TOKEN_COMMA, "Esperava ',' na lista de constantes", TOKEN_IDENT, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador após ',' na lista de constantes", TOKEN_EQ, TOKEN_EOF);
        match(TOKEN_EQ, "Esperava '=' após o identificador na lista de constantes", TOKEN_NUMERO, TOKEN_EOF);
        match(TOKEN_NUMERO, "Esperava um número após '=' na lista de constantes", TOKEN_SEMICOLON, TOKEN_EOF);
        match(TOKEN_SEMICOLON, "Esperava ';' após a declaração de constante na lista", TOKEN_COMMA, TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL, TOKEN_EOF);
        lista_const();
    }
    // Produção lambda
}

// <declaracao_var> ::= VAR ident <lista_var> ; | λ
void declaracao_var() {
    if (current_token.type == TOKEN_VAR) {
        match(TOKEN_VAR, "Esperava 'VAR'", TOKEN_IDENT, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador após 'VAR'", TOKEN_SEMICOLON, TOKEN_COMMA, TOKEN_EOF);
        lista_var();
        match(TOKEN_SEMICOLON, "Esperava ';' após a declaração de variável", TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL, TOKEN_EOF);
    }
    // Produção lambda
}

// <lista_var> ::= , ident <lista_var> | λ
void lista_var() {
    if (current_token.type == TOKEN_COMMA) {
        match(TOKEN_COMMA, "Esperava ',' na lista de variáveis", TOKEN_IDENT, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador após ',' na lista de variáveis", TOKEN_SEMICOLON, TOKEN_COMMA, TOKEN_EOF);
        lista_var();
    }
    // Produção lambda
}

// <declaracao_proc> ::= PROCEDURE ident ; <bloco> ; <declaracao_proc> | λ
void declaracao_proc() {
    if (current_token.type == TOKEN_PROCEDURE) {
        match(TOKEN_PROCEDURE, "Esperava 'PROCEDURE'", TOKEN_IDENT, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador após 'PROCEDURE'", TOKEN_SEMICOLON, TOKEN_EOF);
        match(TOKEN_SEMICOLON, "Esperava ';' após o nome do procedimento", TOKEN_CONST, TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL, TOKEN_EOF); // FIRST de bloco e FOLLOW de PROCEDURE
        bloco();
        match(TOKEN_SEMICOLON, "Esperava ';' após o bloco do procedimento", TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF); // FOLLOW de PROCEDURE
        declaracao_proc();
    }
    // Produção lambda
}

// <comando> ::= ident := <expressao>
//             | CALL ident
//             | BEGIN <lista_comando> END
//             | IF <condicao> THEN <comando>
//             | WHILE <condicao> DO <comando>
//             | λ
void comando() {
    if (current_token.type == TOKEN_IDENT) {
        match(TOKEN_IDENT, "Esperava um identificador para atribuição", TOKEN_ASSIGN, TOKEN_EOF);
        match(TOKEN_ASSIGN, "Esperava ':=' para atribuição", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF); // FIRST de expressao
    } else if (current_token.type == TOKEN_CALL) {
        match(TOKEN_CALL, "Esperava 'CALL'", TOKEN_IDENT, TOKEN_EOF);
        match(TOKEN_IDENT, "Esperava um identificador após 'CALL'", TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF); // FOLLOW de comando
    } else if (current_token.type == TOKEN_BEGIN) {
        match(TOKEN_BEGIN, "Esperava 'BEGIN'", TOKEN_IDENT, TOKEN_CALL, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_END, TOKEN_SEMICOLON, TOKEN_EOF); // FIRST de comando e FOLLOW de lista_comando
        lista_comando();
        match(TOKEN_END, "Esperava 'END' após a lista de comandos", TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF); // FOLLOW de comando
    } else if (current_token.type == TOKEN_IF) {
        match(TOKEN_IF, "Esperava 'IF'", TOKEN_ODD, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF); // FIRST de condicao
        condicao();
        match(TOKEN_THEN, "Esperava 'THEN' após a condição do IF", TOKEN_IDENT, TOKEN_CALL, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF); // FIRST de comando e FOLLOW de comando
        comando();
    } else if (current_token.type == TOKEN_WHILE) {
        match(TOKEN_WHILE, "Esperava 'WHILE'", TOKEN_ODD, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF); // FIRST de condicao
        condicao();
        match(TOKEN_DO, "Esperava 'DO' após a condição do WHILE", TOKEN_IDENT, TOKEN_CALL, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF); // FIRST de comando e FOLLOW de comando
        comando();
    }
    // Produção lambda se nenhum dos tokens acima for encontrado
}

// <lista_comando> ::= <comando> { ; <comando> } | λ
void lista_comando() {
    comando();
    while (current_token.type == TOKEN_SEMICOLON) {
        match(TOKEN_SEMICOLON, "Esperava ';' entre comandos", TOKEN_IDENT, TOKEN_CALL, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF); // FIRST de comando e FOLLOW de lista_comando
        comando();
    }
    // Produção lambda se não houver mais ';'
}

// <expressao> ::= <sinal> <termo> <lista_termo>
void expressao() {
    sinal();
    termo();
    lista_termo();
}

// <sinal> ::= - | + | λ
void sinal() {
    if (current_token.type == TOKEN_PLUS) {
        match(TOKEN_PLUS, "Esperava '+'", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_EOF); // FIRST de termo
    } else if (current_token.type == TOKEN_MINUS) {
        match(TOKEN_MINUS, "Esperava '-'", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_EOF); // FIRST de termo
    }
    // Produção lambda
}

// <lista_termo> ::= + <termo> <lista_termo> | - <termo> <lista_termo> | λ
void lista_termo() {
    if (current_token.type == TOKEN_PLUS || current_token.type == TOKEN_MINUS) {
        if (current_token.type == TOKEN_PLUS) {
            match(TOKEN_PLUS, "Esperava '+'", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_EOF); // FIRST de termo
        } else {
            match(TOKEN_MINUS, "Esperava '-'", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_EOF); // FIRST de termo
        }
        termo();
        lista_termo();
    }
    // Produção lambda
}

// <termo> ::= <fator> <lista_fator>
void termo() {
    fator();
    lista_fator();
}

// <lista_fator> ::= * <fator> <lista_fator> | / <fator> <lista_fator> | λ
void lista_fator() {
    if (current_token.type == TOKEN_MULTIPLY || current_token.type == TOKEN_DIVIDE) {
        if (current_token.type == TOKEN_MULTIPLY) {
            match(TOKEN_MULTIPLY, "Esperava '*'", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_EOF); // FIRST de fator
        } else {
            match(TOKEN_DIVIDE, "Esperava '/'", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_EOF); // FIRST de fator
        }
        fator();
        lista_fator();
    }
    // Produção lambda
}

// <fator> ::= ident | numero | ( <expressao> )
void fator() {
    if (current_token.type == TOKEN_IDENT) {
        match(TOKEN_IDENT, "Esperava um identificador",
              TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_PLUS, TOKEN_MINUS, // FOLLOW(fator)
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // FOLLOW(expressao) via condicao
              TOKEN_SEMICOLON, TOKEN_RPAREN, TOKEN_THEN, TOKEN_DO, TOKEN_END, TOKEN_PERIOD, TOKEN_COMMA, TOKEN_EOF); // FOLLOW(expressao) e comandos
    } else if (current_token.type == TOKEN_NUMERO) {
        match(TOKEN_NUMERO, "Esperava um número",
              TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_PLUS, TOKEN_MINUS, // FOLLOW(fator)
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // FOLLOW(expressao) via condicao
              TOKEN_SEMICOLON, TOKEN_RPAREN, TOKEN_THEN, TOKEN_DO, TOKEN_END, TOKEN_PERIOD, TOKEN_COMMA, TOKEN_EOF);
    } else if (current_token.type == TOKEN_LPAREN) {
        match(TOKEN_LPAREN, "Esperava '('", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF); // FIRST de expressao
        expressao();
        match(TOKEN_RPAREN, "Esperava ')'",
              TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_PLUS, TOKEN_MINUS, // FOLLOW(fator)
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // FOLLOW(expressao) via condicao
              TOKEN_SEMICOLON, TOKEN_RPAREN, TOKEN_THEN, TOKEN_DO, TOKEN_END, TOKEN_PERIOD, TOKEN_COMMA, TOKEN_EOF);
    } else {
        // Erro: Fator inesperado
        fprintf(global_TextSaida, "Erro sintático: Esperava identificador, número ou '('. Encontrou '%s' ('%s').\n",
                current_token.lexeme, getTokenTypeName(current_token.type));
        *global_boolErro = 1;
        // Modo pânico: tentar sincronizar com o FOLLOW set de <fator>
        panic_mode_recover(current_token.type, // O tipo atual é o que estamos tentando sincronizar
                           TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_PLUS, TOKEN_MINUS,
                           TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE,
                           TOKEN_SEMICOLON, TOKEN_RPAREN, TOKEN_THEN, TOKEN_DO, TOKEN_END, TOKEN_PERIOD,
                           TOKEN_COMMA, TOKEN_EOF);
    }
}

// <condicao> ::= ODD <expressao> | <expressao> <relacao> <expressao>
void condicao() {
    // FOLLOW(condicao) = {THEN, DO}
    if (current_token.type == TOKEN_ODD) {
        match(TOKEN_ODD, "Esperava 'ODD'", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF); // FIRST de expressao
        expressao();
    } else {
        expressao();
        relacao();
        expressao();
    }
}

// <relacao> ::= = | <> | < | <= | > | >=
void relacao() {
    // FOLLOW(relacao) = FIRST(expressao) = {+, -, ident, numero, (}
    if (current_token.type == TOKEN_EQ) {
        match(TOKEN_EQ, "Esperava '='", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF); // FIRST de expressao
    } else if (current_token.type == TOKEN_NEQ) {
        match(TOKEN_NEQ, "Esperava '<>'", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF);
    } else if (current_token.type == TOKEN_LT) {
        match(TOKEN_LT, "Esperava '<'", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF);
    } else if (current_token.type == TOKEN_LE) {
        match(TOKEN_LE, "Esperava '<='", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF);
    } else if (current_token.type == TOKEN_GT) {
        match(TOKEN_GT, "Esperava '>'", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF);
    } else if (current_token.type == TOKEN_GE) {
        match(TOKEN_GE, "Esperava '>='", TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF);
    } else {
        // Erro: Operador relacional inesperado
        fprintf(global_TextSaida, "Erro sintático: Esperava operador relacional. Encontrou '%s' ('%s').\n",
                current_token.lexeme, getTokenTypeName(current_token.type));
        *global_boolErro = 1;
        // Modo pânico: tentar sincronizar com o FOLLOW set de <relacao>
        panic_mode_recover(current_token.type, // O tipo atual é o que estamos tentando sincronizar
                           TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN, TOKEN_MINUS, TOKEN_PLUS, TOKEN_EOF); // FIRST de expressao
    }
}

// Função principal para iniciar a análise sintática
void iniciarAnaliseSintatica(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
    global_TextFile = textFile;
    global_TextSaida = textSaida;
    global_boolErro = boolErro;
    global_boolSpace = boolSpace;

    // Obtém o primeiro token antes de iniciar o parsing
    getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace);

    // Inicia a análise a partir da regra inicial <programa>
    programa();

    // Verifica se há lixo após o ponto final do programa
    if (current_token.type != TOKEN_EOF) {
        fprintf(global_TextSaida, "Erro sintático: Caracteres inesperados após o final do programa. Encontrou '%s' ('%s').\n",
                current_token.lexeme, getTokenTypeName(current_token.type));
        *global_boolErro = 1;
    }
}