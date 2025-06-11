// lexico.c

#include "lexico.h"

// Definindo a variável global current_token UMA VEZ
Token current_token;

// Definindo a variável global palReservadas UMA VEZ
char palReservadas[20][20]= {
    "CONST", "VAR", "PROCEDURE", "CALL","BEGIN","END","IF","THEN", "WHILE","DO", "ODD"
};

// --- Implementação das funções léxicas auxiliares (re-adaptadas) ---
// Estas funções NÃO devem imprimir diretamente no textSaida.
// Elas devem apenas consumir os caracteres do textFile e retornar um status
// ou preencher um Token* passado como parâmetro.

// Função para reconhecer e tratar comentários iniciados com '{' e encerrados com '}'
// Retorna ENCONTRADO se um comentário válido for pulado.
// Retorna NAO_ENCONTRADO se não for um '{'.
// Altera *boolErro se houver um comentário não fechado.
int comentario(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
    int c;
    long initial_pos = ftell(textFile); // Salva a posição inicial

    c = fgetc(textFile);
    if (c == EOF) return FIM_DE_ARQUIVO; // Se for EOF, não há comentário

    if (c != '{') {
        fseek(textFile, initial_pos, SEEK_SET); // Volta se não for '{'
        return NAO_ENCONTRADO;
    }

    // Se chegou aqui, é um '{', então estamos dentro de um comentário.
    int found_closing_brace = 0;
    while ((c = fgetc(textFile)) != EOF) {
        if (c == '}') {
            found_closing_brace = 1;
            break; // Comentário fechado
        }
    }

    if (!found_closing_brace) {
        // Comentário não fechado até o fim do arquivo. Erro léxico.
        *boolErro = 1; // Marca o erro
        fprintf(textSaida, "Erro léxico: Comentário não fechado. '%s'\n", "COMENTARIO_NAO_FECHADO"); // Imprime o erro direto
        return ENCONTRADO; // ENCONTRADO no sentido de que "tratamos" a sequência (falhou)
    }

    // Comentário foi fechado com sucesso. Não é um token que o sintático se importa.
    fprintf(textSaida, "{...}, comentario\n"); // Apenas para log do léxico
    return ENCONTRADO;
}


// --- Implementação da nova getNextToken ---
// Esta é a função principal do analisador léxico agora.
void getNextToken(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
    int c;
    long initial_pos;

    // Pula espaços em branco e trata comentários
    while (1) {
        c = fgetc(textFile);
        if (c == EOF) {
            current_token.type = TOKEN_EOF;
            strcpy(current_token.lexeme, "EOF");
            return;
        }
        if (isspace(c)) {
            // Continua pulando espaços
            continue;
        } else if (c == '{') { // Possível início de comentário
            fseek(textFile, -1, SEEK_CUR); // Coloca o '{' de volta para a função `comentario`
            if (comentario(textFile, textSaida, boolErro, boolSpace) == ENCONTRADO) {
                // Comentário foi tratado (pulado ou erro reportado), continua procurando o próximo token
                continue;
            } else {
                // Se comentario retornou NAO_ENCONTRADO (não era '{'), saímos do loop
                // Isso não deve acontecer se a primeira checagem de `c` foi '{'.
                // Se chegou aqui, é um erro de lógica da `comentario` ou do loop.
                // Por segurança, vamos apenas sair e deixar o caractere original ser tratado.
                fseek(textFile, -1, SEEK_CUR); // Coloca o '{' de volta
                break;
            }
        } else {
            fseek(textFile, -1, SEEK_CUR); // Não é espaço nem '{', coloca o caractere de volta
            break; // Sai do loop para processar o caractere
        }
    }

    initial_pos = ftell(textFile); // Salva a posição antes de tentar reconhecer o token

    // Array de símbolos multi-caractere e relacionais
    SSimb relOpAssign[] = {
        {"<>", "simbolo_diferente", TOKEN_NEQ},
        {"<=", "simbolo_menor_igual", TOKEN_LE},
        {">=", "simbolo_maior_igual", TOKEN_GE},
        {":=", "simbolo_atribuicao", TOKEN_ASSIGN},
        {"=", "simbolo_igual", TOKEN_EQ},
        {">", "simbolo_maior", TOKEN_GT},
        {"<", "simbolo_menor", TOKEN_LT},
        // ":", "simbolo_dois_pontos", TOKEN_COLON  // Não na gramática PL/0
    };

    // Tentar reconhecer símbolos multi-caractere primeiro (ex: :=, <>, <=, >=)
    char lookahead[3] = {0}; // Buffer para 2 caracteres + \0
    int chars_read = fread(lookahead, sizeof(char), 2, textFile); // Tenta ler 2 caracteres para checar multi-char

    if (chars_read > 0) {
        for (int i = 0; i < sizeof(relOpAssign) / sizeof(SSimb); i++) {
            if (strlen(relOpAssign[i].simbolo) <= chars_read &&
                strncmp(relOpAssign[i].simbolo, lookahead, strlen(relOpAssign[i].simbolo)) == 0) {
                // Match encontrado
                fseek(textFile, strlen(relOpAssign[i].simbolo) - chars_read, SEEK_CUR); // Ajusta a posição do arquivo
                current_token.type = relOpAssign[i].token_type;
                strcpy(current_token.lexeme, relOpAssign[i].simbolo);
                fprintf(textSaida, "%s, %s\n", current_token.lexeme, getTokenTypeName(current_token.type));
                return;
            }
        }
    }
    fseek(textFile, initial_pos, SEEK_SET); // Volta à posição inicial se não achou match de 2 caracteres

    // Tentar identificar palavras reservadas e identificadores
    char ident_buffer[MAX_IDENT_LEN + 1] = {0};
    int idx = 0;
    c = fgetc(textFile);
    if (isalpha(c)) {
        ident_buffer[idx++] = c;
        while ((c = fgetc(textFile)) != EOF && isalnum(c)) {
            if (idx < MAX_IDENT_LEN) {
                ident_buffer[idx++] = c;
            } else {
                // Identificador muito longo, erro léxico
                strcpy(current_token.lexeme, ident_buffer);
                current_token.type = TOKEN_ERROR;
                *boolErro = 1;
                fprintf(textSaida, "%s, <ERRO_LEXICO> (Identificador muito longo)\n", current_token.lexeme);
                return;
            }
        }
        fseek(textFile, -1, SEEK_CUR); // Coloca o caractere não alfanumérico de volta
        ident_buffer[idx] = '\0';

        // Verificar se é palavra reservada
        for (int j = 0; j < sizeof(palReservadas) / sizeof(palReservadas[0]); j++) {
            if (strcmp(palReservadas[j], ident_buffer) == 0) {
                // É palavra reservada - use o mapeamento correto para TokenType
                // A ordem no enum TokenType e em palReservadas deve ser consistente.
                if (strcmp(ident_buffer, "CONST") == 0) current_token.type = TOKEN_CONST;
                else if (strcmp(ident_buffer, "VAR") == 0) current_token.type = TOKEN_VAR;
                else if (strcmp(ident_buffer, "PROCEDURE") == 0) current_token.type = TOKEN_PROCEDURE;
                else if (strcmp(ident_buffer, "CALL") == 0) current_token.type = TOKEN_CALL;
                else if (strcmp(ident_buffer, "BEGIN") == 0) current_token.type = TOKEN_BEGIN;
                else if (strcmp(ident_buffer, "END") == 0) current_token.type = TOKEN_END;
                else if (strcmp(ident_buffer, "IF") == 0) current_token.type = TOKEN_IF;
                else if (strcmp(ident_buffer, "THEN") == 0) current_token.type = TOKEN_THEN;
                else if (strcmp(ident_buffer, "WHILE") == 0) current_token.type = TOKEN_WHILE;
                else if (strcmp(ident_buffer, "DO") == 0) current_token.type = TOKEN_DO;
                else if (strcmp(ident_buffer, "ODD") == 0) current_token.type = TOKEN_ODD;
                // Adicione mais para outras palavras reservadas se houver
                strcpy(current_token.lexeme, ident_buffer);
                fprintf(textSaida, "%s, %s\n", current_token.lexeme, getTokenTypeName(current_token.type));
                return;
            }
        }
        // Se não é palavra reservada, é um identificador
        current_token.type = TOKEN_IDENT;
        strcpy(current_token.lexeme, ident_buffer);
        fprintf(textSaida, "%s, %s\n", current_token.lexeme, getTokenTypeName(current_token.type));
        return;
    }
    fseek(textFile, initial_pos, SEEK_SET); // Volta a posição se não começou com letra

    // Tentar números
    idx = 0;
    char num_buffer[MAX_IDENT_LEN + 1] = {0}; // Reutilizando MAX_IDENT_LEN para números
    c = fgetc(textFile);
    if (isdigit(c)) {
        num_buffer[idx++] = c;
        while ((c = fgetc(textFile)) != EOF && isdigit(c)) {
            if (idx < MAX_IDENT_LEN) {
                num_buffer[idx++] = c;
            } else {
                // Número muito longo
                strcpy(current_token.lexeme, num_buffer);
                current_token.type = TOKEN_ERROR;
                *boolErro = 1;
                fprintf(textSaida, "%s, <ERRO_LEXICO> (Número muito longo)\n", current_token.lexeme);
                return;
            }
        }
        fseek(textFile, -1, SEEK_CUR); // Coloca o caractere não dígito de volta
        num_buffer[idx] = '\0';
        current_token.type = TOKEN_NUMERO;
        strcpy(current_token.lexeme, num_buffer);
        fprintf(textSaida, "%s, %s\n", current_token.lexeme, getTokenTypeName(current_token.type));
        return;
    }
    fseek(textFile, initial_pos, SEEK_SET); // Volta a posição se não começou com dígito

    // Tentar outros símbolos de um caractere
    c = fgetc(textFile); // Lê um caractere para verificar
    if (c == EOF) {
        current_token.type = TOKEN_EOF;
        strcpy(current_token.lexeme, "EOF");
        return;
    }
    switch (c) {
        case '+': current_token.type = TOKEN_PLUS; strcpy(current_token.lexeme, "+"); break;
        case '-': current_token.type = TOKEN_MINUS; strcpy(current_token.lexeme, "-"); break;
        case '*': current_token.type = TOKEN_MULTIPLY; strcpy(current_token.lexeme, "*"); break;
        case '/': current_token.type = TOKEN_DIVIDE; strcpy(current_token.lexeme, "/"); break;
        case '(': current_token.type = TOKEN_LPAREN; strcpy(current_token.lexeme, "("); break;
        case ')': current_token.type = TOKEN_RPAREN; strcpy(current_token.lexeme, ")"); break;
        case ',': current_token.type = TOKEN_COMMA; strcpy(current_token.lexeme, ","); break;
        case ';': current_token.type = TOKEN_SEMICOLON; strcpy(current_token.lexeme, ";"); break;
        case '.': current_token.type = TOKEN_PERIOD; strcpy(current_token.lexeme, "."); break;
        default:
            // Caractere não reconhecido
            current_token.type = TOKEN_ERROR;
            current_token.lexeme[0] = c;
            current_token.lexeme[1] = '\0'; // Garante que é uma string válida
            *boolErro = 1; // Marca erro léxico
            fprintf(textSaida, "%s, <ERRO_LEXICO> (Caractere inválido)\n", current_token.lexeme);
            return;
    }
    fprintf(textSaida, "%s, %s\n", current_token.lexeme, getTokenTypeName(current_token.type));
}

// Helper para obter o nome do tipo de token
const char* getTokenTypeName(TokenType type) {
    switch (type) {
        case TOKEN_CONST: return "CONST";
        case TOKEN_VAR: return "VAR";
        case TOKEN_PROCEDURE: return "PROCEDURE";
        case TOKEN_CALL: return "CALL";
        case TOKEN_BEGIN: return "BEGIN";
        case TOKEN_END: return "END";
        case TOKEN_IF: return "IF";
        case TOKEN_THEN: return "THEN";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_DO: return "DO";
        case TOKEN_ODD: return "ODD";
        case TOKEN_IDENT: return "ident";
        case TOKEN_NUMERO: return "numero";
        case TOKEN_ASSIGN: return "simbolo_atribuicao";
        case TOKEN_EQ: return "simbolo_igual";
        case TOKEN_NEQ: return "simbolo_diferente";
        case TOKEN_LT: return "simbolo_menor";
        case TOKEN_LE: return "simbolo_menor_igual";
        case TOKEN_GT: return "simbolo_maior";
        case TOKEN_GE: return "simbolo_maior_igual";
        case TOKEN_PLUS: return "simbolo_mais";
        case TOKEN_MINUS: return "simbolo_menos";
        case TOKEN_MULTIPLY: return "simbolo_multiplicacao";
        case TOKEN_DIVIDE: return "simbolo_divisao";
        case TOKEN_LPAREN: return "parenteses_esquerdo";
        case TOKEN_RPAREN: return "parenteses_direito";
        case TOKEN_COMMA: return "simbolo_virgula";
        case TOKEN_SEMICOLON: return "simbolo_ponto_e_virgula";
        case TOKEN_PERIOD: return "simbolo_ponto";
        case TOKEN_COLON: return "simbolo_dois_pontos";
        case TOKEN_ERROR: return "ERRO_LEXICO";
        case TOKEN_EOF: return "FIM_DE_ARQUIVO";
        default: return "UNKNOWN_TOKEN_TYPE";
    }
}