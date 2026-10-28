CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRC      = main.cpp Server.cpp Communicator.cpp Authenticator.cpp UserDatabase.cpp \
           DataProcessor.cpp Logger.cpp UserInterface.cpp
OBJ      = $(SRC:.cpp=.o)

server: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

doc:
	doxygen -g Doxyfile >/dev/null && doxygen Doxyfile

clean:
	rm -f $(OBJ) server
