#include "lexico.h"

// Variáveis globais definidas aqui para serem acessíveis externamente (via 'extern' em lexico.h)
FILE* g_textFile = NULL;
FILE* g_textSaida = NULL;
Token g_currentToken; // O token atual que o parser está analisando

// Lista de palavras reservadas da linguagem P-- (atualizada com "ELSE")
char palReservadas[20][20]= {
    "CONST", "VAR", "PROCEDURE", "CALL","BEGIN","END","IF","THEN", "WHILE","DO",
    "READ", "WRITE", "FOR", "TO", "REAL", "INTEGER", "PROGRAM", "ELSE" // "ELSE" adicionado aqui
};

// Inicializa os ponteiros de arquivo para o analisador léxico
void inicializarLexico(FILE* inputFile, FILE* outputFile) {
    g_textFile = inputFile;
    g_textSaida = outputFile;
}

// Tenta reconhecer e consumir um comentário.
// Se um '{' é encontrado, a função se compromete a consumir o comentário
// ou a reportar um erro léxico relacionado a um comentário malformado.
// Retorna ENCONTRADO se um comentário (válido ou malformado) for processado.
// Retorna NAO_ENCONTRADO caso contrário ou se o caractere inicial não for '{'.
// Retorna FIM_DE_ARQUIVO se EOF for atingido antes de ler o '{'.
int _comentario(Token* token) {
    int c;
    
    c = fgetc(g_textFile); 
    if (c == EOF) return FIM_DE_ARQUIVO;

    if (c != '{') {
        ungetc(c, g_textFile); // Coloca de volta se não for '{'
        return NAO_ENCONTRADO;
    }

    // Encontrou '{', então estamos processando um comentário (potencialmente malformado).
    int i = 0;
    if (i < MAX_IDENT_LEN - 1) token->lexema[i++] = '{';

    while ((c = fgetc(g_textFile)) != EOF) {
        if (i < MAX_IDENT_LEN - 1) token->lexema[i++] = c;

        if (c == '}') {
            token->lexema[i] = '\0'; // Finaliza a string do lexema
            token->tipo = TOKEN_COMENTARIO; // Define o tipo do token como comentário
            return ENCONTRADO; // Comentário válido, totalmente consumido
        }

        // Comentário malformado: quebra de linha antes de '}'.
        // O ponteiro do arquivo está AGORA APÓS o caractere de quebra de linha.
        // A regra do P-- diz que o comentário é de linha única, então quebra de linha é um erro.
        if (c == '\n' || c == '\r') {
            token->lexema[i] = '\0';
            token->tipo = TOKEN_ERRO_LEXICO; // Reporta como token de erro léxico
            return ENCONTRADO; // Retorna token de erro, mas consumiu a entrada problemática
        }
    }

    // EOF antes de '}'
    token->lexema[i] = '\0';
    token->tipo = TOKEN_ERRO_LEXICO; // Reporta como token de erro léxico
    return ENCONTRADO; // Retorna token de erro, mas consumiu a entrada problemática
}

// Função para reconhecer números inteiros ou reais
int _numero(Token* token) {
    char lido[MAX_IDENT_LEN + 1] = {0};
    int c;
    int i = 0;
    int is_real = 0;
    long initial_pos = ftell(g_textFile);

    c = fgetc(g_textFile);
    if (c == EOF) return FIM_DE_ARQUIVO;

    if (!isdigit(c)) {
        fseek(g_textFile, initial_pos, SEEK_SET);
        return NAO_ENCONTRADO;
    }

    lido[i++] = c;

    // Consome dígitos inteiros
    while ((c = fgetc(g_textFile)) != EOF) {
        if (isdigit(c)) {
            if (i < MAX_IDENT_LEN - 1) lido[i++] = c;
            else { // Lexema muito longo, erro
                lido[i] = '\0';
                strcpy(token->lexema, lido);
                token->tipo = TOKEN_ERRO_LEXICO;
                // Consome o resto dos dígitos para não reprocessá-los
                while (isdigit(c = fgetc(g_textFile)) && c != EOF);
                if (c != EOF) ungetc(c, g_textFile);
                return ENCONTRADO;
            }
        } else if (c == '.') {
            is_real = 1;
            if (i < MAX_IDENT_LEN - 1) lido[i++] = c;
            else { // Lexema muito longo, erro
                lido[i] = '\0';
                strcpy(token->lexema, lido);
                token->tipo = TOKEN_ERRO_LEXICO;
                // Consome o resto dos dígitos e o possível ponto
                while (isdigit(c = fgetc(g_textFile)) && c != EOF);
                if (c != EOF) ungetc(c, g_textFile);
                return ENCONTRADO;
            }
            break; // Sai do loop para começar a consumir a parte decimal
        } else {
            ungetc(c, g_textFile); // Coloca de volta o caractere não-dígito
            break;
        }
    }

    if (is_real) {
        // Agora, precisamos consumir a parte decimal (pelo menos um dígito)
        int digits_after_dot = 0;
        while ((c = fgetc(g_textFile)) != EOF) {
            if (isdigit(c)) {
                digits_after_dot = 1;
                if (i < MAX_IDENT_LEN - 1) lido[i++] = c;
                else { // Lexema muito longo, erro
                    lido[i] = '\0';
                    strcpy(token->lexema, lido);
                    token->tipo = TOKEN_ERRO_LEXICO;
                    // Consome o resto dos dígitos
                    while (isdigit(c = fgetc(g_textFile)) && c != EOF);
                    if (c != EOF) ungetc(c, g_textFile);
                    return ENCONTRADO;
                }
            } else {
                ungetc(c, g_textFile); // Coloca de volta o caractere não-dígito
                break;
            }
        }
        if (!digits_after_dot) { // Número real malformado (ex: "123.")
            fseek(g_textFile, initial_pos, SEEK_SET); // Volta ao início para reler e tentar identificar como int ou erro
            return NAO_ENCONTRADO; // Ou podemos retornar ENCONTRADO com TOKEN_ERRO_LEXICO aqui
        }
    }

    lido[i] = '\0';
    strcpy(token->lexema, lido);

    if (is_real) {
        token->tipo = TOKEN_NUMERO_REAL;
        token->valor_numerico_real = atof(lido); // Converte para double
    } else {
        token->tipo = TOKEN_NUMERO_INT;
        token->valor_numerico_int = atoi(lido); // Converte para int
    }
    return ENCONTRADO;
}


// Função para reconhecer identificadores (variáveis) ou palavras reservadas
int _identificador(Token* token) {
    char lido[MAX_IDENT_LEN + 1];
    int nLidos = 0;
    int c;
    long initial_pos = ftell(g_textFile);

    c = fgetc(g_textFile);
    if (c == EOF) return FIM_DE_ARQUIVO;

    if (!isalpha(c)) {
        fseek(g_textFile, initial_pos, SEEK_SET);
        return NAO_ENCONTRADO;
    }

    lido[nLidos++] = c;

    while (nLidos < MAX_IDENT_LEN) {
        c = fgetc(g_textFile);
        if (c == EOF) break;

        if (isalnum(c)) {
            lido[nLidos++] = c;
        } else {
            ungetc(c, g_textFile); // Use ungetc
            break;
        }
    }

    lido[nLidos] = '\0';
    strcpy(token->lexema, lido);

    // Agora são 18 palavras reservadas (0 a 17)
    for (int i = 0; i < 18; i++) {
        if (strcmp(palReservadas[i], lido) == 0) {
            switch (i) {
                case 0: token->tipo = TOKEN_CONST; break;
                case 1: token->tipo = TOKEN_VAR; break;
                case 2: token->tipo = TOKEN_PROCEDURE; break;
                case 3: token->tipo = TOKEN_CALL; break;
                case 4: token->tipo = TOKEN_BEGIN; break;
                case 5: token->tipo = TOKEN_END; break;
                case 6: token->tipo = TOKEN_IF; break;
                case 7: token->tipo = TOKEN_THEN; break;
                case 8: token->tipo = TOKEN_WHILE; break;
                case 9: token->tipo = TOKEN_DO; break;
                case 10: token->tipo = TOKEN_READ; break;
                case 11: token->tipo = TOKEN_WRITE; break;
                case 12: token->tipo = TOKEN_FOR; break;
                case 13: token->tipo = TOKEN_TO; break;
                case 14: token->tipo = TOKEN_REAL; break;
                case 15: token->tipo = TOKEN_INTEGER; break;
                case 16: token->tipo = TOKEN_PROGRAM; break;
                case 17: token->tipo = TOKEN_ELSE; break; // NOVO: case para ELSE
            }
            return ENCONTRADO;
        }
    }

    token->tipo = TOKEN_IDENT;
    return ENCONTRADO;
}

// Função auxiliar para reconhecer operadores de 1 ou 2 caracteres.
int _checarOperador(Token* token, const char* op1, TipoToken tipo1, const char* op2, TipoToken tipo2) {
    char lido[3] = {0}; // Buffer para ler 2 caracteres
    long initial_pos = ftell(g_textFile);
    
    // Tenta ler o máximo de caracteres para o operador maior
    size_t len_op2 = (op2 != NULL) ? strlen(op2) : 0;
    size_t len_op1 = strlen(op1);
    size_t chars_to_read = (len_op2 > len_op1) ? len_op2 : len_op1;

    // Garante que não lemos mais do que o buffer permite
    if (chars_to_read > 2) chars_to_read = 2; 

    // Lê os caracteres
    int nLidos = fread(lido, sizeof(char), chars_to_read, g_textFile);

    if (nLidos < 1) return FIM_DE_ARQUIVO;

    // Prioriza o operador de 2 caracteres (se existir)
    if (op2 != NULL && nLidos >= len_op2 && strncmp(op2, lido, len_op2) == 0) {
        fseek(g_textFile, len_op2 - nLidos, SEEK_CUR); // Ajusta o ponteiro do arquivo
        strcpy(token->lexema, op2);
        token->tipo = tipo2;
        return ENCONTRADO;
    }
    // Tenta o operador de 1 caractere
    if (nLidos >= len_op1 && strncmp(op1, lido, len_op1) == 0) {
        fseek(g_textFile, len_op1 - nLidos, SEEK_CUR); // Ajusta o ponteiro do arquivo
        strcpy(token->lexema, op1);
        token->tipo = tipo1;
        return ENCONTRADO;
    }

    fseek(g_textFile, initial_pos, SEEK_SET); // Volta se não encontrou nada
    return NAO_ENCONTRADO;
}

// Funções para operadores específicos usando _checarOperador
int _relacional(Token* token) {
    // Priorize operadores de 2 caracteres
    if (_checarOperador(token, "<", TOKEN_MENOR, "<>", TOKEN_DIFERENTE) == ENCONTRADO) return ENCONTRADO;
    if (_checarOperador(token, ">", TOKEN_MAIOR, ">=", TOKEN_MAIOR_IGUAL) == ENCONTRADO) return ENCONTRADO;
    if (_checarOperador(token, "=", TOKEN_IGUAL, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    if (_checarOperador(token, "<=", TOKEN_MENOR_IGUAL, NULL, 0) == ENCONTRADO) return ENCONTRADO; // Explicitly check for <=
    return NAO_ENCONTRADO;
}

int _atribuicao(Token* token) {
    // Priorize ':='
    return _checarOperador(token, ":=", TOKEN_ATRIBUICAO, NULL, 0);
}

int _operadorMaisMenos(Token* token) {
    if (_checarOperador(token, "+", TOKEN_MAIS, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    if (_checarOperador(token, "-", TOKEN_MENOS, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    return NAO_ENCONTRADO;
}

int _operadorDivMult(Token* token) {
    if (_checarOperador(token, "/", TOKEN_DIVISAO, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    if (_checarOperador(token, "*", TOKEN_MULTIPLICACAO, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    return NAO_ENCONTRADO;
}

int _ParentesesDireito(Token* token) {
    return _checarOperador(token, ")", TOKEN_PARENTESES_DIR, NULL, 0);
}

int _ParentesesEsquerdo(Token* token) {
    return _checarOperador(token, "(", TOKEN_PARENTESES_ESQ, NULL, 0);
}

int _operadorPontuacao(Token* token) {
    if (_checarOperador(token, ",", TOKEN_VIRGULA, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    if (_checarOperador(token, ";", TOKEN_PONTO_E_VIRGULA, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    if (_checarOperador(token, ".", TOKEN_PONTO, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    // O ':' sozinho deve ser verificado por último se ':= ' é possível.
    if (_checarOperador(token, ":", TOKEN_DOIS_PONTOS, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    return NAO_ENCONTRADO;
}


// Função principal do analisador léxico.
void proximoToken() {
    int c;
    while (1) {
        c = fgetc(g_textFile); // Lê um caractere

        if (c == EOF) {
            g_currentToken.tipo = TOKEN_EOF;
            strcpy(g_currentToken.lexema, "EOF");
            return;
        }

        if (isspace(c)) {
            continue; // Ignora espaços em branco e quebras de linha
        }

        ungetc(c, g_textFile); // Coloca de volta para as funções auxiliares

        // Ordem de precedência para reconhecimento de tokens (do mais complexo ao mais simples)
        // Comentários primeiro para serem ignorados.
        if (_comentario(&g_currentToken) == ENCONTRADO) {
            if (g_currentToken.tipo == TOKEN_COMENTARIO) {
                fprintf(g_textSaida, "%s, comentario\n", g_currentToken.lexema);
                continue; // Comentário válido, busca o próximo token real
            } else { 
                fprintf(g_textSaida, "ERRO LEXICO: %s, <Comentário malformado>\n", g_currentToken.lexema); // Comentário malformado
                return; // Retorna o token de erro para o parser
            }
        }
        
        // Operadores de 2 caracteres devem vir antes dos de 1 caractere que são prefixos.
        if (_atribuicao(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, simbolo_atribuicao\n", g_currentToken.lexema);
            return;
        }
        if (_relacional(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, ", g_currentToken.lexema);
            switch (g_currentToken.tipo) {
                case TOKEN_IGUAL: fprintf(g_textSaida, "simbolo_igual\n"); break;
                case TOKEN_DIFERENTE: fprintf(g_textSaida, "simbolo_diferente\n"); break;
                case TOKEN_MENOR: fprintf(g_textSaida, "simbolo_menor\n"); break;
                case TOKEN_MENOR_IGUAL: fprintf(g_textSaida, "simbolo_menor_igual\n"); break;
                case TOKEN_MAIOR: fprintf(g_textSaida, "simbolo_maior\n"); break;
                case TOKEN_MAIOR_IGUAL: fprintf(g_textSaida, "simbolo_maior_igual\n"); break;
                default: break; // Should not happen
            }
            return;
        }

        // Identificadores (incluindo palavras reservadas) e números
        if (_identificador(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, ", g_currentToken.lexema);
            switch(g_currentToken.tipo) {
                case TOKEN_IDENT: fprintf(g_textSaida, "ident\n"); break;
                case TOKEN_CONST: fprintf(g_textSaida, "const\n"); break;
                case TOKEN_VAR: fprintf(g_textSaida, "var\n"); break;
                case TOKEN_PROCEDURE: fprintf(g_textSaida, "procedure\n"); break;
                case TOKEN_CALL: fprintf(g_textSaida, "call\n"); break;
                case TOKEN_BEGIN: fprintf(g_textSaida, "begin\n"); break;
                case TOKEN_END: fprintf(g_textSaida, "end\n"); break;
                case TOKEN_IF: fprintf(g_textSaida, "if\n"); break;
                case TOKEN_THEN: fprintf(g_textSaida, "then\n"); break;
                case TOKEN_WHILE: fprintf(g_textSaida, "while\n"); break;
                case TOKEN_DO: fprintf(g_textSaida, "do\n"); break;
                case TOKEN_READ: fprintf(g_textSaida, "read\n"); break;
                case TOKEN_WRITE: fprintf(g_textSaida, "write\n"); break;
                case TOKEN_FOR: fprintf(g_textSaida, "for\n"); break;
                case TOKEN_TO: fprintf(g_textSaida, "to\n"); break;
                case TOKEN_REAL: fprintf(g_textSaida, "real\n"); break;
                case TOKEN_INTEGER: fprintf(g_textSaida, "integer\n"); break;
                case TOKEN_PROGRAM: fprintf(g_textSaida, "program\n"); break;
                case TOKEN_ELSE: fprintf(g_textSaida, "else\n"); break; // NOVO: impressão para ELSE
                default: fprintf(g_textSaida, "token_desconhecido\n"); break;
            }
            return;
        }
        if (_numero(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, ", g_currentToken.lexema);
            if (g_currentToken.tipo == TOKEN_NUMERO_INT) fprintf(g_textSaida, "numero_inteiro\n");
            else fprintf(g_textSaida, "numero_real\n");
            return;
        }
        
        // Operadores aritméticos
        if (_operadorMaisMenos(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, ", g_currentToken.lexema);
            if (g_currentToken.tipo == TOKEN_MAIS) fprintf(g_textSaida, "simbolo_mais\n");
            else fprintf(g_textSaida, "simbolo_menos\n");
            return;
        }
        if (_operadorDivMult(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, ", g_currentToken.lexema);
            if (g_currentToken.tipo == TOKEN_DIVISAO) fprintf(g_textSaida, "simbolo_divisao\n");
            else fprintf(g_textSaida, "simbolo_multiplicacao\n");
            return;
        }
        // Parênteses
        if (_ParentesesEsquerdo(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, parenteses_esquerdo\n", g_currentToken.lexema);
            return;
        }
        if (_ParentesesDireito(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, parenteses_direito\n", g_currentToken.lexema);
            return;
        }
        // Pontuação (o ':' sozinho, a vírgula, ponto e vírgula, ponto)
        if (_operadorPontuacao(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, ", g_currentToken.lexema);
            switch (g_currentToken.tipo) {
                case TOKEN_VIRGULA: fprintf(g_textSaida, "simbolo_virgula\n"); break;
                case TOKEN_PONTO_E_VIRGULA: fprintf(g_textSaida, "simbolo_ponto_e_virgula\n"); break;
                case TOKEN_PONTO: fprintf(g_textSaida, "simbolo_ponto\n"); break;
                case TOKEN_DOIS_PONTOS: fprintf(g_textSaida, "simbolo_dois_pontos\n"); break;
                default: break;
            }
            return;
        }

        // Se nenhum dos reconhecedores de token casou, é um caractere não reconhecido.
        fgetc(g_textFile); // CONSUME O CARACTERE NÃO RECONHECIDO para avançar o ponteiro
        g_currentToken.tipo = TOKEN_ERRO_LEXICO;
        g_currentToken.lexema[0] = (char)c; 
        g_currentToken.lexema[1] = '\0';
        fprintf(g_textSaida, "ERRO LEXICO: %s, <Caractere nao reconhecido>\n", g_currentToken.lexema);
        return;
    }
}
