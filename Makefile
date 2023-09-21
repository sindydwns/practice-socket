SERVER = server
CLIENT = client
.DEFAULT_GOAL = all

CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -MMD -MP -g3
SERVERSRC = \
	practice-parser/util/NotSupportException.cpp \
	practice-parser/parser/Parser.cpp \
	practice-parser/parser/ParseResult.cpp \
	practice-parser/parser/APattern.cpp \
	practice-parser/parser/PatternOptionGroup.cpp \
	practice-parser/parser/PatternSequenceGroup.cpp \
	practice-parser/parser/PatternEqual.cpp \
	practice-parser/parser/PatternReadUntil.cpp \
	practice-parser/parser/PatternWord.cpp \
	practice-parser/parser/PatternReadAll.cpp \
	practice-parser/parser/PatternSkipWs.cpp \
	practice-parser/parser/ParseStream.cpp \
	$(SERVER).cpp
CLIENTSRC = \
	$(CLIENT).cpp
.PHONY: all clean fclean re test

all: $(SERVER) $(CLIENT)

DEP = $(SERVERSRC:.cpp=.d)
-include $(DEP)

$(SERVER): $(SERVERSRC:.cpp=.o)
	$(CXX) $(CXXFLAGS) $(OUTPUT_OPTION) $(SERVERSRC:.cpp=.o)

$(CLIENT): $(CLIENTSRC:.cpp=.o)
	$(CXX) $(CXXFLAGS) $(OUTPUT_OPTION) $(CLIENTSRC:.cpp=.o)

$(CLASS:.cpp=.o): %.o: %.cpp %.hpp

clean:
	rm -rf */**/*.o *.o
	rm -rf */**/*.d *.d

fclean: clean
	rm -f $(SERVER) $(CLIENT)

re: fclean
	make all

test: all
	./$(SERVER) | (sleep 1 && ./$(CLIENT))
