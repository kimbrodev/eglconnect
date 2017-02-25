CC		= gcc
UPSTREAM_BASE	= /home/kimbro/code/upstream_bin
INCLUDES	= -I$(UPSTREAM_BASE)/bin/include
CFLAGS		= -g $(INCLUDES)
LIBLOC		= -L$(UPSTREAM_BASE)/lib
LIBS		= -lEGL -lxcb -lGL
NAME		= connect
SRC		= $(NAME).c

$(NAME): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBLOC) $(LIBS) -o $@

clean:
	rm -rf $(NAME)

