#include "lexico.h"

// Variáveis globais definidas aqui para serem acessíveis externamente (via 'extern' em lexico.h)
FILE* g_textFile = NULL;
FILE* g_textSaida = NULL;
Token g_currentToken; // O token atual que o parser está analisando

// Lista de palavras reservadas da linguagem PL/0
char palReservadas[20][20]= {
    "CONST", "VAR", "PROCEDURE", "CALL","BEGIN","END","IF","THEN", "WHILE","DO"
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
    long initial_pos = ftell(g_textFile); // Guarda a posição inicial para possível backtracking

    c = fgetc(g_textFile); // Tenta ler o primeiro caractere (espera-se '{')
    if (c == EOF) return FIM_DE_ARQUIVO;

    if (c != '{') {
        fseek(g_textFile, initial_pos, SEEK_SET); // Não é início de comentário, volta
        return NAO_ENCONTRADO;
    }

    // É um '{', então estamos processando um comentário (válido ou malformado).
    // O ponteiro do arquivo já avançou.
    int i = 0;
    token->lexema[i++] = '{'; // Armazena o '{' no lexema do token

    while ((c = fgetc(g_textFile)) != EOF) {
        if (i < MAX_IDENT_LEN - 1) { // Garante espaço para o terminador nulo
            token->lexema[i++] = c;
        } else {
            // Buffer do lexema estourou para o conteúdo do comentário.
            // Continuamos lendo para encontrar '}' para consumir o comentário do stream,
            // mas o lexema será truncado.
        }

        if (c == '}') {
            token->lexema[i] = '\0'; // Finaliza a string do lexema
            token->tipo = TOKEN_COMENTARIO; // Define o tipo do token como comentário
            return ENCONTRADO; // Comentário válido encontrado e consumido
        }

        if (c == '\n' || c == '\r') {
            // Erro: encontrou uma quebra de linha antes da chave fechar.
            // O '{' e o conteúdo até a quebra de linha já foram consumidos.
            token->lexema[i] = '\0';
            token->tipo = TOKEN_ERRO_LEXICO; // Marca como erro léxico
            // O ponteiro do arquivo está no caractere de quebra de linha.
            // Não precisamos voltar, pois já consumimos a parte malformada.
            return ENCONTRADO; // Retorna ENCONTRADO para que proximoToken processe o erro
        }
    }

    // EOF antes de fechar o comentário -> erro.
    // O '{' e o conteúdo até EOF já foram consumidos.
    token->lexema[i] = '\0';
    token->tipo = TOKEN_ERRO_LEXICO; // Marca como erro léxico
    return ENCONTRADO; // Retorna ENCONTRADO para que proximoToken processe o erro
}

// Tenta reconhecer um número inteiro.
// Retorna ENCONTRADO se um número válido for encontrado.
// Retorna NAO_ENCONTRADO caso contrário.
// Retorna FIM_DE_ARQUIVO se EOF for atingido.
int _numero(Token* token) {
    char lido[MAX_IDENT_LEN] = {0}; // Buffer para o número lido
    int c;
    int i = 0;
    long initial_pos = ftell(g_textFile);

    c = fgetc(g_textFile);
    if (c == EOF) return FIM_DE_ARQUIVO;

    if (!isdigit(c)) {
        fseek(g_textFile, initial_pos, SEEK_SET); // Não é dígito, volta
        return NAO_ENCONTRADO;
    }

    lido[i++] = c; // Primeiro dígito

    // Continua lendo enquanto forem dígitos
    while ((c = fgetc(g_textFile)) != EOF) {
        if (!isdigit(c)) {
            fseek(g_textFile, -1, SEEK_CUR); // Não é dígito, volta 1 caractere
            break;
        }
        if (i < MAX_IDENT_LEN - 1) { // Verifica limite do buffer
            lido[i++] = c;
        } else {
            // Excedeu o tamanho do buffer -> trata como erro léxico
            lido[i] = '\0';
            strcpy(token->lexema, lido);
            token->tipo = TOKEN_ERRO_LEXICO; // Marca como erro léxico por tamanho
            return ENCONTRADO; // Retorna ENCONTRADO para que proximoToken() processe o erro
        }
    }
    lido[i] = '\0'; // Finaliza a string
    strcpy(token->lexema, lido);
    token->tipo = TOKEN_NUMERO;
    token->valor_numerico = atoi(lido); // Converte para inteiro
    return ENCONTRADO;
}

// Tenta reconhecer um identificador ou palavra reservada.
// Retorna ENCONTRADO se um identificador/palavra reservada for encontrado.
// Retorna NAO_ENCONTRADO caso contrário.
// Retorna FIM_DE_ARQUIVO se EOF for atingido.
int _identificador(Token* token) {
    char lido[MAX_IDENT_LEN + 1];
    int nLidos = 0;
    int c;
    long initial_pos = ftell(g_textFile);

    c = fgetc(g_textFile);
    if (c == EOF) return FIM_DE_ARQUIVO;

    if (!isalpha(c)) {
        fseek(g_textFile, initial_pos, SEEK_SET); // Não é letra, volta
        return NAO_ENCONTRADO;
    }

    lido[nLidos++] = c; // Primeira letra

    // Continua lendo enquanto forem letras ou dígitos
    while (nLidos < MAX_IDENT_LEN) {
        c = fgetc(g_textFile);
        if (c == EOF) break;

        if (isalnum(c)) {
            lido[nLidos++] = c;
        } else {
            fseek(g_textFile, -1, SEEK_CUR); // Não é alfanumérico, volta 1 caractere
            break;
        }
    }

    lido[nLidos] = '\0'; // Finaliza a string
    strcpy(token->lexema, lido);

    // Verifica se é uma palavra reservada
    for (int i = 0; i < 10; i++) { // Percorre a lista de palavras reservadas
        if (strcmp(palReservadas[i], lido) == 0) {
            // Se for palavra reservada, define o tipo de token correspondente
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
            }
            return ENCONTRADO; // Palavra reservada encontrada
        }
    }

    token->tipo = TOKEN_IDENT; // Se não for palavra reservada, é um identificador
    return ENCONTRADO;
}

// Função auxiliar para reconhecer operadores de 1 ou 2 caracteres.
// Tenta casar com op2 (2 caracteres) primeiro, depois com op1 (1 caractere).
int _checarOperador(Token* token, const char* op1, TipoToken tipo1, const char* op2, TipoToken tipo2) {
    char lido[3] = {0}; // Buffer para ler até 2 caracteres
    long initial_pos = ftell(g_textFile);
    int nLidos = fread(lido, sizeof(char), 2, g_textFile); // Tenta ler 2 caracteres

    if (nLidos < 1) return FIM_DE_ARQUIVO;

    // Tenta casar com o operador de 2 caracteres primeiro, se op2 for fornecido
    if (op2 != NULL && nLidos >= strlen(op2) && strncmp(op2, lido, strlen(op2)) == 0) {
        fseek(g_textFile, strlen(op2) - nLidos, SEEK_CUR); // Ajusta o ponteiro do arquivo
        strcpy(token->lexema, op2);
        token->tipo = tipo2;
        return ENCONTRADO;
    }
    // Tenta casar com o operador de 1 caractere
    if (nLidos >= strlen(op1) && strncmp(op1, lido, strlen(op1)) == 0) {
        fseek(g_textFile, strlen(op1) - nLidos, SEEK_CUR); // Ajusta o ponteiro do arquivo
        strcpy(token->lexema, op1);
        token->tipo = tipo1;
        return ENCONTRADO;
    }

    fseek(g_textFile, initial_pos, SEEK_SET); // Volta se não casou com nenhum
    return NAO_ENCONTRADO;
}

// Funções para operadores específicos usando _checarOperador
int _relacional(Token* token) {
    if (_checarOperador(token, "<", TOKEN_MENOR, "<>", TOKEN_DIFERENTE) == ENCONTRADO) return ENCONTRADO;
    if (_checarOperador(token, ">", TOKEN_MAIOR, ">=", TOKEN_MAIOR_IGUAL) == ENCONTRADO) return ENCONTRADO;
    if (_checarOperador(token, "=", TOKEN_IGUAL, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    return NAO_ENCONTRADO;
}

int _atribuicao(Token* token) {
    // Primeiro tenta casar com ":=", se não, tenta com ":"
    // Note: O ':' sozinho é tratado como um operador de pontuação.
    // Aqui, ele só será reconhecido se for parte de ':='.
    return _checarOperador(token, ":", TOKEN_DOIS_PONTOS, ":=", TOKEN_ATRIBUICAO);
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
    // O dois pontos sozinho (':') é tratado aqui, se não foi parte de ':='
    if (_checarOperador(token, ":", TOKEN_DOIS_PONTOS, NULL, 0) == ENCONTRADO) return ENCONTRADO;
    return NAO_ENCONTRADO;
}

// Função principal do analisador léxico.
// Lê o próximo token do arquivo de entrada, o classifica e atualiza g_currentToken.
// Lida com espaços em branco e caracteres não reconhecidos, garantindo o avanço do ponteiro do arquivo.
void proximoToken() {
    int c;
    while (1) {
        c = fgetc(g_textFile); // Lê um caractere do arquivo

        if (c == EOF) {
            g_currentToken.tipo = TOKEN_EOF; // Fim de arquivo
            strcpy(g_currentToken.lexema, "EOF");
            return;
        }

        // Ignora espaços em branco (incluindo quebras de linha)
        if (isspace(c)) {
            continue; // Continua para ler o próximo caractere
        }

        // Coloca o caractere lido de volta no stream para que as funções de reconhecimento possam lê-lo
        ungetc(c, g_textFile);

        // Tenta casar o caractere/sequência com os diferentes tipos de tokens
        // A ordem é importante para operadores de múltiplos caracteres e comentários.
        // Comentários devem ser tratados primeiro para serem ignorados ou reportados como erro.
        if (_comentario(&g_currentToken) == ENCONTRADO) {
            if (g_currentToken.tipo == TOKEN_COMENTARIO) {
                fprintf(g_textSaida, "%s, comentario\n", g_currentToken.lexema);
                continue; // Comentário válido, busca o próximo token real
            } else { // É um TOKEN_ERRO_LEXICO retornado por _comentario (comentário malformado)
                fprintf(g_textSaida, "%s, <ERRO_LEXICO>\n", g_currentToken.lexema);
                return; // Retorna o token de erro para o parser
            }
        }
        // Atribuição (:=) deve vir antes de relacional (=) ou pontuação (:)
        if (_atribuicao(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, simbolo_atribuicao\n", g_currentToken.lexema);
            return;
        }
        // Relacionais com dois caracteres (<=, >=, <>) devem vir antes dos de um (<, >)
        if (_relacional(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, ", g_currentToken.lexema);
            switch (g_currentToken.tipo) {
                case TOKEN_IGUAL: fprintf(g_textSaida, "simbolo_igual\n"); break;
                case TOKEN_DIFERENTE: fprintf(g_textSaida, "simbolo_diferente\n"); break;
                case TOKEN_MENOR: fprintf(g_textSaida, "simbolo_menor\n"); break;
                case TOKEN_MENOR_IGUAL: fprintf(g_textSaida, "simbolo_menor_igual\n"); break;
                case TOKEN_MAIOR: fprintf(g_textSaida, "simbolo_maior\n"); break;
                case TOKEN_MAIOR_IGUAL: fprintf(g_textSaida, "simbolo_maior_igual\n"); break;
                default: break; // Não deve acontecer
            }
            return;
        }
        if (_identificador(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, ", g_currentToken.lexema);
            if (g_currentToken.tipo == TOKEN_IDENT) fprintf(g_textSaida, "ident\n");
            else fprintf(g_textSaida, "%s\n", g_currentToken.lexema); // Palavra reservada
            return;
        }
        if (_numero(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, numero\n", g_currentToken.lexema);
            return;
        }
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
        if (_ParentesesEsquerdo(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, parenteses_esquerdo\n", g_currentToken.lexema);
            return;
        }
        if (_ParentesesDireito(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, parenteses_direito\n", g_currentToken.lexema);
            return;
        }
        // Pontuação (:) deve vir depois de atribuição (:=)
        if (_operadorPontuacao(&g_currentToken) == ENCONTRADO) {
            fprintf(g_textSaida, "%s, ", g_currentToken.lexema);
            switch (g_currentToken.tipo) {
                case TOKEN_VIRGULA: fprintf(g_textSaida, "simbolo_virgula\n"); break;
                case TOKEN_PONTO_E_VIRGULA: fprintf(g_textSaida, "simbolo_ponto_e_virgula\n"); break;
                case TOKEN_PONTO: fprintf(g_textSaida, "simbolo_ponto\n"); break;
                case TOKEN_DOIS_PONTOS: fprintf(g_textSaida, "simbolo_dois_pontos\n"); break;
                default: break; // Não deve acontecer
            }
            return;
        }

        // Se nenhum dos reconhecedores de token casou, é um caractere não reconhecido.
        // O caractere 'c' (que foi ungetc'd) ainda está no stream.
        // Precisamos consumi-lo agora e reportar o erro.
        fgetc(g_textFile); // Consome o caractere problemático para avançar o ponteiro
        g_currentToken.tipo = TOKEN_ERRO_LEXICO;
        g_currentToken.lexema[0] = (char)c; // O caractere que não foi reconhecido
        g_currentToken.lexema[1] = '\0';
        fprintf(g_textSaida, "%s, <ERRO_LEXICO>\n", g_currentToken.lexema);
        return; // Retorna o token de erro
    }
}
