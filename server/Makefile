CFLAGS += -I../quiche/include -Wall -g
LDFLAGS += -L../quiche -lquiche
# Needed on macos
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -framework Security -framework Foundation
endif

server: main.c ../quiche/libquiche.a
	$(CC) $(CFLAGS) -o server main.c -Wall $(LDFLAGS)

clean:
	rm -f server server.dSYM
