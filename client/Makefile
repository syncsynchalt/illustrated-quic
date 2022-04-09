CFLAGS += -I../quiche/include -Wall -g
LDFLAGS += -L../quiche -lquiche
# Needed on macos
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -framework Security -framework Foundation
endif

client: main.c ../quiche/libquiche.a
	$(CC) $(CFLAGS) -o client main.c -Wall $(LDFLAGS)

clean:
	rm -rf client client.dSYM
