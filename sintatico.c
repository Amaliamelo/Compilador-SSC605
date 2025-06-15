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
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        panicMode = 0;
    } else if (!panicMode){
        // Se o token já for um erro léxico, não imprima uma nova mensagem de erro sintático.
        if (current_token.type != TOKEN_ERROR) {
            fprintf(global_TextSaida, "Erro sintático: %s. Esperava '%s', encontrou '%s' ('%s').\n",
                    error_message, getTokenTypeName(expected_type), current_token.lexeme, getTokenTypeName(current_token.type));
        }
        *global_boolErro = 1;

        va_list args;
        va_start(args, error_message);
       // fprintf(global_TextSaida, "PANICOOO PANICOO!\n");
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        panic_mode_recover(expected_type, va_arg(args, TokenType), TOKEN_EOF);
        va_end(args);
        // Após a recuperação, se o token atual é o esperado, consuma-o.
        if (current_token.type == expected_type) {
            panicMode = 0;
            getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        }
    }
}

void panic_mode_recover(TokenType expected_type_for_context, ...) {
    va_list args;
    TokenType sync_token_type;

    TokenType global_sync_tokens[] = {
        TOKEN_BEGIN, TOKEN_CALL, TOKEN_IF,
        TOKEN_WHILE, TOKEN_PERIOD, TOKEN_EOF
    };

    int num_global_sync_tokens = sizeof(global_sync_tokens) / sizeof(TokenType);

    int found_sync = 0;

    while (current_token.type != TOKEN_EOF && !found_sync) {
        if (current_token.type == expected_type_for_context) { // Sincroniza com o token esperado no contexto
            found_sync = 1;
          // fprintf(global_TextSaida, "L --> Achei um token de sincronizacao! '%s' ('%s').\n",
         //   current_token.lexeme, getTokenTypeName(current_token.type));
          //  printf( "L --> Achei um token de sincronizacao! '%s' ('%s').\n", current_token.lexeme, getTokenTypeName(current_token.type));
            panicMode = 1;
            return;
        }

        if (!found_sync) { // Sincroniza com tokens específicos passados como argumento
            va_start(args, expected_type_for_context);
            while ((sync_token_type = va_arg(args, TokenType)) != TOKEN_EOF) {
                if (current_token.type == sync_token_type) {
                    found_sync = 1;
                //    fprintf(global_TextSaida, "L --> Achei um token de sincronizacao! '%s' ('%s').\n",
                //    current_token.lexeme, getTokenTypeName(current_token.type));
                    panicMode = 1;
                    return;
                }
            }
            va_end(args);
        }

        for (int i = 0; i < num_global_sync_tokens; ++i) { // Procura tokens de sincronização
            if (current_token.type == global_sync_tokens[i]) {
                found_sync = 1;
              //  fprintf(global_TextSaida, "L --> Achei um token de sincronizacao! '%s' ('%s').\n",
            //    current_token.lexeme, getTokenTypeName(current_token.type));
                panicMode = 1;
                return;
            }
        }

        if (!found_sync) {
            //fprintf(global_TextSaida, "Ignorando token inesperado para recuperacao de erro: '%s' ('%s')\n",
              //      current_token.lexeme, getTokenTypeName(current_token.type));
            getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
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
  //  fprintf(global_TextSaida, "DECLARAÇÃO\n");
    declaracao_const();
    declaracao_var();
    declaracao_proc();
    if (current_token.type == TOKEN_VAR || current_token.type == TOKEN_CONST || current_token.type == TOKEN_PROCEDURE) {
    fprintf(global_TextSaida, "Erro sintático: Declaracao fora de ordem ou inesperada. Esperava 'BEGIN' ou '.' (fim do programa), encontrou '%s' ('%s').\n",
            current_token.lexeme, getTokenTypeName(current_token.type));
    *global_boolErro = 1;

    panic_mode_recover(current_token.type,
                       TOKEN_BEGIN, TOKEN_PERIOD, TOKEN_EOF, // Tokens para onde queremos pular
                       TOKEN_IDENT, TOKEN_CALL, TOKEN_IF, TOKEN_WHILE // Outros FIRST(comando)
                       );
    }
 }

// <constante> ::= CONST ident = numero <mais_const> ; | λ
void declaracao_const() {
  //  fprintf(global_TextSaida, "DECLARAÇÃO CONST\n");
    if (current_token.type == TOKEN_CONST) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        match(TOKEN_IDENT, "Esperava um identificador apos 'CONST'", TOKEN_SEMICOLON, TOKEN_NUMERO, TOKEN_EQ);
        match(TOKEN_EQ, "Esperava '=' apos o identificador na declaracao de constante", TOKEN_SEMICOLON, TOKEN_NUMERO);
        match(TOKEN_NUMERO, "Esperava um numero apos '=' na declaracao de constante", TOKEN_SEMICOLON);
        mais_const();
        match(TOKEN_SEMICOLON, "Esperava ';' apos a declaracao de constante");
    }
}

// <mais_const> ::= , ident = numero <mais_const> | λ
void mais_const() {
    //fprintf(global_TextSaida, "MAIS_CONST\n");
    if (current_token.type == TOKEN_COMMA) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        match(TOKEN_IDENT, "Esperava um identificador apos ',' na lista de constantes",
              TOKEN_EQ, TOKEN_NUMERO);
        match(TOKEN_EQ, "Esperava '=' apos o identificador na lista de constantes",
              TOKEN_NUMERO);
        match(TOKEN_NUMERO, "Esperava um numero apos '=' na lista de constantes");
        mais_const();
    }
}

// <variavel> ::= VAR ident <mais_var> ; | λ
void declaracao_var() {
   // fprintf(global_TextSaida, "DECLARACAO_VAR\n");
    if (current_token.type == TOKEN_VAR) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        match(TOKEN_IDENT, "Esperava um identificador apos 'VAR'",
              TOKEN_SEMICOLON);
        mais_var();
        match(TOKEN_SEMICOLON, "Esperava ';' apos a declaracao de variavel");
    }
}

// <mais_var> ::= , ident <mais_var> | λ
void mais_var() {
   // fprintf(global_TextSaida, "MAIS_VAR\n");
    if (current_token.type == TOKEN_COMMA) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        match(TOKEN_IDENT, "Esperava um identificador apos ',' na lista de variaveis");
        mais_var();
    }
}

// <procedimento> ::= PROCEDURE ident ; <bloco> ; <procedimento> | λ
void declaracao_proc() {
    //fprintf(global_TextSaida, "DECLARACAO_PROC\n");
    if (current_token.type == TOKEN_PROCEDURE) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        match(TOKEN_IDENT, "Esperava um identificador apos 'PROCEDURE'",
              TOKEN_SEMICOLON);
        match(TOKEN_SEMICOLON, "Esperava ';' apos o nome do procedimento",
        TOKEN_SEMICOLON);
        bloco();
        match(TOKEN_SEMICOLON, "Esperava ';' apos o bloco do procedimento");
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

int notEntered = 1;

do {
  //  fprintf(global_TextSaida, "COMANDO\n");
 //   fprintf (global_TextSaida,"L --> current token = %s\n", current_token.lexeme);
    if (current_token.type == TOKEN_IDENT) {
        //printf("Espero :=, tenho %s \n", current_token.lexeme);
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        //printf("Espero :=, tenho %s \n", current_token.lexeme);
        match(TOKEN_ASSIGN, "Esperava ':=' para atribuicao",
              TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN);
        expressao();
    } else if (current_token.type == TOKEN_CALL) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        match(TOKEN_IDENT, "Esperava um identificador apos 'CALL'");
    } else if (current_token.type == TOKEN_BEGIN) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        comando();
        mais_cmd();
        match(TOKEN_END, "Esperava 'END' apos a lista de comandos");
    } else if (current_token.type == TOKEN_IF) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        condicao();
        match(TOKEN_THEN, "Esperava 'THEN' apos a condicao do IF");
        comando();
    } else if (current_token.type == TOKEN_WHILE) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        condicao();
        match(TOKEN_DO, "Esperava 'DO' apos a condicao do WHILE");
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
    } while (panicMode && notEntered--);
}

// <mais_cmd> ::= ; <comando> <mais_cmd> | λ
void mais_cmd() {
    int notEntered = 1;
  //  fprintf(global_TextSaida, "MAIS_CMD\n");
    if (current_token.type == TOKEN_SEMICOLON) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        comando();
        mais_cmd();
    }
}

// <expressao> ::= <operador_unario> <termo> <mais_termos>
void expressao() {
  // fprintf(global_TextSaida, "EXPRESSAO\n");
    operador_unario();
    termo();
    mais_termos();
}

// <operador_unario> ::= - | + | λ
void operador_unario() {
   // fprintf(global_TextSaida, "OPERADOR UNARIO\n");
    if (current_token.type == TOKEN_MINUS) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
    } else if (current_token.type == TOKEN_PLUS) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
    }
}

// <termo> ::= <fator> <mais_fatores>
void termo() {
  //  fprintf(global_TextSaida, "TERMO\n");
    fator();
    mais_fatores();
}

// <mais_termos> ::= - <termo> <mais_termos> | + <termo> <mais_termos> | λ
void mais_termos() {
   // fprintf(global_TextSaida, "MAIS_TERMOS\n");
    if (current_token.type == TOKEN_MINUS || current_token.type == TOKEN_PLUS) {
        TokenType op_type = current_token.type;
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        termo();
        mais_termos();
    }
}

// <fator> ::= ident | numero | ( <expressao> )
void fator() {
  //  fprintf(global_TextSaida, "FATOR\n");
    if (current_token.type == TOKEN_IDENT) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
    } else if (current_token.type == TOKEN_NUMERO) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
    } else if (current_token.type == TOKEN_LPAREN) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        expressao();
        match(TOKEN_RPAREN, "Esperava ')'",
              TOKEN_MULTIPLY, TOKEN_DIVIDE, // Usando TOKEN_MULTIPLY, TOKEN_DIVIDE
              TOKEN_PLUS, TOKEN_MINUS,
              TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_LE, TOKEN_GT, TOKEN_GE, // Usando TOKEN_LE, TOKEN_GE
              TOKEN_SEMICOLON, TOKEN_END, TOKEN_PERIOD, TOKEN_EOF);
    } else {
        if (!panicMode)
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
  //  fprintf(global_TextSaida, "MAIS_FATORES\n");
    if (current_token.type == TOKEN_MULTIPLY || current_token.type == TOKEN_DIVIDE) { // Usando TOKEN_MULTIPLY, TOKEN_DIVIDE
        TokenType op_type = current_token.type;
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        fator();
        mais_fatores();
    }
}

// <condicao> ::= ODD <expressao> | <expressao> <relacional> <expressao>
void condicao() {
   // fprintf(global_TextSaida, "CONDICAO\n");
    if (current_token.type == TOKEN_ODD) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
        expressao();
    } else {
        expressao();
        relacao();
        expressao();
    }
}

// <relacional> ::= = | <> | < | <= | > | >=
void relacao() {
  //  fprintf(global_TextSaida, "RELACAO\n");
    if (current_token.type == TOKEN_EQ) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
    } else if (current_token.type == TOKEN_NEQ) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
    } else if (current_token.type == TOKEN_LT) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
    } else if (current_token.type == TOKEN_LE) { // Usando TOKEN_LE
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
    } else if (current_token.type == TOKEN_GT) {
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
    } else if (current_token.type == TOKEN_GE) { // Usando TOKEN_GE
        panicMode = 0;
        getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);
    } else {
        if (!panicMode)
        fprintf(global_TextSaida, "Erro sintatico: Operador relacional esperado. Encontrou '%s' ('%s').\n",
                current_token.lexeme, getTokenTypeName(current_token.type));
        *global_boolErro = 1;
        // Modificado L
        panic_mode_recover(current_token.type,
        TOKEN_PLUS, TOKEN_MINUS, TOKEN_IDENT, TOKEN_NUMERO, TOKEN_LPAREN,
        TOKEN_THEN, TOKEN_DO, TOKEN_SEMICOLON);
    }
}

// Função para iniciar a análise sintática
void iniciarAnaliseSintatica(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
    global_TextFile = textFile;
    global_TextSaida = textSaida;
    global_boolErro = boolErro;
    global_boolSpace = boolSpace;

    getNextToken(global_TextFile, global_TextSaida, global_boolErro, global_boolSpace, panicMode);

    programa();
    panicMode = 0;

   /* if (current_token.type != TOKEN_EOF) {
        fprintf(global_TextSaida, "Erro sintatico: Caracteres inesperados apos o final do programa. Encontrou '%s' ('%s').\n",
                current_token.lexeme, getTokenTypeName(current_token.type));
        *global_boolErro = 1;
    }*/
}