NAME	=	Matt_daemon
SRCS	=	Tintin_reporter.cpp \
			Daemon.cpp \
			matt_deamon.cpp
OBJS	=	$(SRCS:.cpp=.o)
CC	=	g++
CFLAGS	= -Wall -Wextra -Werror

all	:	$(NAME)

$(NAME)	:	$(OBJS)
		$(CC) $(CFLAGS) $^ -o $@

%.o	:	%.cpp
		$(CC) $(CFLAGS) -c $< -o $@

clean	:
		$(RM) $(OBJS)

fclean	:	clean
		rm -rf $(NAME)

re	:	fclean all
