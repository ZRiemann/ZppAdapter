#ZPP_LIB = zppzmq.o object.o zppmysql.o
ZPP_OUT = tstzmq.o zppmain.o tstframework.o

BIN_DIR:=../$(BIN_NAME)

.PHONY : all
all : $(BIN_DIR)/zpp_test
#all : $(BIN_DIR)/$(ZPP_VER) $(BIN_DIR)/zpp_test

#$(BIN_DIR)/$(ZPP_VER) : $(ZPP_LIB)
#	ar crs $@ $^

#$(BIN_DIR)/zpp_test : $(ZPP_OUT) $(BIN_DIR)/$(ZPP_VER)
$(BIN_DIR)/zpp_test :
	$(CC) $(CFLAGS)  -o $@ $(ZPP_OUT) -lmysqlclient -lzit -lzmq -pthread -lrt
