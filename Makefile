# Simplify Makefile
#
# 2008 Axel Eirola

CC=		gcc
CCFLAGS=	-O3 -Wall -g

LIB=		./lib/

LFLAGS=		-static -L$(LIB) -llp
INC=		-I$(LIB)


OBJS=		./misc.o \
		./normalize.o \
		./occurrences.o \
		./readwrite.o \
		./simplification.o \
		./transformations/compute.o \
		./transformations/contra.o \
		./transformations/helpers.o \
		./transformations/litnonmin.o \
		./transformations/nonmin.o \
		./transformations/partev.o \
		./transformations/taut.o \
		./transformations/trivial.o


all:		simplify

simplify:	$(OBJS)
		$(CC) $(CCFLAGS) $(OBJS) -o simplify $(LFLAGS)

clean:		
		rm -f *.o
		rm -f ./transformations/*.o
		rm -f simplify

%.o:		%.c
		$(CC) $(CCFLAGS) -c $*.c -o $*.o $(INC) 
