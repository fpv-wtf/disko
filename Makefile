ifeq ($(nodep),)
include dep.def
endif

#project name
ifndef PROJECTNAME
    export PROJECTNAME=disko
    export PROJECTVERSION=1.1.1
endif

all clean update:
	@$(shell [ -d ./lib ] || mkdir lib)
	@+make -C  ./src/mmsinfo   $@
	@+make -C  ./src/mmstools  $@
	@+make -C  ./src/mmsconfig $@
	@+make -C  ./src/mmsgui    $@
	@+make -C  ./src/mmsbase   $@
	@+make -C  ./src/mmsinput  $@
ifdef __HAVE_MMSMEDIA__
	@+make -C  ./src/mmsmedia  $@
endif
	@+make -C  ./src/mmscore   $@
ifdef __HAVE_MMSSIP__
	@+make -C  ./src/mmssip    $@
endif
	@+make -C  ./tools/taff    $@
ifdef __HAVE_MMSFLASH__
	@+make -C  ./src/mmsflash  $@
endif

install-disko:
	@+make -C  ./src/mmstools  install
	@+make -C  ./src/mmsconfig install
	@+make -C  ./src/mmsgui    install
	@+make -C  ./src/mmsbase   install
	@+make -C  ./src/mmsinput  install
ifdef __HAVE_MMSMEDIA__
	@+make -C  ./src/mmsmedia  install
endif
	@+make -C  ./src/mmscore   install
ifdef __HAVE_MMSSIP__
	@+make -C  ./src/mmssip    install
endif
	@+make -C  ./src/mmsinfo   install
	@+make -C  ./tools/taff    install
ifdef __HAVE_MMSFLASH__
	@+make -C  ./src/mmsflash  install
endif
	@install -T -m 644 ./inc/mms.h $(DESTDIR)$(incdir)/mms.h
	@install -d "$(DESTDIR)$(datadir)"
	@pushd share &> /dev/null; \
		for i in `find . -type f | grep -v /CVS/`; do \
			dir=`dirname $$i`; \
			install -d "$(DESTDIR)$(datadir)/$$dir"; \
			install -c -m 644 "$$i" "$(DESTDIR)$(datadir)/$$dir/"; \
		done; \
	popd &> /dev/null;

install-pkgconfig:
	@echo "writing pkgconfig information"
	@install -d "$(DESTDIR)$(libdir)/../pkgconfig"
	@echo "prefix=$(prefix)" > $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc
	@echo "exec_prefix=\$${prefix}/bin" >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc
	@echo "libdir=\$${prefix}/lib/disko" >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc
	@echo "includedir=\$${prefix}/include/disko" >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc
	@echo  >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc
	@echo "Name: $(PROJECTNAME)" >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc
	@echo "Description: Multimedia Framework on top of DirectFB" >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc
	@echo "Version: 1.5.0" >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc
	@echo $(req_str) >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc
	@echo $(libs_str)   >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc 
	@echo $(cflags_str) >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc
	@echo "URL: http://www.diskohq.org" >> $(DESTDIR)$(libdir)/../pkgconfig/$(PROJECTNAME).pc

install: install-disko install-pkgconfig

check:
	@echo check for libpng
	@pkg-config --print-errors --exists libpng
	@echo check for libxml++
	@pkg-config --print-errors --exists libxml++-2.6
	@echo check for sqlite3
	@pkg-config --print-errors --exists sqlite3
	@echo check for libcurl
	@pkg-config --print-errors --exists libcurl
	@echo check for freetype2
	@pkg-config --print-errors --exists freetype2
	@echo check for directfb
	@pkg-config --print-errors --exists directfb
	@echo check for libsigc++
	@pkg-config --print-errors --exists sigc++-2.0
	@echo check for libxine
	@pkg-config --print-errors --exists libxine
		
dox:
	@doxygen ./doc/conf/$(PROJECTNAME).conf
	
srcpkg:
	@source ./scripts/create_src_pkg.sh
