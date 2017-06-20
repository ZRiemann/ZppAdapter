# =========================
# -Wl,--as-needed   ;only link needed *.so
# export LD_LIBRARY_PATH=/<libpath>  ;only for test
# -Wl,-rpaht=/<libpath>              ;app load file
# -------------------------
# #cp /<libpath>.lib*.so /usr/lib
# #chmod 755 /usr/lib/lib*.so
# #ldconfig
# =========================
# version control (solve dll hell)
# link-name     soname          real-name(lib<zit>.so.<major>.<minor>.<release>
# libzit.so ==> libzit.so.1 ==> libzit.so.1.1.1234
# <major>: not compatibility
# <minor>: downward compatibility
# <release>: compatible with each other
# gcc -shared -W,soname,libzit.so.0 -o llibzit.so.0.0.0 $^ ;make so
# ls -s libzit.so.0.0.0 libzit.so.0
# =============================
#$(BIN_DIR)/libzit.a : $(LIB_ZIT)
#	ar crv $@ $^
#	$(CC) -g -Wl,-rpath=. -L$(BIN_DIR) -lzit -lpthread -lrt -D_REENTRANT $^ -o $@

#ZIT_LIB=atomic.o  jet.o  module.o  mutex.o  queue.o  ringbuf.o  semaphore.o  ssl.o  thread.o time.o traceconsole.o tracelog.o  trace.o \
#tracering.o type.o list.o container.o framework.o socket.o filesys.o convert.o
#ZIT_TST=tutility.o main.o tbase.o tthread.o
#ZIT_FLAGS='-lpthread -lrt -D_REENTRANT'
ZPP_LIB = zppzmq.o object.o zrpdjson.o zppmysql.o
ZPP_OUT = tstzmq.o zppmain.o tstframework.o 
.PHONY : all
all : $(BIN_DIR)/zpp_test $(BIN_DIR)/$(ZPP_VER)

$(BIN_DIR)/zpp_test : $(ZPP_OUT) $(BIN_DIR)/$(ZPP_VER)
	$(CC) $(GDB)  -o $@ $(ZPP_OUT) -L../bin -lzpp -lmysqlclient -lzit -lzmq -lrt


$(BIN_DIR)/$(ZPP_VER) : $(ZPP_LIB)
	ar crs $@ $^
