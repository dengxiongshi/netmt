#当前方案所包含的项目名称，也就是目录名，每个方案的会不一样（通常不同的方案需要修改这里）       
SUBDIRS = \
          libhv     \
          cmd         \

                          
                          
.PHONY: $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ all
        

.PHONY: uninstall

uninstall:
	for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir uninstall ; done

install:
	for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir install ; done

depend dep:
	for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir .depend ; done

clean:
	for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir clean ; done

#
# A rule to do nothing.
#
dummy:


#
################################################################################
