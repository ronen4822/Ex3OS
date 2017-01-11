CC=gcc
CFLAGS=-c -Wall -g -pedantic
LIBS=-g
SRV_OBJS=Ex31.o
SRV_SRC=Ex31.c 
CLIENT_OBJS=Ex32.o
CLIENT_SRC=Ex32.c

all: server.out client.out

server.out: $(SRV_OBJS)
	$(CC) $(SRV_OBJS) -o server.out $(LIBS)

$(SRV_OBJS): $(SRV_SRC)
	$(CC) $(CFLAGS) $(SRV_SRC)

client.out: $(CLIENT_OBJS)
	$(CC) $(CLIENT_OBJS) -o client.out $(LIBS)

$(CLIENT_OBJS): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC)

clean:
	rm -f $(SRV_OBJS) $(CLIENT_OBJS) server.out client.out
