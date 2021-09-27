
# 通常编译一个新的工程时，只需亚修改PATH_INCLUDE的内容，其他的都不需要修改
# 如果需要在这里链接某个输入库，则需要修改并打开PATH_LIBARAY和NAME_LIBARAY

TOPTMP               = $(shell pwd)
TOPDIR               = $(shell echo ${TOPTMP} |sed -r 's/(.*\/source).*/\1/g')
include $(TOPDIR)/config.mk

#输入的头文件路径
INCLUDE :=      -I$(TOPDIR)/pubinc/                     \
                -I.

LIBS      = -lstdc++ -lpthread
DYNAFILE  = libnetmtcom.so#End

				
LIB_INC   = -L$(TOPDIR)/../out/bin -L./  -L$(TOPDIR)/../out/lib
DYNADIR   =$(TOPDIR)/../out/lib
EXCSRC	  =   
LIBCPPOBJS	 =  $(patsubst %.cpp,%.o,$(filter-out $(EXCSRC),$(wildcard *.cpp hash/*.cpp cmd/*.cpp)))
LIBCOBJS     =  $(patsubst %.c,%.o,$(filter-out $(EXCSRC),$(wildcard *.c hash/*.c cmd/*.c)))

DYNALIBRARY	 = $(DYNADIR)/$(DYNAFILE)

all: $(DYNALIBRARY)

clean:
	$(OS_RM) $(DYNALIBRARY) $(LIBCOBJS) $(LIBCPPOBJS)

$(DYNALIBRARY): .depend $(LIBCOBJS) $(LIBCPPOBJS)
	$(OS_CC) $(OS_CCDEBUGFLAGS) $(LDFLAGS) $(LIB_INC) $(LIBS) -o $@ $(LIBCOBJS) $(LIBCPPOBJS)

.depend: makefile $(LIBCOBJS:.o=.c) $(LIBCPPOBJS:.o=.cpp)
	$(OS_CC) -MM $(OS_CCFLAGS) $(OS_CCDYNAFLAGS) $(OS_CCDEBUGFLAGS) $(INCLUDE) $(LIBCOBJS:.o=.c) $(LIBCPPOBJS:.o=.cpp) > $@

ifeq (.depend, $(wildcard .depend))
include .depend
endif

%.o:%.c
	$(OS_CC)  $(OS_CCFLAGS) $(OS_CCDYNAFLAGS) $(OS_CCDEBUGFLAGS) $(INCLUDE) -c $< -o $@

%.o:%.cpp
	$(OS_CC)  $(OS_CCFLAGS) $(OS_CCDYNAFLAGS) $(OS_CCDEBUGFLAGS) $(INCLUDE) -c $< -o $@

