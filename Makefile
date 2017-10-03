include mk/pre-setup.mk

INCLUDE =./include
INCLUDE +=./spine/include
INCLUDE +=./e3api/include


SRC = e3infra
SRC += e3net
SRC += e3api
SRC += spine

include mk/post-setup.mk
