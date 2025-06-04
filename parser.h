#ifndef PARSER_H
#define PARSER_H

#include "lexico.h" // Inclui as definições de token e a função proximoToken

// Função principal do analisador sintático
void analisarSintaticamente();

// Funções para cada não-terminal da gramática PL/0 (seguindo a descida recursiva)
void programa();
void bloco();
void declaracaoConstante();
void declaracaoVariavel();
void declaracaoProcedimento();
void comando();
void atribuicao();
void chamadaProcedimento();
void comandoCondicional();
void comandoRepeticao();
void comandoComposto();
void condicao();
void expressao();
void termo();
void fator();

// Funções auxiliares para o parser
// Reporta um erro sintático para o arquivo de saída
void erroSintatico(const char* mensagem);
// Tenta consumir o token esperado; se não for o esperado, reporta erro.
void consumirToken(TipoToken esperado);
// Implementa a recuperação de erros em modo pânico, pulando tokens até encontrar um de sincronização.
void recuperarErro(TipoToken* sincronizacao, int num_sinc);

#endif // PARSER_H
