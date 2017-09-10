COMPILE_SHARED_LIBRARY=y

DLIB_TARGET=libe3datapath.so
SLIB_TARGET=libe3datapath.a

CCFLAG += -I${RTE_SDK}/$(RTE_TARGET)/include -include rte_config.h -include ./config/e3_config.h
CCFLAG += -D_LGPL_SOURCE
CCFLAG += -g3  -Wall -fpic -O3 -m64  -march=native

LDFLAG+= -L${RTE_SDK}/$(DPDK_ARCH)/lib -L/usr/lib/x86_64-linux-gnu -lurcu-qsbr

CC=gcc
LD=ld
