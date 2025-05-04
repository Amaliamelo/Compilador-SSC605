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

typedef struct {
    char simbolo[3];
    char denominador[MAX_DEN];
}SSimb;

char palReservadas[20][20]= {
    "CONST", "VAR", "PROCEDURE", "CALL","BEGIN","END","IF","THEN", "WHILE","DO"
};

/*int relacional(FILE* textFile, int* boolErro){

SSimb relacionais[] = {
    {"<>", "simbolo_diferente"},
    {"<=", "simbolo_menor_igual"},
    {">=", "simbolo_maior_igual"},
    {"=", "simbolo_igual"},
    {">", "simbolo_maior"},
    {"<", "simbolo_menor"}
};

char lido[3] = {0}, c;
lido[2] = '\0';
int nLidos = 0;


if ((nLidos = fread(lido, sizeof(char), 2, textFile)) < 1) return FIM_DE_ARQUIVO;

//printf(" nLidos = %d, (%s)\n", nLidos, lido);

for (int i = 0; i < sizeof(relacionais) / sizeof(relacionais[0]); i++){
   // printf ("Comparando (%s) com (%s).\n", relacionais[i].simbolo, lido);
    if (strncmp(relacionais[i].simbolo, lido, strlen(relacionais[i].simbolo)) == 0){
        fseek (textFile, strlen(relacionais[i].simbolo)-(nLidos), SEEK_CUR);
      //  printf("nLidos = %d strlen(relacionais[i].simbolo)) = %d \n", nLidos, strlen(relacionais[i].simbolo));
        if (*boolErro == 1){
            printf (", <ERRO_LEXICO>\n");
            *boolErro = 0;
        }
        printf("%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
        return ENCONTRADO;
    }
}

fseek (textFile, -nLidos, SEEK_CUR);
printf ("ftell = (%ld) \n", ftell(textFile));
return NAO_ENCONTRADO;

}*/

int comentario(FILE* textFile, int* boolErro, int* boolSpace) {
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
    printf("{");  // Início do comentário

    while ((c = fgetc(textFile)) != EOF) {
        putchar(c);  // Imprime o conteúdo do comentário

        if (c == '}') {
            // Comentário fechado corretamente
            if (*boolErro == 1) {
                printf(", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            } else {
                printf(", comentario\n");
            }
            return ENCONTRADO;
        }

        if (c == '\n') {
            // Erro: encontrou uma quebra de linha antes da chave fechar
            if (*boolErro == 1) {
                printf(", <ERRO_LEXICO>\n");
            } else {
                printf(", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            }
            return NAO_ENCONTRADO;
        }
    }

    // EOF antes de fechar o comentário -> erro
    if (*boolErro == 1) {
        printf(", <ERRO_LEXICO>\n");
    } else {
        printf(", <ERRO_LEXICO>\n");
        *boolErro = 0;
        *boolSpace = 0;
    }
    return NAO_ENCONTRADO;
}

int numero(FILE* textFile, int* boolErro, int* boolSpace) {
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
            printf("%s, <ERRO_LEXICO>\n", lido);
            *boolErro = 0;
            *boolSpace = 0;
            return ENCONTRADO;
        }
    }
    lido[i] = '\0';  // Finaliza string

    if (*boolErro == 1) {
        printf("%s, <ERRO_LEXICO>\n", lido);
        *boolErro = 0;
        *boolSpace = 0;
    } else {
        printf("%s, numero\n", lido);
    }
    return ENCONTRADO;
}

int identificador(FILE* textFile, int* boolErro, int* boolSpace) {
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
                printf(", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            }
             printf ("%s, %s \n", palReservadas[i], palReservadas[i]);
            return NAO_ENCONTRADO;
        }
    }

    if (*boolErro == 1) {
        printf(", <ERRO_LEXICO>\n");
        *boolErro = 0;
        *boolSpace = 0;
    }
    printf("%s, ident\n", lido);
    return ENCONTRADO;
}

int relacional(FILE* textFile, int* boolErro, int* boolSpace) {
    SSimb relacionais[] = {
        {"<>", "simbolo_diferente"},
        {"<=", "simbolo_menor_igual"},
        {">=", "simbolo_maior_igual"},
        {"=", "simbolo_igual"},
        {">", "simbolo_maior"},
        {"<", "simbolo_menor"}
    };

    char lido[3] = {0}; // Initialize all elements to zero
    int nLidos = 0;

    //printf ("ENTRANDO COM (%ld)\n", ftell(textFile));

    nLidos = fread(lido, sizeof(char), 2, textFile);
    if (nLidos < 1) return FIM_DE_ARQUIVO;

    //printf(" nLidos = %d, (%s), ftell = %ld\n", nLidos, lido, ftell(textFile));

    for (int i = 0; i < sizeof(relacionais) / sizeof(relacionais[0]); i++) {
        int simbolo_len = strlen(relacionais[i].simbolo);
        if (strncmp(relacionais[i].simbolo, lido, simbolo_len) == 0) {
            // Check if we read enough characters to match the symbol
            //printf(" nLidos = %d, (%s), ftell = %ld\n", nLidos, lido, ftell(textFile));
            if (nLidos >= simbolo_len) {
                fseek(textFile, simbolo_len - nLidos, SEEK_CUR);
            } else {
                // If the symbol is longer than what we read, adjust accordingly
                fseek(textFile, -nLidos, SEEK_CUR);
                fread(lido, sizeof(char), simbolo_len, textFile);
            }
           // printf(" boolErro = %d \n", *boolErro);
            if (*boolErro == 1) {
                printf(", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            }
            printf("%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
            return ENCONTRADO;
        }
    }

    fseek(textFile, -nLidos, SEEK_CUR);
    //printf("SAINDO COM ftell = (%ld)\n", ftell(textFile));
    return NAO_ENCONTRADO;
}

int atribuicao(FILE* textFile, int* boolErro, int* boolSpace) {
    SSimb relacionais[] = {
        {":=", "simbolo_atribuicao"},
    };

    char lido[3] = {0}; // Initialize all elements to zero
    int nLidos = 0;

    //printf ("ENTRANDO COM (%ld)\n", ftell(textFile));

    nLidos = fread(lido, sizeof(char), 2, textFile);
    if (nLidos < 1) return FIM_DE_ARQUIVO;

    //printf(" nLidos = %d, (%s), ftell = %ld\n", nLidos, lido, ftell(textFile));

    for (int i = 0; i < sizeof(relacionais) / sizeof(relacionais[0]); i++) {
        int simbolo_len = strlen(relacionais[i].simbolo);
        if (strncmp(relacionais[i].simbolo, lido, simbolo_len) == 0) {
            // Check if we read enough characters to match the symbol
            //printf(" nLidos = %d, (%s), ftell = %ld\n", nLidos, lido, ftell(textFile));
            if (nLidos >= simbolo_len) {
                fseek(textFile, simbolo_len - nLidos, SEEK_CUR);
            } else {
                // If the symbol is longer than what we read, adjust accordingly
                fseek(textFile, -nLidos, SEEK_CUR);
                fread(lido, sizeof(char), simbolo_len, textFile);
            }
           // printf(" boolErro = %d \n", *boolErro);
            if (*boolErro == 1) {
                printf(", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            }
            printf("%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
            return ENCONTRADO;
        }
    }

    fseek(textFile, -nLidos, SEEK_CUR);
    //printf("SAINDO COM ftell = (%ld)\n", ftell(textFile));
    return NAO_ENCONTRADO;
}

/*
int operadorUnario(FILE* textFile){

SSimb relacionais[] = {
    {"+", "simbolo_mais"},
    {"-", "simbolo_menos"}
};

char lido[3], c;
lido[2] = '\0';
int nLidos = 0;

do {
    c = getc(textFile);
    if (c == EOF) return 0;
} while (isspace(c));

ungetc(c, textFile);

nLidos = fread (lido, sizeof(char), 1, textFile);

for (int i = 0; i < sizeof(relacionais) / sizeof(relacionais[0]); i++){
   // printf ("Comparando (%s) com (%s).\n", relacionais[i].simbolo, lido);
    if (strncmp(relacionais[i].simbolo, lido, strlen(relacionais[i].simbolo)) == 0){
        fseek (textFile, strlen(relacionais[i].simbolo)-(nLidos), SEEK_CUR);
      //  printf("nLidos = %d strlen(relacionais[i].simbolo)) = %d \n", nLidos, strlen(relacionais[i].simbolo));
        printf("%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
        return 1;
    }
} 

//printf("nLidos = %d\n", nLidos);
fseek (textFile, -(nLidos), SEEK_CUR);
return 1;

}*/


int operadorMaisMenos(FILE* textFile, int* boolErro, int* boolSpace){

SSimb relacionais[] = {
    {"+", "simbolo_mais"},
    {"-", "simbolo_menos"}
};

char lido[2], c;
lido[1] = '\0';
int nLidos = 0;


if (fread(&c, sizeof(char), 1, textFile) != 1) return FIM_DE_ARQUIVO;

lido[0] = c;

nLidos = 1;

for (int i = 0; i < sizeof(relacionais) / sizeof(relacionais[0]); i++){

    if (strncmp(relacionais[i].simbolo, lido, strlen(relacionais[i].simbolo)) == 0){
        fseek (textFile, strlen(relacionais[i].simbolo)-(nLidos), SEEK_CUR);
        if (*boolErro == 1){
            printf (", <ERRO_LEXICO>\n");
            *boolErro = 0;
            *boolSpace = 0;
        }
        printf("%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
        return ENCONTRADO;
    }
}


fseek (textFile, -(nLidos), SEEK_CUR);
return NAO_ENCONTRADO;

} 


int operadorPontuacao(FILE* textFile, int* boolErro, int* boolSpace){

SSimb relacionais[] = {
    {",", "simbolo_virgula"},
    {";", "simbolo_ponto_e_virgula"},
    {".", "simbolo_ponto"},
    {": ", "simbolo_dois_ponto"}
};

char lido[2], c;
lido[1] = '\0';
int nLidos = 0;


if (fread(&c, sizeof(char), 1, textFile) != 1) return FIM_DE_ARQUIVO;

lido[0] = c;

nLidos = 1;

for (int i = 0; i < sizeof(relacionais) / sizeof(relacionais[0]); i++){;
    if (strncmp(relacionais[i].simbolo, lido, strlen(relacionais[i].simbolo)) == 0){
        fseek (textFile, strlen(relacionais[i].simbolo)-(nLidos), SEEK_CUR);
        if (*boolErro == 1){
            printf (", <ERRO_LEXICO>\n");
            *boolErro = 0;
            *boolSpace = 0;
        }
        printf("%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
        return ENCONTRADO;
    }
}

//printf("nLidos = %d\n", nLidos);
fseek (textFile, -(nLidos), SEEK_CUR);
return NAO_ENCONTRADO;

}


int operadorDivMult(FILE* textFile, int* boolErro, int* boolSpace){

SSimb relacionais[] = {
    {"/", "simbolo_divisao"},
    {"*", "simbolo_multiplicacao"}
};

char lido[2], c;
lido[1] = '\0';
int nLidos = 0;


if (fread(&c, sizeof(char), 1, textFile) != 1) return FIM_DE_ARQUIVO;

lido[0] = c;

nLidos = 1;

for (int i = 0; i < sizeof(relacionais) / sizeof(relacionais[0]); i++){;
    if (strncmp(relacionais[i].simbolo, lido, strlen(relacionais[i].simbolo)) == 0){
        fseek (textFile, strlen(relacionais[i].simbolo)-(nLidos), SEEK_CUR);
        if (*boolErro == 1){
            printf (", <ERRO_LEXICO>\n");
            *boolErro = 0;
            *boolSpace = 0;
        }
        printf("%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
        return ENCONTRADO;
    }
}

//printf("nLidos = %d\n", nLidos);
fseek (textFile, -(nLidos), SEEK_CUR);
return NAO_ENCONTRADO;

}

int ParentesesDireito(FILE* textFile, int* boolErro, int* boolSpace){

SSimb relacionais[] = {
    {")", "parenteses_direito"},
};

char lido[2], c;
lido[1] = '\0';
int nLidos = 0;


if (fread(&c, sizeof(char), 1, textFile) != 1) return FIM_DE_ARQUIVO;

lido[0] = c;

nLidos = 1;

    if (strncmp(relacionais[0].simbolo, lido, strlen(relacionais[0].simbolo)) == 0){
        fseek (textFile, strlen(relacionais[0].simbolo)-(nLidos), SEEK_CUR);
        if (*boolErro == 1){
            printf (", <ERRO_LEXICO>\n");
            *boolErro = 0;
            *boolSpace = 0;
        }
        printf("%s, %s\n", relacionais[0].simbolo, relacionais[0].denominador);
        return ENCONTRADO;
    }


//printf("nLidos = %d\n", nLidos);
fseek (textFile, -(nLidos), SEEK_CUR);
return NAO_ENCONTRADO;

}

int ParentesesEsquerdo(FILE* textFile, int* boolErro, int* boolSpace){

SSimb relacionais[] = {
    {"(", "parenteses_esquerdo"},
};

char lido[2], c;
lido[1] = '\0';
int nLidos = 0;

if (fread(&c, sizeof(char), 1, textFile) != 1) return FIM_DE_ARQUIVO;


lido[0] = c;

nLidos = 1;


    if (strncmp(relacionais[0].simbolo, lido, strlen(relacionais[0].simbolo)) == 0){
        fseek (textFile, strlen(relacionais[0].simbolo)-(nLidos), SEEK_CUR);
        if (*boolErro == 1){
            printf (", <ERRO_LEXICO>\n");
            *boolErro = 0;
            *boolSpace = 0;
        }
        printf("%s, %s\n", relacionais[0].simbolo, relacionais[0].denominador);
        return ENCONTRADO;
    }


//printf("nLidos = %d\n", nLidos);
fseek (textFile, -(nLidos), SEEK_CUR);
return NAO_ENCONTRADO;

} 

int main(){
    FILE* TextFile = fopen("compilador.txt", "rb");
    char erroLexico = 'e';
    int c, result, fimArq = 0, bytesLidos = 0;
    int boolErro = 0, boolSpace = 0;

    if (!TextFile) {
        printf("Failed to open file");
        return 1;
    }

    while ((result = (relacional(TextFile, &boolErro, &boolSpace) 
    * operadorMaisMenos(TextFile, &boolErro, &boolSpace)
    * operadorDivMult(TextFile, &boolErro,  &boolSpace)
    * ParentesesDireito(TextFile, &boolErro,  &boolSpace)
    * ParentesesEsquerdo(TextFile, &boolErro, &boolSpace)
    * identificador(TextFile, &boolErro, &boolSpace)
    * comentario (TextFile, &boolErro, &boolSpace)
    * numero (TextFile, &boolErro,  &boolSpace)
    * operadorPontuacao(TextFile, &boolErro,  &boolSpace)
    * atribuicao (TextFile, &boolErro,  &boolSpace))) % FIM_DE_ARQUIVO != 0) {  // Se não for fim de arquivo

       // printf(" (%ld) ", ftell(TextFile));
        if (result % ENCONTRADO != 0){   //Se nenhum for encontrado

            fimArq = (fread(&erroLexico, sizeof(char), 1, TextFile) == 0);
            if (!isspace (erroLexico)){
                boolErro = 1;
                if (boolSpace == 1){
                    printf (", <ERRO_LEXICO>\n");
                    boolSpace = 0;
                }
                printf ("%c %d %d ", erroLexico, boolErro, boolSpace);
            } else {
                if (boolErro == 1) boolSpace = 1;
            }
            if (fimArq && boolErro) { 
                printf (", <ERRO_LEXICO>\n");
                return 0;} 

            } 
    } 
    
    if (boolErro) {
        printf (", <ERRO_LEXICO>\n");
    } 

     fclose(TextFile);
     return 0;
}


// || operadorMaisMenos(TextFile) || ParentesesDireito(TextFile) || ParentesesEsquerdo(TextFile))