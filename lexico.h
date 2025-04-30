#ifndef ARQUIVO_H
#define ARQUIVO_H





// Função para adicionar um token ao array
void adicionarToken(const char *lexema, const char *token, int linha, int status);

// Função para validar identificadores
void automatoIdentificador(const char *palavra, int num_linha);

// Função para validar números inteiros e reais
void automatoNumero(const char *aux, int num_linha);
    
// Função para imprimir os tokens
void imprimirTokens();

// Função para verificar se o token é uma palavra
int ehPalavra(const char *token); 
// Função para verificar se o token é um número
int ehNumero(const char *token);
int ehPalavraReservada(const char *lexema);

// Função para verificar se o token é um símbolo
int ehOperador(const char *lexema);

#endif // ARQUIVO_H
