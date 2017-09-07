CCFLAG += $(foreach item,$(INCLUDE),-I$(item))

SRCS =$(foreach item,$(SRC),$(wildcard $(item)/*.c))
OBJ=$(patsubst %.c,%.o,$(SRCS))

ifeq ($(COMPILE_SHARED_LIBRARY),y)
        TARGET=$(DLIB_TARGET)
else
        TARGET=$(SLIB_TARGET)
endif

%.o:%.c
	$(CC)  $(CCFLAG) -c -o $@ $<

$(TARGET):$(OBJ)
ifeq ($(TARGET),$(DLIB_TARGET))
	gcc -shared  $(LDFLAG) -o $(DLIB_TARGET)  $(OBJ)
else
	ar cr $(SLIB_TARGET) $(OBJ)
endif

install:$(DLIB_TARGET)
	rm -f /lib64/$(DLIB_TARGET)
	cp ./$(DLIB_TARGET) /lib64
uninstall:$(DLIB_TARGET)
	-rm -f /lib64/$(DLIB_TARGET)
clean:
	-rm -f $(TARGET)
	find -name "*.o" -exec rm -f {} \;
