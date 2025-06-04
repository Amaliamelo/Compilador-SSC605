# Nome do compilador C
CC = gcc

# Nome do executável final
TARGET = compilador

# Lista de todos os arquivos fonte .c no projeto
SRCS = main.c lexico.c parser.c

# Gera os nomes dos arquivos objeto (.o) a partir dos arquivos fonte .c
OBJS = $(SRCS:.c=.o)

# Alvo padrão: compilar o executável
all: $(TARGET)

# Regra para compilar o executável:
# Depende de todos os arquivos objeto.
# Linka os arquivos objeto para criar o executável final.
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Regras de compilação para cada arquivo objeto (.o) a partir de seus arquivos .c correspondentes.
# Cada regra também lista os arquivos de cabeçalho (.h) dos quais depende,
# garantindo que o .o seja recompilado se qualquer um dos .h mudar.

# Regra para main.o: depende de main.c, lexico.h e parser.h
main.o: main.c lexico.h parser.h
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para lexico.o: depende de lexico.c e lexico.h
lexico.o: lexico.c lexico.h
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para parser.o: depende de parser.c, parser.h e lexico.h
parser.o: parser.c parser.h lexico.h
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para rodar o programa:
# Primeiro garante que o executável esteja compilado, depois o executa.
run: $(TARGET)
	./$(TARGET)

# Regra para limpar os arquivos gerados:
# Remove o executável e todos os arquivos objeto.
clean:
	rm -f $(TARGET) $(OBJS)

# Regra para verificar erros estáticos no código:
# Compila todos os arquivos fonte apenas para verificar a sintaxe, sem gerar executável.
lint:
	$(CC) $(CFLAGS) -fsyntax-only $(SRCS)

# Declara alvos que não correspondem a nomes de arquivos reais,
# para evitar conflitos com arquivos de mesmo nome.
.PHONY: all run clean lint
