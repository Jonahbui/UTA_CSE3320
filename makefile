SRC1 = msh.c
EXE = $(SRC1:.c=.e)

all : $(SRC1)
	gcc -Wall -o $(EXE) $(SRC1) -std=c99 -g
