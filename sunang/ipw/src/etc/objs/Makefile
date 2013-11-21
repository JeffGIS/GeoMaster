include $(IPW)/lib/make/Makestart

SCRIPT=objs
SCRIPT_DIR=$(ETC_DIR)

default: script
	$(MAKE) script SCRIPT=srcs

install: install-script
	$(MAKE) install-script SCRIPT=srcs

include $(IPW)/lib/make/Makerules

srcs.sh: objs.sh
	cp objs.sh srcs.sh
