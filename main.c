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

// Estrutura que representa símbolos e seus respectivos nomes (denominadores)
typedef struct {
    char simbolo[3];
    char denominador[MAX_DEN];
}SSimb;

// Lista de palavras reservadas da linguagem
char palReservadas[20][20]= {
    "CONST", "VAR", "PROCEDURE", "CALL","BEGIN","END","IF","THEN", "WHILE","DO"
};


// Função para reconhecer e tratar comentários iniciados com '{' e encerrados com '}'
int comentario(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
    int c;
    int encontrouAbertura = 0;

    c = fgetc(textFile);
    if (c == EOF) return FIM_DE_ARQUIVO;

    if (c != '{') {
        // Não é início de comentário, voltar e sair
        fseek(textFile, -1, SEEK_CUR);
        return NAO_ENCONTRADO;
    }

    encontrouAbertura = 1;
    fprintf(textSaida, "{");  // Início do comentário

    while ((c = fgetc(textFile)) != EOF) {
        fputc(c, textSaida);  // Imprime o conteúdo do comentário

        if (c == '}') {
            // Comentário fechado corretamente
            if (*boolErro == 1) {
                fprintf(textSaida, ", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            } else {
                fprintf(textSaida, ", comentario\n");
            }
            return ENCONTRADO;
        }

        if (c == '\n') {
            // Erro: encontrou uma quebra de linha antes da chave fechar
            if (*boolErro == 1) {
                fprintf(textSaida, ", <ERRO_LEXICO>\n");
            } else {
                fprintf(textSaida, ", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            }
            return NAO_ENCONTRADO;
        }
    }

    // EOF antes de fechar o comentário -> erro
    if (*boolErro == 1) {
        fprintf(textSaida, ", <ERRO_LEXICO>\n");
    } else {
        fprintf(textSaida, ", <ERRO_LEXICO>\n");
        *boolErro = 0;
        *boolSpace = 0;
    }
    return NAO_ENCONTRADO;
}

// Função para reconhecer números inteiros válidos
int numero(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
    char lido[100] = {0};  // Buffer para guardar o número lido (tamanho arbitrário)
    int c;
    int i = 0;

    c = fgetc(textFile);
    if (c == EOF) return FIM_DE_ARQUIVO;

    if (!isdigit(c)) {
        // Não é número, volta e sai
        fseek(textFile, -1, SEEK_CUR);
        return NAO_ENCONTRADO;
    }

    // Primeiro dígito
    lido[i++] = c;

    // Continua lendo enquanto forem dígitos
    while ((c = fgetc(textFile)) != EOF) {
        if (!isdigit(c)) {
            fseek(textFile, -1, SEEK_CUR);  // Não é dígito, volta 1
            break;
        }
        if (i < sizeof(lido) - 1) {
            lido[i++] = c;
        } else {
            // Excedeu o tamanho do buffer -> trata como erro léxico
            fprintf(textSaida, "%s, <ERRO_LEXICO>\n", lido);
            *boolErro = 0;
            *boolSpace = 0;
            return ENCONTRADO;
        }
    }
    lido[i] = '\0';  // Finaliza string

    if (*boolErro == 1) {
        fprintf(textSaida, "%s, <ERRO_LEXICO>\n", lido);
        *boolErro = 0;
        *boolSpace = 0;
    } else {
        fprintf(textSaida, "%s, numero\n", lido);
    }
    return ENCONTRADO;
}

// Função para reconhecer identificadores (variáveis) ou palavras reservadas
int identificador(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
    char lido[MAX_IDENT_LEN + 1]; // +1 para o \0
    int nLidos = 0;
    int c;

    // Ler o primeiro caractere e checar se é letra
    c = fgetc(textFile);
    if (c == EOF) return FIM_DE_ARQUIVO;

    if (!isalpha(c)) {
        // Não é identificador, voltar 1 byte e sair
        fseek(textFile, -1, SEEK_CUR);
        return NAO_ENCONTRADO;
    }

    // É uma letra: começar a formar o identificador
    lido[nLidos++] = c;

    while (nLidos < MAX_IDENT_LEN) {
        c = fgetc(textFile);
        if (c == EOF) break;

        if (isalnum(c)) {
            lido[nLidos++] = c;
        } else {
            // Encontrou algo que não é letra nem dígito -> voltar 1 byte
            fseek(textFile, -1, SEEK_CUR);
            break;
        }
    }

    lido[nLidos] = '\0'; // Terminar string

    for (int i = 0; i < 10; i++) { // Aqui sabemos que são 10 palavras
        if (strcmp(palReservadas[i], lido) == 0) {
            // É uma palavra reservada -> não é identificador
           // fseek (textFile, -strlen(palReservadas[i])-1, SEEK_CUR);
            if (*boolErro == 1) {
                fprintf(textSaida, ", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            }
             fprintf (textSaida, "%s, %s \n", palReservadas[i], palReservadas[i]);
            return NAO_ENCONTRADO;
        }
    }

    if (*boolErro == 1) {
        fprintf(textSaida, ", <ERRO_LEXICO>\n");
        *boolErro = 0;
        *boolSpace = 0;
    }
    fprintf(textSaida, "%s, ident\n", lido);
    return ENCONTRADO;
}

// Função para reconhecer simbolos
// Lê até dois caracteres do arquivo e compara com os símbolos esperados.
// Se encontrar um dos operadores definidos, escreve no arquivo de saída com seu nome descritivo.
// Caso contrário, retorna NAO_ENCONTRADO e desfaz a leitura dos caracteres.
int simbolos(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
/* Nesse vetor, são utilizados símbolos de, no máximo, 2 caracteres.
   Além disso, deve estar em ordem decrescente de caracteres para que a
   função funcione.*/  
    SSimb simbolos[] = {
        {"<>", "simbolo_diferente"},
        {"<=", "simbolo_menor_igual"},
        {">=", "simbolo_maior_igual"},
        {"=", "simbolo_igual"},
        {">", "simbolo_maior"},
        {"<", "simbolo_menor"},
        {": ", "simbolo_dois_pontos"},
        {":=", "simbolo_atribuicao"},
        {"+", "simbolo_mais"},
        {"-", "simbolo_menos"},
        {",", "simbolo_virgula"},
        {";", "simbolo_ponto_e_virgula"},
        {".", "simbolo_ponto"},
        {"/", "simbolo_divisao"},
        {"*", "simbolo_multiplicacao"},
        {")", "parenteses_direito"},
        {"(", "parenteses_esquerdo"},
        {".", "simbolo_ponto"}

    };

    char lido[3] = {0}; // Initialize all elements to zero
    int nLidos = 0;

    //fprintf(textSaida, "ENTRANDO COM (%ld)\n", ftell(textFile));

    nLidos = fread(lido, sizeof(char), 2, textFile); // Lê símbolos de até 2 caracteres.
    if (nLidos < 1) return FIM_DE_ARQUIVO;

    //fprintf(textSaida, " nLidos = %d, (%s), ftell = %ld\n", nLidos, lido, ftell(textFile));

    for (int i = 0; i < sizeof(simbolos) / sizeof(simbolos[0]); i++) {
        int simbolo_len = strlen(simbolos[i].simbolo);
        if (strncmp(simbolos[i].simbolo, lido, simbolo_len) == 0) {
            // Se encontramos um simbolo que é menor ou igual a quantidade de caracteres lida:
            if (nLidos >= simbolo_len) {
                fseek(textFile, simbolo_len - nLidos, SEEK_CUR);
            } 
           // fprintf(textSaida, " boolErro = %d \n", *boolErro);
            if (*boolErro == 1) {
                fprintf(textSaida, ", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            }
            fprintf(textSaida, "%s, %s\n", simbolos[i].simbolo, simbolos[i].denominador);
            return ENCONTRADO;
        }
    }

    fseek(textFile, -nLidos, SEEK_CUR);
    //fprintf(textSaida, "SAINDO COM ftell = (%ld)\n", ftell(textFile));
    return NAO_ENCONTRADO;
}

//Função Principal
int main(){

int teste;
char entrada[] = "testeX.txt";
char saida[] = "saidaX.txt";

    for (teste = 1; teste < 1; teste++) {

        entrada[5] = '0' + teste;
        saida[5] = '0' + teste;

        FILE* TextFile = fopen(entrada, "rb");
        FILE* TextSaida = fopen(saida, "w");

        char erroLexico = 'e';
        int c, result, fimArq = 0, bytesLidos = 0;
        int boolErro = 0, boolSpace = 0;

        if (!TextFile) {
            fprintf(TextSaida, "Failed to open file");
            return 1;
        }

        while ((result = (simbolos(TextFile, TextSaida, &boolErro, &boolSpace) 
        * identificador(TextFile, TextSaida, &boolErro, &boolSpace)
        * comentario (TextFile, TextSaida, &boolErro, &boolSpace)
        * numero (TextFile, TextSaida, &boolErro,  &boolSpace))) % FIM_DE_ARQUIVO != 0) {  // Se não for fim de arquivo

        // ffprintf(textSaida, textSaida, " (%ld) ", ftell(TextFile));
            if (result % ENCONTRADO != 0){   //Se nenhum for encontrado

                fimArq = (fread(&erroLexico, sizeof(char), 1, TextFile) == 0);
                if (!isspace (erroLexico)){
                    boolErro = 1;
                    if (boolSpace == 1){
                        fprintf (TextSaida, ", <ERRO_LEXICO>\n");
                        boolSpace = 0;
                    }
                    fprintf (TextSaida, "%c", erroLexico);
                } else {
                    if (boolErro == 1) boolSpace = 1;
                }
                if (fimArq && boolErro) { 
                    fprintf (TextSaida, ", <ERRO_LEXICO>\n");
                    return 0;} 

                } 
        } 
        
        if (boolErro) {
            fprintf (TextSaida, ", <ERRO_LEXICO>\n");
        } 

        fclose(TextFile);
        fclose(TextSaida);
    }
     return 0;
}
