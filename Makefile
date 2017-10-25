include mk/pre-setup.mk

INCLUDE =./include
INCLUDE +=./spine/include
INCLUDE +=./e3api/include
INCLUDE +=./leaf/include

SRC = e3infra
SRC += e3net
SRC += e3api
SRC += spine
SRC +=leaf

include mk/post-setup.mk
