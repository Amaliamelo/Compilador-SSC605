#include "lexico.h"




// Função para reconhecer operadores relacionais: <>, <=, >=, =, >, < e dois-pontos
// Lê até dois caracteres do arquivo e compara com os símbolos relacionais esperados.
// Se encontrar um dos operadores definidos, escreve no arquivo de saída com seu nome descritivo.
// Caso contrário, retorna NAO_ENCONTRADO e desfaz a leitura dos caracteres.
int relacional(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
    SSimb relacionais[] = {
        {"<>", "simbolo_diferente"},
        {"<=", "simbolo_menor_igual"},
        {">=", "simbolo_maior_igual"},
        {"=", "simbolo_igual"},
        {">", "simbolo_maior"},
        {"<", "simbolo_menor"},
        {": ", "simbolo_dois_pontos"}
    };

    char lido[3] = {0}; // Initialize all elements to zero
    int nLidos = 0;

    //fprintf(textSaida, "ENTRANDO COM (%ld)\n", ftell(textFile));

    nLidos = fread(lido, sizeof(char), 2, textFile);
    if (nLidos < 1) return FIM_DE_ARQUIVO;

    //fprintf(textSaida, " nLidos = %d, (%s), ftell = %ld\n", nLidos, lido, ftell(textFile));

    for (int i = 0; i < sizeof(relacionais) / sizeof(relacionais[0]); i++) {
        int simbolo_len = strlen(relacionais[i].simbolo);
        if (strncmp(relacionais[i].simbolo, lido, simbolo_len) == 0) {
            // Check if we read enough characters to match the symbol
            //fprintf(textSaida, " nLidos = %d, (%s), ftell = %ld\n", nLidos, lido, ftell(textFile));
            if (nLidos >= simbolo_len) {
                fseek(textFile, simbolo_len - nLidos, SEEK_CUR);
            } else {
                // If the symbol is longer than what we read, adjust accordingly
                fseek(textFile, -nLidos, SEEK_CUR);
                fread(lido, sizeof(char), simbolo_len, textFile);
            }
           // fprintf(textSaida, " boolErro = %d \n", *boolErro);
            if (*boolErro == 1) {
                fprintf(textSaida, ", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            }
            fprintf(textSaida, "%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
            return ENCONTRADO;
        }
    }

    fseek(textFile, -nLidos, SEEK_CUR);
    //fprintf(textSaida, "SAINDO COM ftell = (%ld)\n", ftell(textFile));
    return NAO_ENCONTRADO;
}

// Função para reconhecer o símbolo de atribuição :=
int atribuicao(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace) {
    SSimb relacionais[] = {
        {":=", "simbolo_atribuicao"},
    };

    char lido[3] = {0}; // Initialize all elements to zero
    int nLidos = 0;

    //fprintf(textSaida, "ENTRANDO COM (%ld)\n", ftell(textFile));

    nLidos = fread(lido, sizeof(char), 2, textFile);
    if (nLidos < 1) return FIM_DE_ARQUIVO;

    //fprintf(textSaida, " nLidos = %d, (%s), ftell = %ld\n", nLidos, lido, ftell(textFile));

    for (int i = 0; i < sizeof(relacionais) / sizeof(relacionais[0]); i++) {
        int simbolo_len = strlen(relacionais[i].simbolo);
        if (strncmp(relacionais[i].simbolo, lido, simbolo_len) == 0) {
            // Check if we read enough characters to match the symbol
            //fprintf(textSaida, " nLidos = %d, (%s), ftell = %ld\n", nLidos, lido, ftell(textFile));
            if (nLidos >= simbolo_len) {
                fseek(textFile, simbolo_len - nLidos, SEEK_CUR);
            } else {
                // If the symbol is longer than what we read, adjust accordingly
                fseek(textFile, -nLidos, SEEK_CUR);
                fread(lido, sizeof(char), simbolo_len, textFile);
            }
           // fprintf(textSaida, " boolErro = %d \n", *boolErro);
            if (*boolErro == 1) {
                fprintf(textSaida, ", <ERRO_LEXICO>\n");
                *boolErro = 0;
                *boolSpace = 0;
            }
            fprintf(textSaida, "%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
            return ENCONTRADO;
        }
    }

    fseek(textFile, -nLidos, SEEK_CUR);
    //fprintf(textSaida, "SAINDO COM ftell = (%ld)\n", ftell(textFile));
    return NAO_ENCONTRADO;
}


// Função para reconhecer os operadores aritméticos '+' e '-'
int operadorMaisMenos(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace){

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
            fprintf (textSaida, ", <ERRO_LEXICO>\n");
            *boolErro = 0;
            *boolSpace = 0;
        }
        fprintf(textSaida, "%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
        return ENCONTRADO;
    }
}


fseek (textFile, -(nLidos), SEEK_CUR);
return NAO_ENCONTRADO;

} 


// Função para reconhecer operadores de pontuação: vírgula, ponto e vírgula, ponto, dois-pontos
int operadorPontuacao(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace){

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
            fprintf (textSaida, ", <ERRO_LEXICO>\n");
            *boolErro = 0;
            *boolSpace = 0;
        }
        fprintf(textSaida, "%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
        return ENCONTRADO;
    }
}

//fprintf(textSaida, "nLidos = %d\n", nLidos);
fseek (textFile, -(nLidos), SEEK_CUR);
return NAO_ENCONTRADO;

}


int operadorDivMult(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace){

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
            fprintf (textSaida, ", <ERRO_LEXICO>\n");
            *boolErro = 0;
            *boolSpace = 0;
        }
        fprintf(textSaida, "%s, %s\n", relacionais[i].simbolo, relacionais[i].denominador);
        return ENCONTRADO;
    }
}

//fprintf(textSaida, "nLidos = %d\n", nLidos);
fseek (textFile, -(nLidos), SEEK_CUR);
return NAO_ENCONTRADO;

}

// Função para reconhecer parênteses direitos: )
int ParentesesDireito(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace){

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
            fprintf (textSaida, ", <ERRO_LEXICO>\n");
            *boolErro = 0;
            *boolSpace = 0;
        }
        fprintf(textSaida, "%s, %s\n", relacionais[0].simbolo, relacionais[0].denominador);
        return ENCONTRADO;
    }


//ffprintf(textSaida, textSaida, "nLidos = %d\n", nLidos);
fseek (textFile, -(nLidos), SEEK_CUR);
return NAO_ENCONTRADO;

}

// Função para reconhecer parênteses esquerdos: (
int ParentesesEsquerdo(FILE* textFile, FILE* textSaida, int* boolErro, int* boolSpace){

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
            fprintf (textSaida, ", <ERRO_LEXICO>\n");
            *boolErro = 0;
            *boolSpace = 0;
        }
        fprintf(textSaida, "%s, %s\n", relacionais[0].simbolo, relacionais[0].denominador);
        return ENCONTRADO;
    }


//ffprintf(textSaida, textSaida, "nLidos = %d\n", nLidos);
fseek (textFile, -(nLidos), SEEK_CUR);
return NAO_ENCONTRADO;

}