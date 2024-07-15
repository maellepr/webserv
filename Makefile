NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -MMD -std=c++98

INC_PATH = ./includes/
INC = -I $(INC_PATH)

SRCS_PATH = ./srcs/
SRC = main.cpp webserv.cpp Server.cpp VirtualServer.cpp Client.cpp utils.cpp \
	Location.cpp Request.cpp Response.cpp 
SRCS = $(addprefix $(SRCS_PATH), $(SRC))

OBJS_PATH = .objects/
OBJ = $(SRC:.cpp=.o)
OBJS = $(addprefix $(OBJS_PATH), $(OBJ))

DEPS = $(OBJS:.o=.d)

all: $(OBJS_PATH) $(NAME)

$(OBJS_PATH):
	mkdir -p $(OBJS_PATH)

$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp
		${CXX} ${CXXFLAGS} -c $< -o $@ $(INC)

$(NAME) : $(OBJS)
		$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(INC)

-include $(DEPS)

clean:
		rm -rf $(OBJS_PATH)

fclean:
		rm -rf $(NAME) $(OBJS_PATH)
	
re: fclean
		make all

.PHONY: all clean fclean re