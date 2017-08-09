#top level makefile
ifeq ($(VER), debug)
	GDB=-g
else ifeq ($(VER), release)
	GDB=-O3
else ifeq ($(VER), pic)
	GDB=-O3
	FPIC=-fPIC
	PIC=_pic
else
$(warning default release module)
$(warning useage: make VER={debug|release})
	VER=release
	GDB=-O3
endif

CC=g++
BIN_NAME=$(VER)
ROOT_DIR=.
OUT_NAME=make
OUT_DIR=$(ROOT_DIR)/$(OUT_NAME)
OBJS_DIR=$(OUT_NAME)/$(BIN_NAME)_obj
BIN_DIR=$(OUT_NAME)/$(BIN_NAME)
INST_DIR=/usr/local/lib
ZPP_NAME=libzpp
ZPP_VER=$(ZPP_NAME)$(PIC).a
CFLAGS=$(GDB) -Wall -Werror -I. $(FPIC)
# **** export variable to sub makefiles ***
export CC CFLAGS BIN_DIR BIN_NAME GDB ZPP_VER VER

define make_obj
	@mkdir -p $(OUT_NAME)
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(BIN_DIR)
	@cp -u makeout.mk $(OBJS_DIR)/makefile
	@make -C $(ROOT_DIR)/tests/protobuf
	@./compiler.sh tests $(OBJS_DIR) $(CC) "$(CFLAGS)"
endef

#define install_zpp
#	@rm -f $(INST_DIR)/$(ZPP_NAME)* &&\
#	rm -fr /usr/local/include/zpp && \
#	cp -r zpp /usr/local/include/ && \
#	cp $(BIN_DIR)/$(ZPP_VER) $(INST_DIR) && ldconfig
#endef

.PHONY : all
all: makezit makeout

.PHONY : makezit
makezit:
	$(make_obj)

.PHONY : makeout
makeout:
	@make -C $(OBJS_DIR)
	@make -C tests

.PHONY:clean
clean:
	@rm -fr $(OUT_DIR) &&\
	rm -f makeworker tests/protobuf/*.pb* tests/protobuf/reader tests/protobuf/write
	@make -C $(ROOT_DIR)/tests/protobuf clean
	@make -C $(ROOT_DIR)/tests/pic2fun clean


.PHONY:install
install :
	@cp -r zpp /usr/local/include/
#ifeq ($(VER), pic)
#	cp $(BIN_DIR)/$(ZPP_VER) $(INST_DIR)
#else
#	$(install_zpp)
#endif

.PHONY:uninstall
uninstall:
	@rm -fr /usr/local/include/zpp
#	@rm -f $(INST_DIR)/$(ZPP_VER)
