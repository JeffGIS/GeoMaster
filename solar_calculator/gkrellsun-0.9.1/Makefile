ZIPVER=

all:
	@echo To build gkrellsun for GKrellM 1.x, run make in src
	@echo To build gkrellsun for GKrellM 2.x, run make in src20

zip:
ifeq ($(ZIPVER),)
	@echo You must specify ZIPVER for the zip target
else
	rm -rf /tmp/gkrellsun-$(ZIPVER)
	rm -f /tmp/tar.exclude
	rm -f /tmp/gkrellsun-$(ZIPVER).tar.gz
	rm -f /tmp/gkrellsun-$(ZIPVER).zip
	mkdir /tmp/gkrellsun-$(ZIPVER)
	touch /tmp/tar.exclude
	find . -print  | grep /CVS$$ | cut -c3- >> /tmp/tar.exclude
	find . -print  | grep /CVS/ | cut -c3- >> /tmp/tar.exclude
	find . -print  | grep .classes | cut -c3- >> /tmp/tar.exclude
	find . -print  | grep /debian | cut -c3- >> /tmp/tar.exclude
	find . -type f -name "*~"  | cut -c3- >> /tmp/tar.exclude
	find . -type f -name "*.o"  | cut -c3- >> /tmp/tar.exclude
	find . -type f -name "*.so"  | cut -c3- >> /tmp/tar.exclude
	find . -type f -name "#*"  | cut -c3- >> /tmp/tar.exclude
	find . -type f -name "suninfo"  | cut -c3- >> /tmp/tar.exclude
	tar cf - * --exclude-from /tmp/tar.exclude | (cd /tmp/gkrellsun-$(ZIPVER); tar xf -)
	cd /tmp && tar cf - gkrellsun-$(ZIPVER) | gzip > gkrellsun-$(ZIPVER).tar.gz
	cd /tmp && zip -rpD gkrellsun-$(ZIPVER).zip gkrellsun-$(ZIPVER)
	rm -f tar.exclude
endif
