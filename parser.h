#ifndef PARSER_H
#define PARSER_H

#include "lexico.h" // Inclui as definições de token e a função proximoToken

// Função principal do analisador sintático
void analisarSintaticamente();

// Funções para cada não-terminal da gramática P-- (seguindo a descida recursiva)
// Regra 1: <programa> ::= program ident ; <corpo> .
void programa();

// Regra 2: <corpo> ::= <dc> begin <comandos> end
void corpo();

// Regra 3: <dc> ::= <dc_c> <dc_v> <dc_p>
void dc(); // Declarações gerais (const, var, proc)

// Regra 4: <dc_c> ::= const ident = <numero> ; <dc_c> | λ
void dcConst(); // Declaração de constante

// Regra 5: <dc_v> ::= var <variaveis> : <tipo_var> ; <dc_v> | λ
void dcVar(); // Declaração de variável

// Regra 6: <tipo_var> ::= real | integer
void tipoVar();

// Regra 7: <variaveis> ::= ident <mais_var>
void variaveis();

// Regra 8: <mais_var> ::= , <variaveis> | λ
void maisVar();

// Regra 9: <dc_p> ::= procedure ident <parametros> ; <corpo_p> <dc_p> | λ
void dcProc(); // Declaração de procedimento

// Regra 10: <parametros> ::= ( <lista_par> ) | λ
void parametros();

// Regra 11: <lista_par> ::= <variaveis> : <tipo_var> <mais_par>
void listaPar();

// Regra 12: <mais_par> ::= ; <lista_par> | λ
void maisPar();

// Regra 13: <corpo_p> ::= <dc_loc> begin <comandos> end ;
void corpoP(); // Corpo de procedimento

// Regra 14: <dc_loc> ::= <dc_v>
void dcLoc(); // Declarações locais (só variáveis)

// Regra 15: <lista_arg> ::= ( <argumentos> ) | λ
void listaArg(); // Lista de argumentos em chamada

// Regra 16: <argumentos> ::= ident <mais_ident>
void argumentos();

// Regra 17: <mais_ident> ::= ; <argumentos> | λ
void maisIdent();

// Regra 18: <pfalsa> ::= else <cmd> | λ
void pfalsa(); // Parte 'else' do IF

// Regra 19: <comandos> ::= <cmd> ; <comandos> | λ
void comandos(); // Lista de comandos

// Regra 20: <cmd> ::= read ( <variaveis> ) | write ( <variaveis> ) | while ( <condicao> ) do <cmd> |
//                     if <condicao> then <cmd> <pfalsa> | ident := <expressao> | ident <lista_arg> |
//                     begin <comandos> end | for ident := <expressao> to <expressao> do <cmd> (NOVO)
void cmd(); // Comando único

// Regra 21: <condicao> ::= <expressao> <relacao> <expressao>
void condicao();

// Regra 22: <relacao> ::= = | <> | >= | <= | > | <
void relacao();

// Regra 23: <expressao> ::= <termo> <outros_termos>
void expressao();

// Regra 24: <op_un> ::= + | - | λ
void opUn(); // Operador unário

// Regra 25: <outros_termos> ::= <op_ad> <termo> <outros_termos> | λ
void outrosTermos(); // Continuação da expressão

// Regra 26: <op_ad> ::= + | -
void opAd(); // Operador de adição/subtração

// Regra 27: <termo> ::= <op_un> <fator> <mais_fatores>
void termo();

// Regra 28: <mais_fatores> ::= <op_mul> <fator> <mais_fatores> | λ
void maisFatores(); // Continuação do termo

// Regra 29: <op_mul> ::= * | /
void opMul(); // Operador de multiplicação/divisão

// Regra 30: <fator> ::= ident | <numero> | ( <expressao> )
void fator();

// Funções auxiliares para o parser
void erroSintatico(const char* mensagem);
void consumirToken(TipoToken esperado);
void recuperarErro(TipoToken* sincronizacao, int num_sinc);

#endif // PARSER_H
