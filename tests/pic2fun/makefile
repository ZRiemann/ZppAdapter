.PHONY : all
all : main libtst_res.so msg.pb.cc msg1.pb.cc resource.o resource_pic.o

resource.o : resource.cpp msg.pb.cc
	g++ -c -g -o $@ resource.cpp

resource_pic.o : resource.cpp msg.pb.cc
	g++ -c -g -fPIC -o $@ resource.cpp

main : main.cpp resource.o msg.pb.cc
	g++ -g -o $@ $^ /usr/local/lib/libprotobuf.a -lzit -lpthread -ldl -lrt
#	g++ -g -o $@ $^ -lprotobuf -lzit -lpthread -ldl -lrt

libtst_res.so : resource_pic.o plg_res.cpp plg_res_imp.cpp msg.pb.cc msg1.pb.cc
	g++ -g -fPIC -shared -o $@ $^ /usr/local/lib/libprotobuf.a

msg.pb.cc : msg.proto
	protoc --proto_path=.:/usr/local/include --cpp_out=. msg.proto

msg1.pb.cc : msg1.proto
	protoc --proto_path=.:/usr/local/include --cpp_out=. msg1.proto

.PHONY : install
install :
	@cp libtst_res.so /usr/local/lib &&\
	ldconfig

.PHONY : uninstall
uninstall :
	@rm -f /usr/local/lib/libtst_res.so &&\
	ldconfig

.PHONY : clean
clean :
	@rm -f main libtst_res.so msg.pb* msg1.pb*