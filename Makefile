# Nome do compilador
CC = gcc

# Opções de compilação
CFLAGS = -o

# Nome do executável
TARGET = compilador

# Arquivo principal
SRC = main.c

# Alvo padrão: compilar o executável
all: $(TARGET)

# Regra para compilar o executável
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Regra para rodar o programa
run: $(TARGET)
	./$(TARGET)

# Regra para limpar os arquivos gerados
clean:
	rm -f $(TARGET)

# Regra para verificar erros estáticos no código
lint:
	$(CC) $(CFLAGS) -fsyntax-only $(SRC)

.PHONY: all run clean lint
	