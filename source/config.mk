OS := OS_LINUX
OS_CC           := gcc 
OS_CCFLAGS      := -Wall -Wno-deprecated -Wno-unused-function -march=native  

OS_CCDEBUGFLAGS := -g
OS_LD           := gcc 
LDFLAGS   = -shared 
OS_CCDYNAFLAGS  := -fPIC
OS_RM           := rm -f
