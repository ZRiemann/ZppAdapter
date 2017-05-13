#top level makefile

CC=g++
ROOT_DIR=$(shell pwd)
OUT_NAME=make
OUT_DIR=$(ROOT_DIR)/$(OUT_NAME)
OBJS_DIR=$(OUT_DIR)/obj
BIN_DIR=$(OUT_DIR)/bin
GDB=-g
#GDB=-O3
INST_DIR=/usr/local/lib
ZPP_NAME=libzpp
ZPP_VER=$(ZPP_NAME).a
CFLAGS='$(GDB) -Wall -Werror -I$(ROOT_DIR)'
#CFLAGS='$(GDB) -fPIC -Wall -Werror -I$(ROOT_DIR)'
# for test flag
CFLAGST = '$(GDB) -Wall -Werror -I$(ROOT_DIR)'
# **** export variable to sub makefiles ***
export CC CFLAGS OBJS_DIR BIN_DIR GDB ZPP_VER

define make_obj
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(BIN_DIR)
	@cp -u makeout.mk $(OBJS_DIR)/makefile
	@gcc makeworker.c -o makeworker
	@./makeworker . $(OBJS_DIR) .cpp $(CC) $(CFLAGS)
endef

define install_zpp
	rm -f $(INST_DIR)/$(ZPP_NAME)* &&\
	rm -fr /usr/local/include/zpp && \
	cp -r zpp /usr/local/include/ && \
	cp $(BIN_DIR)/$(ZPP_VER) $(INST_DIR) && ldconfig
endef

.PHONY : all
all: makezit makeout
.PHONY:makezit
makezit:
	$(make_obj)
makeout:
	@make -C $(OBJS_DIR)
	@make -C $(ROOT_DIR)/tests

#.PHONY : test
#test :
#	@./makeworker tests $(OBJS_DIR) .cpp $(CC) $(CFLAGST) &&\
#	make -C $(OBJS_DIR) test

#.PHONY : arm_test
#arm_test :
#	@./makeworker tests $(OBJS_DIR) .cpp $(CC) $(CFLAGST) &&\
#	make -C $(OBJS_DIR) arm_test

.PHONY:clean
clean:
	@rm -fr $(OUT_DIR) &&\
	rm -f makeworker tests/protobuf/*.pb* tests/protobuf/reader tests/protobuf/write

.PHONY:install
install :
	$(install_zpp)

.PHONY:uninstall
uninstall:
	@rm -f $(INST_DIR)/$(ZIT_NAME)* &&\
	ldconfig &&\
	rm -fr /usr/local/include/zpp
