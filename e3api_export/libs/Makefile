ifeq ($(E3_SDK),)
$(error "E3_SDK environment variable not set")
endif

EXPORTED_LIB=libe3api.so

API_LIBS_DIR =$(E3_SDK)/e3api/
SRCs= $(wildcard $(API_LIBS_DIR)/*.c)
SRCs += $(wildcard ./*.c)
SRCs += $(E3_SDK)/e3infra/e3-hashmap.c

OBJs = $(patsubst %.c,%.o,$(SRCs))
CFLAGS += -I$(E3_SDK) -I. -fpic -lpthread -lrt


%.o:%.c
	gcc $(CFLAGS) -o $@   -c $<
$(EXPORTED_LIB):$(OBJs)
	gcc -shared -fPIC -o $(EXPORTED_LIB)   $(OBJs)
clean:
	rm -rf $(EXPORTED_LIB) *.o
