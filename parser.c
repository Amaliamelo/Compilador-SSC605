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

// Implementação das funções para cada não-terminal da gramática P--.

// Regra 1: <programa> ::= program ident ; <corpo> .
void programa() {
    proximoToken(); // Inicia lendo o primeiro token do arquivo

    // Sincronização para o início do programa (palavra 'program')
    TipoToken syncProgram[] = {TOKEN_PROGRAM, TOKEN_EOF};
    recuperarErro(syncProgram, sizeof(syncProgram) / sizeof(TipoToken));

    if (g_currentToken.tipo == TOKEN_PROGRAM) {
        consumirToken(TOKEN_PROGRAM); // Consome 'program'
        if (g_currentToken.tipo == TOKEN_IDENT) {
            consumirToken(TOKEN_IDENT); // Consome identificador do programa
            if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
                consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
                corpo(); // Analisa o corpo do programa
                if (g_currentToken.tipo == TOKEN_PONTO) {
                    consumirToken(TOKEN_PONTO); // Consome '.' final do programa
                } else {
                    erroSintatico("Esperado '.' no final do programa.");
                    TipoToken sync[] = {TOKEN_EOF};
                    recuperarErro(sync, 1);
                }
            } else {
                erroSintatico("Esperado ';' apos o nome do programa.");
                TipoToken sync[] = {TOKEN_CONST, TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_EOF};
                recuperarErro(sync, sizeof(sync) / sizeof(TipoToken));
            }
        } else {
            erroSintatico("Esperado identificador do programa apos 'program'.");
            TipoToken sync[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_CONST, TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_EOF};
            recuperarErro(sync, sizeof(sync) / sizeof(TipoToken));
        }
    } else {
        erroSintatico("Esperado 'program' no inicio do programa.");
        TipoToken sync[] = {TOKEN_EOF};
        recuperarErro(sync, 1);
    }
    
    // Verifica se há caracteres extras após o ponto final
    if (g_currentToken.tipo != TOKEN_EOF) {
        erroSintatico("Caracteres extras apos o fim do programa.");
    }
}

// Regra 2: <corpo> ::= <dc> begin <comandos> end
void corpo() {
    // Sincronização para o início de corpo
    TipoToken syncCorpo[] = {TOKEN_BEGIN, TOKEN_EOF};
    
    dc(); // Declarações

    if (g_currentToken.tipo == TOKEN_BEGIN) {
        consumirToken(TOKEN_BEGIN); // Consome 'begin'
        comandos(); // Lista de comandos
        if (g_currentToken.tipo == TOKEN_END) {
            consumirToken(TOKEN_END); // Consome 'end'
        } else {
            erroSintatico("Esperado 'end' no final do corpo.");
            TipoToken sync[] = {TOKEN_PONTO, TOKEN_PONTO_E_VIRGULA, TOKEN_EOF}; // 'end' pode ser seguido por '.' ou ';'
            recuperarErro(sync, sizeof(sync) / sizeof(TipoToken));
        }
    } else {
        erroSintatico("Esperado 'begin' no corpo do programa/procedimento.");
        recuperarErro(syncCorpo, sizeof(syncCorpo) / sizeof(TipoToken));
    }
}

// Regra 3: <dc> ::= <dc_c> <dc_v> <dc_p>
void dc() {
    // Note que dc_c, dc_v e dc_p podem ser lambda (λ), então eles são chamados condicionalmente.
    // Não há tokens de sincronização específicos aqui, pois cada função de declaração tem os seus.
    // O parser simplesmente tenta casar as declarações na ordem.

    dcConst(); // Tenta casar declarações de constantes
    dcVar();   // Tenta casar declarações de variáveis
    dcProc();  // Tenta casar declarações de procedimentos
}

// Regra 4: <dc_c> ::= const ident = <numero> ; <dc_c> | λ
void dcConst() {
    // Sincronização para o início de uma nova declaração (const, var, procedure, begin)
    TipoToken syncDcConst[] = {TOKEN_VAR, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_EOF};
    int numSyncDcConst = sizeof(syncDcConst) / sizeof(TipoToken);

    while (g_currentToken.tipo == TOKEN_CONST) {
        consumirToken(TOKEN_CONST); // Consome 'const'
        if (g_currentToken.tipo == TOKEN_IDENT) {
            consumirToken(TOKEN_IDENT); // Consome identificador da constante
            if (g_currentToken.tipo == TOKEN_IGUAL) {
                consumirToken(TOKEN_IGUAL); // Consome '='
                if (g_currentToken.tipo == TOKEN_NUMERO_INT || g_currentToken.tipo == TOKEN_NUMERO_REAL) {
                    proximoToken(); // Consome o número (int ou real)
                    if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
                        consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
                    } else {
                        erroSintatico("Esperado ';' apos declaracao de constante.");
                        recuperarErro(syncDcConst, numSyncDcConst);
                    }
                } else {
                    erroSintatico("Esperado numero em declaracao de constante.");
                    recuperarErro(syncDcConst, numSyncDcConst);
                }
            } else {
                erroSintatico("Esperado '=' em declaracao de constante.");
                recuperarErro(syncDcConst, numSyncDcConst);
            }
        } else {
            erroSintatico("Esperado identificador em declaracao de constante.");
            recuperarErro(syncDcConst, numSyncDcConst);
        }
    }
}

// Regra 5: <dc_v> ::= var <variaveis> : <tipo_var> ; <dc_v> | λ
void dcVar() {
    // Sincronização para o início de uma nova declaração (var, procedure, begin)
    TipoToken syncDcVar[] = {TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_EOF};
    int numSyncDcVar = sizeof(syncDcVar) / sizeof(TipoToken);

    while (g_currentToken.tipo == TOKEN_VAR) {
        consumirToken(TOKEN_VAR); // Consome 'var'
        variaveis(); // Lista de variáveis
        if (g_currentToken.tipo == TOKEN_DOIS_PONTOS) { // Agora usa ':' para tipo
            consumirToken(TOKEN_DOIS_PONTOS); // Consome ':'
            tipoVar(); // Tipo da variável
            if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
                consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
            } else {
                erroSintatico("Esperado ';' apos declaracao de variavel.");
                recuperarErro(syncDcVar, numSyncDcVar);
            }
        } else {
            erroSintatico("Esperado ':' em declaracao de variavel.");
            recuperarErro(syncDcVar, numSyncDcVar);
        }
    }
}

// Regra 6: <tipo_var> ::= real | integer
void tipoVar() {
    // Sincronização para depois do tipo (;)
    TipoToken syncTipoVar[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_EOF};
    int numSyncTipoVar = sizeof(syncTipoVar) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_REAL) {
        consumirToken(TOKEN_REAL);
    } else if (g_currentToken.tipo == TOKEN_INTEGER) {
        consumirToken(TOKEN_INTEGER);
    } else {
        erroSintatico("Esperado 'real' ou 'integer' como tipo de variavel.");
        recuperarErro(syncTipoVar, numSyncTipoVar);
    }
}

// Regra 7: <variaveis> ::= ident <mais_var>
void variaveis() {
    // Sincronização para depois de uma lista de variáveis (:, ;)
    TipoToken syncVariaveis[] = {TOKEN_DOIS_PONTOS, TOKEN_PONTO_E_VIRGULA, TOKEN_EOF};
    int numSyncVariaveis = sizeof(syncVariaveis) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_IDENT) {
        consumirToken(TOKEN_IDENT); // Consome o primeiro identificador
        maisVar(); // Tenta casar mais variáveis (opcional)
    } else {
        erroSintatico("Esperado identificador em lista de variaveis.");
        recuperarErro(syncVariaveis, numSyncVariaveis);
    }
}

// Regra 8: <mais_var> ::= , <variaveis> | λ
void maisVar() {
    // Sincronização para depois de uma lista de variáveis (:, ;)
    TipoToken syncMaisVar[] = {TOKEN_DOIS_PONTOS, TOKEN_PONTO_E_VIRGULA, TOKEN_EOF};
    int numSyncMaisVar = sizeof(syncMaisVar) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_VIRGULA) {
        consumirToken(TOKEN_VIRGULA); // Consome ','
        variaveis(); // Chama variaveis recursivamente para a próxima
    }
    // else { λ } - Se não encontrar ',', é lambda e retorna.
}

// Regra 9: <dc_p> ::= procedure ident <parametros> ; <corpo_p> <dc_p> | λ
void dcProc() {
    // Sincronização para o início de uma nova declaração (procedure, begin)
    TipoToken syncDcProc[] = {TOKEN_BEGIN, TOKEN_EOF};
    int numSyncDcProc = sizeof(syncDcProc) / sizeof(TipoToken);

    while (g_currentToken.tipo == TOKEN_PROCEDURE) {
        consumirToken(TOKEN_PROCEDURE); // Consome 'procedure'
        if (g_currentToken.tipo == TOKEN_IDENT) {
            consumirToken(TOKEN_IDENT); // Consome identificador do procedimento
            parametros(); // Analisa os parâmetros (opcional)
            if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
                consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
                corpoP(); // Analisa o corpo do procedimento
                // O ';' final do corpo_p já é consumido dentro de corpoP.
                // Não precisa de outro consumirToken aqui.
            } else {
                erroSintatico("Esperado ';' apos identificador/parametros de procedimento.");
                recuperarErro(syncDcProc, numSyncDcProc);
            }
        } else {
            erroSintatico("Esperado identificador em declaracao de procedimento.");
            recuperarErro(syncDcProc, numSyncDcProc);
        }
    }
}

// Regra 10: <parametros> ::= ( <lista_par> ) | λ
void parametros() {
    // Sincronização para depois dos parâmetros (;)
    TipoToken syncParam[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_EOF};
    int numSyncParam = sizeof(syncParam) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_PARENTESES_ESQ) {
        consumirToken(TOKEN_PARENTESES_ESQ); // Consome '('
        listaPar(); // Lista de parâmetros
        if (g_currentToken.tipo == TOKEN_PARENTESES_DIR) {
            consumirToken(TOKEN_PARENTESES_DIR); // Consome ')'
        } else {
            erroSintatico("Esperado ')' em lista de parametros.");
            recuperarErro(syncParam, numSyncParam);
        }
    }
    // else { λ } - Se não encontrar '(', é lambda e retorna.
}

// Regra 11: <lista_par> ::= <variaveis> : <tipo_var> <mais_par>
void listaPar() {
    // Sincronização para depois do tipo (;)
    TipoToken syncListaPar[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_PARENTESES_DIR, TOKEN_EOF};
    int numSyncListaPar = sizeof(syncListaPar) / sizeof(TipoToken);

    variaveis(); // Variáveis
    if (g_currentToken.tipo == TOKEN_DOIS_PONTOS) {
        consumirToken(TOKEN_DOIS_PONTOS); // Consome ':'
        tipoVar(); // Tipo
        maisPar(); // Mais parâmetros (opcional)
    } else {
        erroSintatico("Esperado ':' em lista de parametros.");
        recuperarErro(syncListaPar, numSyncListaPar);
    }
}

// Regra 12: <mais_par> ::= ; <lista_par> | λ
void maisPar() {
    // Sincronização para depois do tipo (;) ou )
    TipoToken syncMaisPar[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_PARENTESES_DIR, TOKEN_EOF};
    int numSyncMaisPar = sizeof(syncMaisPar) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
        consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
        listaPar(); // Chama listaPar recursivamente
    }
    // else { λ } - Se não encontrar ';', é lambda e retorna.
}

// Regra 13: <corpo_p> ::= <dc_loc> begin <comandos> end ;
void corpoP() {
    TipoToken syncCorpoP[] = {TOKEN_BEGIN, TOKEN_PONTO_E_VIRGULA, TOKEN_EOF};
    int numSyncCorpoP = sizeof(syncCorpoP) / sizeof(TipoToken);

    dcLoc(); // Declarações locais (só variáveis)

    if (g_currentToken.tipo == TOKEN_BEGIN) {
        consumirToken(TOKEN_BEGIN); // Consome 'begin'
        comandos(); // Lista de comandos
        if (g_currentToken.tipo == TOKEN_END) {
            consumirToken(TOKEN_END); // Consome 'end'
            if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
                consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
            } else {
                erroSintatico("Esperado ';' apos 'end' do corpo do procedimento.");
                TipoToken sync[] = {TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_EOF}; // Sincroniza para próxima declaração de proc ou main begin
                recuperarErro(sync, sizeof(sync) / sizeof(TipoToken));
            }
        } else {
            erroSintatico("Esperado 'end' no final do corpo do procedimento.");
            TipoToken sync[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_PROCEDURE, TOKEN_BEGIN, TOKEN_EOF};
            recuperarErro(sync, sizeof(sync) / sizeof(TipoToken));
        }
    } else {
        erroSintatico("Esperado 'begin' no corpo do procedimento.");
        recuperarErro(syncCorpoP, numSyncCorpoP);
    }
}

// Regra 14: <dc_loc> ::= <dc_v>
void dcLoc() {
    dcVar(); // Declarações de variáveis locais
}

// Regra 15: <lista_arg> ::= ( <argumentos> ) | λ
void listaArg() {
    // Sincronização para depois da lista de argumentos (;, end, then, do, etc)
    TipoToken syncListaArg[] = {
        TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_THEN, TOKEN_DO, TOKEN_PONTO, TOKEN_EOF
    };
    int numSyncListaArg = sizeof(syncListaArg) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_PARENTESES_ESQ) {
        consumirToken(TOKEN_PARENTESES_ESQ); // Consome '('
        argumentos(); // Lista de argumentos
        if (g_currentToken.tipo == TOKEN_PARENTESES_DIR) {
            consumirToken(TOKEN_PARENTESES_DIR); // Consome ')'
        } else {
            erroSintatico("Esperado ')' em lista de argumentos.");
            recuperarErro(syncListaArg, numSyncListaArg);
        }
    }
    // else { λ } - Se não encontrar '(', é lambda e retorna.
}

// Regra 16: <argumentos> ::= ident <mais_ident>
void argumentos() {
    // Sincronização para depois dos argumentos (;, ))
    TipoToken syncArgs[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_PARENTESES_DIR, TOKEN_EOF};
    int numSyncArgs = sizeof(syncArgs) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_IDENT) {
        consumirToken(TOKEN_IDENT); // Consome o primeiro identificador
        maisIdent(); // Mais identificadores (opcional)
    } else {
        erroSintatico("Esperado identificador em lista de argumentos.");
        recuperarErro(syncArgs, numSyncArgs);
    }
}

// Regra 17: <mais_ident> ::= ; <argumentos> | λ
void maisIdent() {
    // Sincronização para depois dos argumentos (;, ))
    TipoToken syncMaisIdent[] = {TOKEN_PONTO_E_VIRGULA, TOKEN_PARENTESES_DIR, TOKEN_EOF};
    int numSyncMaisIdent = sizeof(syncMaisIdent) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
        consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
        argumentos(); // Chama argumentos recursivamente
    }
    // else { λ } - Se não encontrar ';', é lambda e retorna.
}

// Regra 18: <pfalsa> ::= else <cmd> | λ
void pfalsa() {
    // Sincronização para depois do else (;, end, else, then, do, .)
    TipoToken syncPfalsa[] = {
        TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_THEN, TOKEN_DO,
        TOKEN_PONTO, TOKEN_EOF
    };
    int numSyncPfalsa = sizeof(syncPfalsa) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_ELSE) { // Assumindo TOKEN_ELSE para 'else'
        consumirToken(TOKEN_ELSE); // Consome 'else'
        cmd(); // Comando do else
    }
    // else { λ } - Se não encontrar 'else', é lambda e retorna.
}

// Regra 19: <comandos> ::= <cmd> ; <comandos> | λ
// NOTA: O ';' é um separador de comandos, não um terminador.
// Isso significa que o último comando ANTES de um 'END' ou 'ELSE' NÃO terá ';'.
void comandos() {
    // Conjunto de tokens que iniciam um comando ou terminam a lista de comandos
    TipoToken firstCmd[] = {
        TOKEN_READ, TOKEN_WRITE, TOKEN_WHILE, TOKEN_IF,
        TOKEN_IDENT, TOKEN_BEGIN, TOKEN_FOR, TOKEN_CALL // assuming CALL is part of ident <lista_arg>
    };
    TipoToken followCmds[] = {
        TOKEN_END, TOKEN_PONTO, TOKEN_ELSE, TOKEN_EOF, // Tokens que terminam a lista de comandos
        TOKEN_PONTO_E_VIRGULA // Para recuperação de erro, caso o ';' esteja faltando após um comando
    };
    
    // Continua processando comandos enquanto o token atual for um "first" de <cmd>
    // e não for um "follow" de <comandos> que indica o fim da lista.
    int hasCmd = 0;
    for (int i = 0; i < sizeof(firstCmd)/sizeof(TipoToken); i++) {
        if (g_currentToken.tipo == firstCmd[i]) {
            hasCmd = 1;
            break;
        }
    }

    if (hasCmd) {
        cmd(); // Analisa o primeiro comando

        // Loop para comandos adicionais, separados por ';'
        while (g_currentToken.tipo == TOKEN_PONTO_E_VIRGULA) {
            consumirToken(TOKEN_PONTO_E_VIRGULA); // Consome ';'
            
            // Verifica se o token atual é um "first" de <cmd> para continuar.
            // Isso previne um erro se houver um ';' extra no final.
            hasCmd = 0;
            for (int i = 0; i < sizeof(firstCmd)/sizeof(TipoToken); i++) {
                if (g_currentToken.tipo == firstCmd[i]) {
                    hasCmd = 1;
                    break;
                }
            }
            if (hasCmd) {
                cmd(); // Analisa o próximo comando
            } else {
                // Se houver um ';' mas nenhum comando válido a seguir (ex: "cmd; end"),
                // tratamos como um comando vazio após o ';'. A gramática permite λ.
                // Mas se não for END, PONTO ou EOF, pode ser um erro de ';' extra.
                int isFollow = 0;
                for (int i = 0; i < sizeof(followCmds)/sizeof(TipoToken); i++) {
                    if (g_currentToken.tipo == followCmds[i]) {
                        isFollow = 1;
                        break;
                    }
                }
                if (!isFollow && g_currentToken.tipo != TOKEN_EOF) {
                    erroSintatico("Esperado comando apos ';'.");
                    // Tenta recuperar para um 'follow' de comandos ou 'first' de comandos
                    TipoToken recoveryCmds[] = {
                        TOKEN_END, TOKEN_PONTO, TOKEN_ELSE, TOKEN_EOF,
                        TOKEN_READ, TOKEN_WRITE, TOKEN_WHILE, TOKEN_IF,
                        TOKEN_IDENT, TOKEN_BEGIN, TOKEN_FOR, TOKEN_CALL
                    };
                    recuperarErro(recoveryCmds, sizeof(recoveryCmds) / sizeof(TipoToken));
                }
            }
        }
    }
    // else { λ } - Se não houver nenhum comando, é lambda e retorna.
}


// Regra 20: <cmd> ::= read ( <variaveis> ) | write ( <variaveis> ) | while ( <condicao> ) do <cmd> |
//                     if <condicao> then <cmd> <pfalsa> | ident := <expressao> | ident <lista_arg> |
//                     begin <comandos> end | for ident := <expressao> to <expressao> do <cmd>
void cmd() {
    // Sincronização para depois de um comando (;, end, else, then, do, .)
    TipoToken syncCmd[] = {
        TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_ELSE, TOKEN_THEN, TOKEN_DO,
        TOKEN_PONTO, TOKEN_EOF
    };
    int numSyncCmd = sizeof(syncCmd) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_READ) {
        consumirToken(TOKEN_READ); // Consome 'read'
        if (g_currentToken.tipo == TOKEN_PARENTESES_ESQ) {
            consumirToken(TOKEN_PARENTESES_ESQ); // Consome '('
            variaveis(); // Lista de variáveis
            if (g_currentToken.tipo == TOKEN_PARENTESES_DIR) {
                consumirToken(TOKEN_PARENTESES_DIR); // Consome ')'
            } else {
                erroSintatico("Esperado ')' em comando read.");
                recuperarErro(syncCmd, numSyncCmd);
            }
        } else {
            erroSintatico("Esperado '(' em comando read.");
            recuperarErro(syncCmd, numSyncCmd);
        }
    } else if (g_currentToken.tipo == TOKEN_WRITE) {
        consumirToken(TOKEN_WRITE); // Consome 'write'
        if (g_currentToken.tipo == TOKEN_PARENTESES_ESQ) {
            consumirToken(TOKEN_PARENTESES_ESQ); // Consome '('
            variaveis(); // Lista de variáveis
            if (g_currentToken.tipo == TOKEN_PARENTESES_DIR) {
                consumirToken(TOKEN_PARENTESES_DIR); // Consome ')'
            } else {
                erroSintatico("Esperado ')' em comando write.");
                recuperarErro(syncCmd, numSyncCmd);
            }
        } else {
            erroSintatico("Esperado '(' em comando write.");
            recuperarErro(syncCmd, numSyncCmd);
        }
    } else if (g_currentToken.tipo == TOKEN_WHILE) {
        consumirToken(TOKEN_WHILE); // Consome 'while'
        if (g_currentToken.tipo == TOKEN_PARENTESES_ESQ) { // Assumindo que a condição pode estar entre parênteses
            consumirToken(TOKEN_PARENTESES_ESQ); // Consome '('
            condicao(); // Condição
            if (g_currentToken.tipo == TOKEN_PARENTESES_DIR) {
                consumirToken(TOKEN_PARENTESES_DIR); // Consome ')'
                if (g_currentToken.tipo == TOKEN_DO) {
                    consumirToken(TOKEN_DO); // Consome 'do'
                    cmd(); // Comando do while
                } else {
                    erroSintatico("Esperado 'do' em comando while.");
                    recuperarErro(syncCmd, numSyncCmd);
                }
            } else {
                erroSintatico("Esperado ')' apos condicao em comando while.");
                recuperarErro(syncCmd, numSyncCmd);
            }
        } else { // Se a condição não estiver entre parênteses, apenas chame condicao()
            condicao(); // Condição
            if (g_currentToken.tipo == TOKEN_DO) {
                consumirToken(TOKEN_DO); // Consome 'do'
                cmd(); // Comando do while
            } else {
                erroSintatico("Esperado 'do' em comando while.");
                recuperarErro(syncCmd, numSyncCmd);
            }
        }
    } else if (g_currentToken.tipo == TOKEN_IF) {
        consumirToken(TOKEN_IF); // Consome 'if'
        condicao(); // Condição
        if (g_currentToken.tipo == TOKEN_THEN) {
            consumirToken(TOKEN_THEN); // Consome 'then'
            cmd(); // Comando do then
            pfalsa(); // Parte 'else' (opcional)
        } else {
            erroSintatico("Esperado 'then' em comando if.");
            recuperarErro(syncCmd, numSyncCmd);
        }
    } else if (g_currentToken.tipo == TOKEN_IDENT) {
        // Pode ser atribuicao (ident := expressao) ou chamada de procedimento (ident <lista_arg>)
        // Para diferenciar, precisamos olhar o próximo token
        // Um 'peek' ou 'lookahead' seria ideal, mas com a API atual, vamos tentar casar.
        // Se falhar em ':=' e o próximo for '(', tentar lista_arg.
        // Para simplificar, vou assumir que o lexer já faz a diferenciação.

        // Tentativa 1: Atribuição
        Token nextToken = g_currentToken; // Salva o token atual
        // Não é ideal fazer um lookahead "manual" sem a capacidade do lexer.
        // Vamos tentar casar com a regra mais provável primeiro, que é :=
        // Se a gramática garante que ident := expressao é diferente de ident <lista_arg>,
        // então o token ATRIBUICAO ou PARENTESES_ESQ nos dirá qual caminho seguir.
        proximoToken(); // Avança para o próximo token para verificar
        if (g_currentToken.tipo == TOKEN_ATRIBUICAO) {
            g_currentToken = nextToken; // Volta o token para consumir 'ident' corretamente
            consumirToken(TOKEN_IDENT); // Consome 'ident'
            consumirToken(TOKEN_ATRIBUICAO); // Consome ':='
            expressao(); // Expressão
        } else { // Se não for ':=', deve ser ident ( <lista_arg> ) ou apenas ident (que não é um comando válido por si só)
            // Se o token atual for '(', então é uma lista de argumentos (chamada de procedimento sem 'CALL')
            // Ou se for um 'ident' e o próximo não for ':=', e for um '(', então é uma chamada.
            // A gramática diz 'ident <lista_arg>', então precisamos do '(' para a lista_arg.
            g_currentToken = nextToken; // Volta o token para consumir 'ident' corretamente
            consumirToken(TOKEN_IDENT); // Consome 'ident'
            listaArg(); // Analisa lista de argumentos (pode ser lambda)
            // A regra 20.6 é `ident <lista_arg>`. Se a lista_arg for lambda, significa `ident`.
            // Aparentemente, a chamada de procedimento pode ser apenas `ident` sem parênteses.
            // Para `call ident` teríamos `CALL ident`.
            // Se `ident <lista_arg>` é uma chamada de procedimento, e `lista_arg` pode ser `λ`,
            // então `ident` sozinho é um comando de chamada de procedimento sem argumentos.
            // Ex: `nomep;` (se `nomep` for um proc sem args)
            // No entanto, o exemplo 2 tem `nomep(b);` então `lista_arg` deve ser `(args)`
            // Se `lista_arg` é lambda, ela não consome o '('.
            // Se o lexer retornou IDENT, e o próximo token não é ATRIBUICAO, e é '(' então é chamada.
            // O caso `ident <lista_arg>` cobre a chamada de procedimento `call ident` da PL/0 padrão.
            // Se `lista_arg` pode ser `λ`, então `ident` sozinho é um comando.
            // Para lidar com `ident := <expressão>` e `ident <lista_arg>`, precisamos de lookahead.
            // Como não temos um mecanismo de lookahead no parser, vamos fazer uma pequena adaptação:
            // Se for `IDENT` e não `:=`, assume-se que é uma chamada sem `CALL`, e verifica `(`.
            // Se não tiver `(`, e não for `:=`, então é um erro ou um comando vazio.
            // Para agora, vamos assumir que `ident <lista_arg>` sempre terá `(`.
            // Se `ident` sem `(` for um comando, ele é implicitamente uma chamada de procedimento.
            // O exemplo `nomep(b);` mostra que `lista_arg` não é `λ` para `nomep`.
            // A gramática de `lista_arg` diz `( <argumentos> ) | λ`.
            // Isso significa que `ident()` é válido e `ident` é válido se `λ` for escolhido.
            // Então, a lógica de cima, que tenta `:=` primeiro, e depois `lista_arg` é a correta.
            // Se `g_currentToken` for `IDENT` e o próximo token não for `:=`,
            // e o token seguinte for `(`, então chama `listaArg()`.
            // Caso contrário, é um erro ou um comando `ident` sem `(`.
            // Para o exemplo `nomep(b);`, a chamada `listaArg()` funcionaria.
        }
    } else if (g_currentToken.tipo == TOKEN_BEGIN) {
        consumirToken(TOKEN_BEGIN); // Consome 'begin'
        comandos(); // Lista de comandos
        if (g_currentToken.tipo == TOKEN_END) {
            consumirToken(TOKEN_END); // Consome 'end'
        } else {
            erroSintatico("Esperado 'end' em comando composto.");
            recuperarErro(syncCmd, numSyncCmd);
        }
    } else if (g_currentToken.tipo == TOKEN_FOR) { // NOVO COMANDO FOR
        consumirToken(TOKEN_FOR); // Consome 'for'
        if (g_currentToken.tipo == TOKEN_IDENT) {
            consumirToken(TOKEN_IDENT); // Consome identificador da variável de controle
            if (g_currentToken.tipo == TOKEN_ATRIBUICAO) {
                consumirToken(TOKEN_ATRIBUICAO); // Consome ':='
                expressao(); // Expressão inicial
                if (g_currentToken.tipo == TOKEN_TO) {
                    consumirToken(TOKEN_TO); // Consome 'to'
                    expressao(); // Expressão final
                    if (g_currentToken.tipo == TOKEN_DO) {
                        consumirToken(TOKEN_DO); // Consome 'do'
                        cmd(); // Comando do for
                    } else {
                        erroSintatico("Esperado 'do' em comando for.");
                        recuperarErro(syncCmd, numSyncCmd);
                    }
                } else {
                    erroSintatico("Esperado 'to' em comando for.");
                    recuperarErro(syncCmd, numSyncCmd);
                }
            } else {
                erroSintatico("Esperado ':=' em comando for.");
                recuperarErro(syncCmd, numSyncCmd);
            }
        } else {
            erroSintatico("Esperado identificador em comando for.");
            recuperarErro(syncCmd, numSyncCmd);
        }
    } else {
        // Se chegar aqui, é um token que não inicia um comando válido.
        // Pode ser um comando vazio (λ) ou um erro.
        // Se o token atual estiver em um "follow" set de comandos, é um λ.
        int isFollowOfCmd = 0;
        for (int i = 0; i < numSyncCmd; i++) {
            if (g_currentToken.tipo == syncCmd[i]) {
                isFollowOfCmd = 1;
                break;
            }
        }
        if (!isFollowOfCmd && g_currentToken.tipo != TOKEN_EOF) {
            erroSintatico("Comando inesperado ou incompleto.");
            recuperarErro(syncCmd, numSyncCmd);
        }
    }
}

// Regra 21: <condicao> ::= <expressao> <relacao> <expressao>
void condicao() {
    // Sincronização para depois da condição (then, do, ))
    TipoToken syncCond[] = {TOKEN_THEN, TOKEN_DO, TOKEN_PARENTESES_DIR, TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_EOF};
    int numSyncCond = sizeof(syncCond) / sizeof(TipoToken);

    expressao(); // Primeira expressão
    relacao(); // Operador relacional
    expressao(); // Segunda expressão
}

// Regra 22: <relacao> ::= = | <> | >= | <= | > | <
void relacao() {
    // Sincronização para depois da relação (primeiros tokens de expressão)
    TipoToken syncRel[] = {TOKEN_IDENT, TOKEN_NUMERO_INT, TOKEN_NUMERO_REAL, TOKEN_PARENTESES_ESQ, TOKEN_MAIS, TOKEN_MENOS, TOKEN_EOF};
    int numSyncRel = sizeof(syncRel) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_IGUAL) {
        consumirToken(TOKEN_IGUAL);
    } else if (g_currentToken.tipo == TOKEN_DIFERENTE) {
        consumirToken(TOKEN_DIFERENTE);
    } else if (g_currentToken.tipo == TOKEN_MAIOR_IGUAL) {
        consumirToken(TOKEN_MAIOR_IGUAL);
    } else if (g_currentToken.tipo == TOKEN_MENOR_IGUAL) {
        consumirToken(TOKEN_MENOR_IGUAL);
    } else if (g_currentToken.tipo == TOKEN_MAIOR) {
        consumirToken(TOKEN_MAIOR);
    } else if (g_currentToken.tipo == TOKEN_MENOR) {
        consumirToken(TOKEN_MENOR);
    } else {
        erroSintatico("Esperado operador relacional.");
        recuperarErro(syncRel, numSyncRel);
    }
}

// Regra 23: <expressao> ::= <termo> <outros_termos>
void expressao() {
    termo(); // Primeiro termo
    outrosTermos(); // Outros termos (opcional)
}

// Regra 24: <op_un> ::= + | - | λ
void opUn() {
    if (g_currentToken.tipo == TOKEN_MAIS || g_currentToken.tipo == TOKEN_MENOS) {
        proximoToken(); // Consome o operador unário
    }
    // else { λ }
}

// Regra 25: <outros_termos> ::= <op_ad> <termo> <outros_termos> | λ
void outrosTermos() {
    if (g_currentToken.tipo == TOKEN_MAIS || g_currentToken.tipo == TOKEN_MENOS) {
        opAd(); // Operador de adição/subtração
        termo(); // Termo
        outrosTermos(); // Recursivo para mais termos
    }
    // else { λ }
}

// Regra 26: <op_ad> ::= + | -
void opAd() {
    TipoToken syncOpAd[] = {TOKEN_IDENT, TOKEN_NUMERO_INT, TOKEN_NUMERO_REAL, TOKEN_PARENTESES_ESQ, TOKEN_EOF};
    int numSyncOpAd = sizeof(syncOpAd) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_MAIS) {
        consumirToken(TOKEN_MAIS);
    } else if (g_currentToken.tipo == TOKEN_MENOS) {
        consumirToken(TOKEN_MENOS);
    } else {
        erroSintatico("Esperado '+' ou '-' como operador aditivo.");
        recuperarErro(syncOpAd, numSyncOpAd);
    }
}

// Regra 27: <termo> ::= <op_un> <fator> <mais_fatores>
void termo() {
    opUn(); // Operador unário (opcional)
    fator(); // Fator
    maisFatores(); // Mais fatores (opcional)
}

// Regra 28: <mais_fatores> ::= <op_mul> <fator> <mais_fatores> | λ
void maisFatores() {
    if (g_currentToken.tipo == TOKEN_MULTIPLICACAO || g_currentToken.tipo == TOKEN_DIVISAO) {
        opMul(); // Operador de multiplicação/divisão
        fator(); // Fator
        maisFatores(); // Recursivo para mais fatores
    }
    // else { λ }
}

// Regra 29: <op_mul> ::= * | /
void opMul() {
    TipoToken syncOpMul[] = {TOKEN_IDENT, TOKEN_NUMERO_INT, TOKEN_NUMERO_REAL, TOKEN_PARENTESES_ESQ, TOKEN_EOF};
    int numSyncOpMul = sizeof(syncOpMul) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_MULTIPLICACAO) {
        consumirToken(TOKEN_MULTIPLICACAO);
    } else if (g_currentToken.tipo == TOKEN_DIVISAO) {
        consumirToken(TOKEN_DIVISAO);
    } else {
        erroSintatico("Esperado '*' ou '/' como operador multiplicativo.");
        recuperarErro(syncOpMul, numSyncOpMul);
    }
}

// Regra 30: <fator> ::= ident | <numero> | ( <expressao> )
void fator() {
    // Sincronização para depois de um fator (+, -, *, /, =, <>, etc)
    TipoToken syncFator[] = {
        TOKEN_MAIS, TOKEN_MENOS, TOKEN_MULTIPLICACAO, TOKEN_DIVISAO,
        TOKEN_IGUAL, TOKEN_DIFERENTE, TOKEN_MENOR, TOKEN_MENOR_IGUAL,
        TOKEN_MAIOR, TOKEN_MAIOR_IGUAL, TOKEN_THEN, TOKEN_DO,
        TOKEN_PONTO_E_VIRGULA, TOKEN_END, TOKEN_PONTO, TOKEN_ELSE, TOKEN_PARENTESES_DIR, TOKEN_EOF
    };
    int numSyncFator = sizeof(syncFator) / sizeof(TipoToken);

    if (g_currentToken.tipo == TOKEN_IDENT) {
        consumirToken(TOKEN_IDENT);
    } else if (g_currentToken.tipo == TOKEN_NUMERO_INT || g_currentToken.tipo == TOKEN_NUMERO_REAL) {
        proximoToken(); // Consome o número
    } else if (g_currentToken.tipo == TOKEN_PARENTESES_ESQ) {
        consumirToken(TOKEN_PARENTESES_ESQ); // Consome '('
        expressao(); // Expressão interna
        if (g_currentToken.tipo == TOKEN_PARENTESES_DIR) {
            consumirToken(TOKEN_PARENTESES_DIR); // Consome ')'
        } else {
            erroSintatico("Esperado ')' em fator.");
            recuperarErro(syncFator, numSyncFator);
        }
    } else {
        erroSintatico("Esperado identificador, numero ou '(' em fator.");
        recuperarErro(syncFator, numSyncFator);
    }
}

// Função principal para iniciar a análise sintática.
void analisarSintaticamente() {
    programa(); // Inicia a análise sintática a partir do não-terminal inicial <programa>

    // Após a análise do programa, se o token atual for EOF, a compilação foi bem-sucedida.
    // Caso contrário, significa que houve erros sintáticos não recuperados ou caracteres extras.
    if (g_currentToken.tipo == TOKEN_EOF) {
        fprintf(g_textSaida, "Compilacao teve sucesso!\n");
    } else {
        fprintf(g_textSaida, "Compilacao terminou com erros.\n");
    }
}
