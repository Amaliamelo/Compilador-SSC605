#include "parser.h"

// A variável global g_currentToken é definida em lexico.c e declarada como extern em lexico.h.
// Ela contém o token atual que o analisador sintático está processando.

// Função para reportar um erro sintático no arquivo de saída.
void erroSintatico(const char* mensagem) {
    fprintf(g_textSaida, "ERRO SINTATICO: %s. Token atual: '%s' (Tipo: %d)\n",
            mensagem, g_currentToken.lexema, g_currentToken.tipo);
}

// Função para consumir o token esperado.
// Se o token atual for o esperado, avança para o próximo token.
// Caso contrário, reporta um erro sintático.
void consumirToken(TipoToken esperado) {
    if (g_currentToken.tipo == esperado) {
        proximoToken(); // Avança para o próximo token lendo-o do léxico
    } else {
        erroSintatico("Token inesperado");
        // A recuperação de erro em modo pânico será chamada após a detecção do erro,
        // permitindo que o parser tente se recuperar.
    }
}

// Função para recuperação de erros em modo pânico.
// Pula tokens até encontrar um dos tokens de sincronização ou o fim do arquivo.
void recuperarErro(TipoToken* sincronizacao, int num_sinc) {
    int encontrado = 0;
    while (!encontrado && g_currentToken.tipo != TOKEN_EOF) {
        for (int i = 0; i < num_sinc; i++) {
            if (g_currentToken.tipo == sincronizacao[i]) {
                encontrado = 1; // Token de sincronização encontrado
                break;
            }
        }
        if (!encontrado) {
            proximoToken(); // Pula o token atual, pois não é um token de sincronização
        }
    }
}

// Implementação das funções para cada não-terminal da gramática PL/0.
// As funções seguem a estrutura da gramática e chamam 'consumirToken'
// para verificar a correspondência dos tokens e 'erroSintatico'/'recuperarErro'
// para lidar com desvios da gramática.

// <programa> ::= <bloco> .
void programa() {
    proximoToken(); // Inicia lendo o primeiro token do arquivo
    bloco(); // Chama a função para analisar o bloco principal do programa

    // Após o bloco, espera-se um ponto '.'
    if (g_currentToken.tipo == TOKEN_PONTO) {
        consumirToken(TOKEN_PONTO);
    } else {
        erroSintatico("Esperado '.' no final do programa");
        TipoToken sync[] = {TOKEN_EOF}; // Tokens de sincronização para o fim do programa
        recuperarErro(sync, 1);
    }

    // Verifica se há caracteres extras após o ponto final
    if (g_currentToken.tipo != TOKEN_EOF) {
        erroSintatico("Caracteres extras após o fim do programa.");
        // Não há mais tokens para sincronizar aqui, pois é o final esperado.
    }
}

// <bloco> ::= [CONST <declaracao_constante> { , <declaracao_constante> } ; ]
//             [VAR <declaracao_variavel> { , <declaracao_variavel> } ; ]
//             { PROCEDURE <identificador> ; <bloco> ; }
//             <comando>
void bloco() {
    // Conjunto de tokens de sincronização para o início de um bloco ou após um erro em declarações
    TipoToken syncBloco[] = {
        TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL,
        TOKEN_IDENT, TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_PONTO, TOKEN_EOF
    };
    int numSyncBloco = sizeof(syncBloco) / sizeof(TipoToken);

    // Declarações de CONSTANTES
    if (g_currentToken.tipo == TOKEN_CONST) {
        consumirToken(TOKEN_CONST); // Consome 'CONST'
        declaracaoConstante(); // Analisa a primeira declaração de constante
        while (g_currentToken.tipo == TOKEN_VIRGULA) {
            consumirToken(TOKEN_VIRGULA); // Consome ','
            declaracaoConstante(); // Analisa as declarações de constante subsequentes
        }
        if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
            consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
        } else {
            erroSintatico("Esperado ';' após declaração de constante");
            recuperarErro(syncBloco, numSyncBloco);
        }
    }

    // Declarações de VARIÁVEIS
    if (g_currentToken.tipo == TOKEN_VAR) {
        consumirToken(TOKEN_VAR); // Consome 'VAR'
        declaracaoVariavel(); // Analisa a primeira declaração de variável
        while (g_currentToken.tipo == TOKEN_VIRGULA) {
            consumirToken(TOKEN_VIRGULA); // Consome ','
            declaracaoVariavel(); // Analisa as declarações de variável subsequentes
        }
        if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
            consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
        } else {
            erroSintatico("Esperado ';' após declaração de variável");
            recuperarErro(syncBloco, numSyncBloco);
        }
    }

    // Declarações de PROCEDIMENTOS
    while (g_currentToken.tipo == TOKEN_PROCEDURE) {
        declaracaoProcedimento(); // Analisa a declaração de um procedimento
    }

    // Comando principal do bloco
    comando();
}

// <declaracao_constante> ::= <identificador> = <numero>
void declaracaoConstante() {
    TipoToken syncConst[] = {TOKEN_VIRGULA, TOKEN_PONTO_E_VIRGULA, TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_EOF};
    int numSyncConst = sizeof(syncConst) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_IDENT) {
        consumirToken(TOKEN_IDENT); // Consome o identificador da constante
        if (g_currentToken.tipo == TOKEN_IGUAL) {
            consumirToken(TOKEN_IGUAL); // Consome '='
            if (g_currentToken.tipo == TOKEN_NUMERO) {
                consumirToken(TOKEN_NUMERO); // Consome o número (valor da constante)
            } else {
                erroSintatico("Esperado número após '=' em declaração de constante");
                recuperarErro(syncConst, numSyncConst);
            }
        } else {
            erroSintatico("Esperado '=' em declaração de constante");
            recuperarErro(syncConst, numSyncConst);
        }
    } else {
        erroSintatico("Esperado identificador em declaração de constante");
        recuperarErro(syncConst, numSyncConst);
    }
}

// <declaracao_variavel> ::= <identificador>
void declaracaoVariavel() {
    TipoToken syncVar[] = {TOKEN_VIRGULA, TOKEN_PONTO_E_VIRGULA, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_EOF};
    int numSyncVar = sizeof(syncVar) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_IDENT) {
        consumirToken(TOKEN_IDENT); // Consome o identificador da variável
    } else {
        erroSintatico("Esperado identificador em declaração de variável");
        recuperarErro(syncVar, numSyncVar);
    }
}

// <declaracao_procedimento> ::= PROCEDURE <identificador> ; <bloco> ;
void declaracaoProcedimento() {
    TipoToken syncProc[] = {TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_IF, TOKEN_WHILE, TOKEN_CALL, TOKEN_IDENT, TOKEN_PONTO, TOKEN_EOF};
    int numSyncProc = sizeof(syncProc) / sizeof(TipoToken);

    consumirToken(TOKEN_PROCEDURE); // Consome 'PROCEDURE'
    if (g_currentToken.tipo == TOKEN_IDENT) {
        consumirToken(TOKEN_IDENT); // Consome o identificador do procedimento
        if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
            consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
            bloco(); // Analisa o bloco do procedimento
            if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
                consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';' após o bloco do procedimento
            } else {
                erroSintatico("Esperado ';' após bloco de procedimento");
                recuperarErro(syncProc, numSyncProc);
            }
        } else {
            erroSintatico("Esperado ';' após identificador de procedimento");
            recuperarErro(syncProc, numSyncProc);
        }
    } else {
        erroSintatico("Esperado identificador em declaração de procedimento");
        recuperarErro(syncProc, numSyncProc);
    }
}

// <comando> ::= <atribuicao> | <chamada_procedimento> | <comando_condicional> | <comando_repeticao> | <comando_composto> | <comando_vazio>
void comando() {
    // Conjunto de tokens de sincronização para o início de um novo comando ou após um erro em um comando
    TipoToken syncComando[] = {
        TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_EOF, TOKEN_THEN, TOKEN_DO, // Tokens que podem seguir um comando
        TOKEN_IDENT, TOKEN_CALL, TOKEN_IF, TOKEN_WHILE, TOKEN_BEGIN // Tokens que iniciam um comando
    };
    int numSyncComando = sizeof(syncComando) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_IDENT) {
        atribuicao(); // Se for identificador, assume-se que é uma atribuição
    } else if (g_currentToken.tipo == TOKEN_CALL) {
        chamadaProcedimento(); // Se for 'CALL', é uma chamada de procedimento
    } else if (g_currentToken.tipo == TOKEN_IF) {
        comandoCondicional(); // Se for 'IF', é um comando condicional
    } else if (g_currentToken.tipo == TOKEN_WHILE) {
        comandoRepeticao(); // Se for 'WHILE', é um comando de repetição
    } else if (g_currentToken.tipo == TOKEN_BEGIN) {
        comandoComposto(); // Se for 'BEGIN', é um comando composto
    } else {
        // Se não for nenhum dos tokens que iniciam um comando,
        // pode ser um comando vazio (epsilon) ou um erro.
        // Se for um token de sincronização, consideramos um comando vazio e não reportamos erro.
        int is_sync_token = 0;
        for (int i = 0; i < numSyncComando; i++) {
            if (g_currentToken.tipo == syncComando[i]) {
                is_sync_token = 1;
                break;
            }
        }
        if (!is_sync_token && g_currentToken.tipo != TOKEN_EOF) {
            erroSintatico("Comando inesperado ou incompleto");
            recuperarErro(syncComando, numSyncComando);
        }
        // Se for um token de sincronização ou EOF, é um comando vazio, não faz nada.
    }
}

// <atribuicao> ::= <identificador> := <expressao>
void atribuicao() {
    TipoToken syncAtrib[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_EOF, TOKEN_THEN, TOKEN_DO};
    int numSyncAtrib = sizeof(syncAtrib) / sizeof(TipoToken);

    consumirToken(TOKEN_IDENT); // Consome o identificador
    if (g_currentToken.tipo == TOKEN_ATRIBUICAO) {
        consumirToken(TOKEN_ATRIBUICAO); // Consome ':='
        expressao(); // Analisa a expressão à direita da atribuição
    } else {
        erroSintatico("Esperado ':=' em comando de atribuição");
        recuperarErro(syncAtrib, numSyncAtrib);
    }
}

// <chamada_procedimento> ::= CALL <identificador>
void chamadaProcedimento() {
    TipoToken syncCall[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_EOF, TOKEN_THEN, TOKEN_DO};
    int numSyncCall = sizeof(syncCall) / sizeof(TipoToken);

    consumirToken(TOKEN_CALL); // Consome 'CALL'
    if (g_currentToken.tipo == TOKEN_IDENT) {
        consumirToken(TOKEN_IDENT); // Consome o identificador do procedimento a ser chamado
    } else {
        erroSintatico("Esperado identificador após 'CALL'");
        recuperarErro(syncCall, numSyncCall);
    }
}

// <comando_condicional> ::= IF <condicao> THEN <comando>
void comandoCondicional() {
    TipoToken syncIf[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_EOF};
    int numSyncIf = sizeof(syncIf) / sizeof(TipoToken);

    consumirToken(TOKEN_IF); // Consome 'IF'
    condicao(); // Analisa a condição
    if (g_currentToken.tipo == TOKEN_THEN) {
        consumirToken(TOKEN_THEN); // Consome 'THEN'
        comando(); // Analisa o comando a ser executado se a condição for verdadeira
    } else {
        erroSintatico("Esperado 'THEN' em comando condicional");
        recuperarErro(syncIf, numSyncIf);
    }
}

// <comando_repeticao> ::= WHILE <condicao> DO <comando>
void comandoRepeticao() {
    TipoToken syncWhile[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_EOF};
    int numSyncWhile = sizeof(syncWhile) / sizeof(TipoToken);

    consumirToken(TOKEN_WHILE); // Consome 'WHILE'
    condicao(); // Analisa a condição de repetição
    if (g_currentToken.tipo == TOKEN_DO) {
        consumirToken(TOKEN_DO); // Consome 'DO'
        comando(); // Analisa o comando a ser repetido
    } else {
        erroSintatico("Esperado 'DO' em comando de repetição");
        recuperarErro(syncWhile, numSyncWhile);
    }
}

// <comando_composto> ::= BEGIN <comando> { ; <comando> } END
void comandoComposto() {
    TipoToken syncBeginEnd[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_EOF};
    int numSyncBeginEnd = sizeof(syncBeginEnd) / sizeof(TipoToken);

    consumirToken(TOKEN_BEGIN); // Consome 'BEGIN'
    comando(); // Analisa o primeiro comando
    while (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
        consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
        comando(); // Analisa os comandos subsequentes
    }
    if (g_currentToken.tipo == TOKEN_END) {
        consumirToken(TOKEN_END); // Consome 'END'
    } else {
        erroSintatico("Esperado 'END' em comando composto");
        recuperarErro(syncBeginEnd, numSyncBeginEnd);
    }
}

// <condicao> ::= <expressao> <operador_relacional> <expressao> | ODD <expressao>
void condicao() {
    // Tokens de sincronização para a recuperação de erros em condições
    TipoToken syncCond[] = {TOKEN_THEN, TOKEN_DO, TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_EOF};
    int numSyncCond = sizeof(syncCond) / sizeof(TipoToken);

    // Verifica se a condição começa com 'ODD' (tratado como identificador especial aqui)
    if (g_currentToken.tipo == TOKEN_IDENT && strcmp(g_currentToken.lexema, "ODD") == 0) {
        proximoToken(); // Consome 'ODD'
        expressao(); // Analisa a expressão após 'ODD'
    } else {
        // Caso contrário, espera-se uma expressão seguida por um operador relacional e outra expressão
        expressao(); // Analisa a primeira expressão

        // Verifica se o token atual é um operador relacional
        if (g_currentToken.tipo == TOKEN_IGUAL || g_currentToken.tipo == TOKEN_DIFERENTE ||
            g_currentToken.tipo == TOKEN_MENOR || g_currentToken.tipo == TOKEN_MENOR_IGUAL ||
            g_currentToken.tipo == TOKEN_MAIOR || g_currentToken.tipo == TOKEN_MAIOR_IGUAL) {
            proximoToken(); // Consome o operador relacional
            expressao(); // Analisa a segunda expressão
        } else {
            erroSintatico("Esperado operador relacional em condição");
            recuperarErro(syncCond, numSyncCond);
        }
    }
}

// <expressao> ::= [ + | - ] <termo> { ( + | - ) <termo> }
void expressao() {
    // Verifica se há um sinal unário no início da expressão
    if (g_currentToken.tipo == TOKEN_MAIS || g_currentToken.tipo == TOKEN_MENOS) {
        proximoToken(); // Consome '+' ou '-'
    }
    termo(); // Analisa o primeiro termo da expressão

    // Loop para termos adicionais conectados por '+' ou '-'
    while (g_currentToken.tipo == TOKEN_MAIS || g_currentToken.tipo == TOKEN_MENOS) {
        proximoToken(); // Consome '+' ou '-'
        termo(); // Analisa o próximo termo
    }
}

// <termo> ::= <fator> { ( * | / ) <fator> }
void termo() {
    fator(); // Analisa o primeiro fator do termo

    // Loop para fatores adicionais conectados por '*' ou '/'
    while (g_currentToken.tipo == TOKEN_MULTIPLICACAO || g_currentToken.tipo == TOKEN_DIVISAO) {
        proximoToken(); // Consome '*' ou '/'
        fator(); // Analisa o próximo fator
    }
}

// <fator> ::= <identificador> | <numero> | ( <expressao> )
void fator() {
    // Tokens de sincronização para a recuperação de erros em fatores
    TipoToken syncFator[] = {
        TOKEN_MAIS, TOKEN_MENOS, TOKEN_MULTIPLICACAO, TOKEN_DIVISAO,
        TOKEN_IGUAL, TOKEN_DIFERENTE, TOKEN_MENOR, TOKEN_MENOR_IGUAL,
        TOKEN_MAIOR, TOKEN_MAIOR_IGUAL, TOKEN_THEN, TOKEN_DO,
        TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_EOF,
        TOKEN_PARENTESES_DIR // Adicionado para recuperação de parênteses não fechados
    };
    int numSyncFator = sizeof(syncFator) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_IDENT) {
        consumirToken(TOKEN_IDENT); // Se for identificador, consome-o
    } else if (g_currentToken.tipo == TOKEN_NUMERO) {
        consumirToken(TOKEN_NUMERO); // Se for número, consome-o
    } else if (g_currentToken.tipo == TOKEN_PARENTESES_ESQ) {
        consumirToken(TOKEN_PARENTESES_ESQ); // Se for '(', consome-o
        expressao(); // Analisa a expressão dentro dos parênteses
        if (g_currentToken.tipo == TOKEN_PARENTESES_DIR) {
            consumirToken(TOKEN_PARENTESES_DIR); // Espera e consome ')'
        } else {
            erroSintatico("Esperado ')' em fator");
            recuperarErro(syncFator, numSyncFator);
        }
    } else {
        erroSintatico("Esperado identificador, número ou '(' em fator");
        recuperarErro(syncFator, numSyncFator);
    }
}

// Função principal para iniciar a análise sintática.
// Chama a função 'programa' para começar a análise da gramática.
// Ao final, imprime uma mensagem de sucesso ou falha.
void analisarSintaticamente() {
    programa(); // Inicia a análise sintática a partir do não-terminal inicial <programa>

    // Após a análise do programa, se o token atual for EOF, a compilação foi bem-sucedida.
    // Caso contrário, significa que houve erros sintáticos não recuperados ou caracteres extras.
    if (g_currentToken.tipo == TOKEN_EOF) {
        fprintf(g_textSaida, "Compilação teve sucesso!\n");
    } else {
        fprintf(g_textSaida, "Compilação terminou com erros.\n");
    }
}
