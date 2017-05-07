all : write reader msg.pb.cc

write : msg.pb.cc write.cc
	g++ msg.pb.cc write.cc -o write -lprotobuf

reader : msg.pb.cc reader.cc
	g++ msg.pb.cc reader.cc -o reader -lprotobuf

clean :
	rm -f write reader msg.*.cc msg.*.h *.o msg.pb *~

msg.pb.cc : msg.proto
	protoc --proto_path=.:/usr/local/include --cpp_out=. msg.proto

