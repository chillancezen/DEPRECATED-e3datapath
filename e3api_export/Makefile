
ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

ifeq ($(E3_SDK),)
$(error "E3_SDK environment variable not set")
endif

# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

# binary name
APP = api_export

# all source are stored in SRCS-y
SRCS-y := export_e3api.c

CFLAGS += -O3 -I/$(E3_SDK)/e3api/include -include $(E3_SDK)/config/e3_config.h 
CFLAGS += -I/root/jzheng/userspace-rcu-master/src -L/root/mywork/userspace-rcu/src/.libs
CFLAGS += -L/lib64
LDFLAGS += -lurcu-qsbr -le3datapath -lzmq 
include $(RTE_SDK)/mk/rte.extapp.mk


export:$(OBJs)
	./build/api_export
