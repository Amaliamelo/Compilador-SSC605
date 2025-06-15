// main.c

#include "lexico.h"   // Para as definições de Token e getNextToken
#include "sintatico.h" // Para a função iniciarAnaliseSintatica

// current_token é declarada como 'extern' em lexico.h e definida em lexico.c.
// Nao precisa definir novamente aqui.
//Função Principal
int main(){
    char nomeEntrada[] = "testeX.txt";
    char nomeSaida[] = "saidaX.txt";

    for (int i = 1; i < 7; i++) {
    
    nomeEntrada[5] = i + '0';
    nomeSaida[5] = i + '0';

    FILE* TextFile = fopen(nomeEntrada, "rb");
    FILE* TextSaida = fopen(nomeSaida, "w");
    int boolErro = 0; // Flag de erro (léxico ou sintático)
    int boolSpace = 0; // Usado para erros léxicos e no comentário, mantém para compatibilidade

    if (!TextFile) {
        fprintf(TextSaida, "Failed to open file\n");
        return 1;
    }

    // Inicia a análise sintática
    iniciarAnaliseSintatica(TextFile, TextSaida, &boolErro, &boolSpace);

    // O resultado da compilação (sucesso ou falha) já é impresso por `programa()` em sintatico.c

    fclose(TextFile);
    fclose(TextSaida);
    }
    return 0;
}