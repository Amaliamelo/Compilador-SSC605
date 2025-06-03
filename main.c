#include "lexico.h"

//Função Principal
int main(){
    FILE* TextFile = fopen("compilador.txt", "rb");
    FILE* TextSaida = fopen("saida.txt", "w");
    char erroLexico = 'e';
    int c, result, fimArq = 0, bytesLidos = 0;
    int boolErro = 0, boolSpace = 0;

    if (!TextFile) {
        fprintf(TextSaida, "Failed to open file");
        return 1;
    }

    while ((result = (relacional(TextFile, TextSaida, &boolErro, &boolSpace) 
    * operadorMaisMenos(TextFile, TextSaida, &boolErro, &boolSpace)
    * operadorDivMult(TextFile, TextSaida, &boolErro,  &boolSpace)
    * ParentesesDireito(TextFile, TextSaida, &boolErro,  &boolSpace)
    * ParentesesEsquerdo(TextFile, TextSaida, &boolErro, &boolSpace)
    * identificador(TextFile, TextSaida, &boolErro, &boolSpace)
    * comentario (TextFile, TextSaida, &boolErro, &boolSpace)
    * numero (TextFile, TextSaida, &boolErro,  &boolSpace)
    * operadorPontuacao(TextFile, TextSaida, &boolErro,  &boolSpace)
    * atribuicao (TextFile, TextSaida, &boolErro,  &boolSpace))) % FIM_DE_ARQUIVO != 0) {  // Se não for fim de arquivo

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
     return 0;
}