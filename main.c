#include "lexico.h" // Inclui as definições e funções do analisador léxico
#include "parser.h" // Inclui as definições e funções do analisador sintático

// Função principal do programa
int main(){
    // Abre o arquivo de entrada (programa PL/0) para leitura binária
    FILE* TextFile = fopen("compilador.txt", "rb");
    // Abre o arquivo de saída para escrita
    FILE* TextSaida = fopen("saida.txt", "w");

    // Verifica se o arquivo de entrada foi aberto com sucesso
    if (!TextFile) {
        fprintf(stderr, "Failed to open input file 'compilador.txt'\n");
        if (TextSaida) fclose(TextSaida); // Fecha o arquivo de saída se ele foi aberto
        return 1; // Retorna código de erro
    }
    // Verifica se o arquivo de saída foi aberto com sucesso
    if (!TextSaida) {
        fprintf(stderr, "Failed to open output file 'saida.txt'\n");
        if (TextFile) fclose(TextFile); // Fecha o arquivo de entrada se ele foi aberto
        return 1; // Retorna código de erro
    }

    // Inicializa o analisador léxico, passando os ponteiros dos arquivos
    inicializarLexico(TextFile, TextSaida);

    // Inicia a análise sintática do programa
    analisarSintaticamente();

    // Fecha os arquivos após a conclusão da análise
    fclose(TextFile);
    fclose(TextSaida);
    
    return 0; // Retorna sucesso
}
