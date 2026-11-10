# Компилятор и флаги
CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -g -I.
LDFLAGS   =

# Исходники проекта (общие для обеих целей)
COMMON_SRC = \
    Server.cpp \
    UserInterface.cpp \
    UserDatabase.cpp \
    Authenticator.cpp \
    DataProcessor.cpp \
    Communicator.cpp \
    Logger.cpp

COMMON_OBJ = $(COMMON_SRC:.cpp=.o)

# Итоговые исполняемые файлы
SERVER_BIN = server
TEST_BIN   = run_tests

# ==================== Цели ====================

# По умолчанию — собираем оба
all: $(SERVER_BIN) $(TEST_BIN)

# Сервер
$(SERVER_BIN): $(COMMON_OBJ) main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Тесты
$(TEST_BIN): $(COMMON_OBJ) tests.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lUnitTest++

# Универсальное правило компиляции
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==================== Вспомогательные цели ====================

# Запустить сервер
run: $(SERVER_BIN)
	./$(SERVER_BIN)

# Запустить тесты
test: $(TEST_BIN)
	./$(TEST_BIN)

# Очистка
clean:
	rm -f $(COMMON_OBJ) main.o tests.o
	rm -f $(SERVER_BIN) $(TEST_BIN)
	rm -f t.log errors.log

.PHONY: all run test clean
