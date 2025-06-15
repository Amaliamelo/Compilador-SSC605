# Nome do compilador
CC = gcc

# Nome do executável
TARGET = compilador

# Arquivos fonte
SRCS = main.c lexico.c sintatico.c

# Arquivos objeto (gerados a partir dos arquivos fonte)
OBJS = $(SRCS:.c=.o)

# Alvo padrão: compilar o executável
all: $(TARGET)

# Regra para compilar o executável: liga os arquivos objeto
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Regra para compilar arquivos .c em .o
# Isso usa uma regra implícita do make para compilar cada .c em seu respectivo .o
# e garante que o .o seja recompilado se o .c ou o .h mudar.
%.o: %.c lexico.h
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para rodar o programa
run: $(TARGET)
	./$(TARGET)

# Regra para limpar os arquivos gerados
clean:
	rm -f $(TARGET) $(OBJS)

# Regra para verificar erros estáticos no código
lint:
	$(CC) $(CFLAGS) -fsyntax-only $(SRCS)

.PHONY: all run clean lint
