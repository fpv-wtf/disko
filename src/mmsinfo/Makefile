include ../../dep.def

all: prepare mmsinfo.o
	@echo " [LD]        (libmmsinfo.so)"
	@$(CXX) -shared $(LDFLAGS) -o $(LIBDIR)/libmmsinfo.so mmsinfo.o
	
clean:
	@echo "Cleaning mmsinfo"
	@rm -f mmsinfo.o
	@rm -f $(LIBDIR)/libmmsinfo.so
	@rm -f ../../inc/mmsinfo.h

update:
	@echo "mmsinfo is up to date."
		
prepare:
	$(shell sed s/__DISKO_PREFIX__/$(shell echo $(prefix) | tr -t  '/' '#' | sed s/#/\\\\\\\\\\\//g)/ ../../inc/mmsinfo/mmsinfo.h.tmpl > ../../inc/mmsinfo/mmsinfo.h)
	$(shell rm -f mmsinfo.o)

install: prepare all
	@echo " [INSTALL]   (mmsinfo)"
	@install -d $(DESTDIR)$(incdir)/mmsinfo
	@cp ../../inc/mmsinfo/mmsinfo.h $(DESTDIR)$(incdir)/mmsinfo/
	@chmod 644 $(DESTDIR)$(incdir)/mmsinfo/mmsinfo.h
	@install -d $(DESTDIR)$(libdir)
	@install -m 755 $(LIBDIR)/libmmsinfo.so $(DESTDIR)$(libdir)/

%.o : %.cpp
	@echo " [CXX]       ($@)"
	@$(CXX) $(CFLAGS) -I../../inc -shared -c $< -o $@
