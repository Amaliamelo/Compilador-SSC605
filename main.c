#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexico.h"

// Teste do automatoIdentificador
int main(int argc, char *argv[]) {
    // Verifica se o número correto de argumentos foi passado
    if (argc != 2) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    //Ponteiros para os arquivos
    FILE *arquivoEntrada;

    // Abrir o arquivo de entrada em modo leitura
    arquivoEntrada = fopen(argv[1], "r");
    if (arquivoEntrada == NULL) {
        printf("Erro ao abrir o arquivo de entrada.\n");
        return 1; // Encerra com erro
    }

    char linha[256]; // Buffer para leitura de linhas
    int numeroLinha = 0;


    // Lê o arquivo linha por linha
    while (fgets(linha, sizeof(linha), arquivoEntrada) != NULL) {
        numeroLinha++; // Incrementa o número da linha

        // Divide a linha em tokens
        char *token = strtok(linha, " \t\n\r;(),:."); // Separadores
        while (token != NULL) {
            // Verifica se é uma palavra reservada
            if (token[0] == '=' && strlen(token) > 1) {
                // Se for algo como '=1' ou '=fat'
                char operador[2] = "=";
                adicionarToken(operador, "OPERADOR", numeroLinha, 1);
                // O restante do token será processado
                if (ehNumero(token + 1)) {
                    automatoNumero(token + 1, numeroLinha); // Número após o '='
                } else if (ehPalavra(token + 1)) {
                    automatoIdentificador(token + 1, numeroLinha); // Palavra após o '='
                }
            }
            // Caso o token seja um símbolo como '{', '}' ou outros
            else if (token[0] == '{' || token[0] == '}' || token[0] == '[' || token[0] == ']') {
                adicionarToken(token, "SIMBOLO", numeroLinha, 1);
            }
            else if (ehPalavraReservada(token)) {
                adicionarToken(token, "RESERVADA", numeroLinha, 1);
            }
            // Verifica se é um operador
            else if (ehOperador(token)) {
                adicionarToken(token, "OPERADOR", numeroLinha, 1);
            }
            // Verifica o tipo do token
            else if (ehPalavra(token)) {
                automatoIdentificador(token, numeroLinha); // Chama função A para palavras
            } else if (ehNumero(token)) {
                automatoNumero(token, numeroLinha); // Chama função B para números
            } else {
                adicionarToken(token, "OUTRO", numeroLinha, 0); // Outros tokens
            }
            token = strtok(NULL, " \t\n\r;(),:."); // Próximo token
        }
    }


    // Fechar os arquivos
    fclose(arquivoEntrada);
    imprimirTokens();
    return 0;
}