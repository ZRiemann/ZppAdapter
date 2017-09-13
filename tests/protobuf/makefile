all : write reader msg.pb.cc msg1.pb.cc write1 reader1 pb2json

write : msg.pb.cc write.cc
	g++ msg.pb.cc write.cc -o write -lprotobuf

reader : msg.pb.cc reader.cc
	g++ $^ -o $@ -lprotobuf

clean :
	@rm -f write reader msg*.cc msg*.h *.o msg.pb *~
	@rm -f reader1 write1 pb2json msg1.pb

msg.pb.cc : msg.proto
	protoc --proto_path=.:/usr/local/include --cpp_out=. msg.proto

msg1.pb.cc : msg1.proto
	protoc --proto_path=.:/usr/local/include --cpp_out=. msg1.proto

write1 : msg.pb.cc write1.cc msg1.pb.cc
	g++ $^ -o $@ -lprotobuf

reader1 : msg.pb.cc reader1.cc msg1.pb.cc
	g++ $^ -o $@ -lprotobuf

pb2json : tst_pb2json.cc msg1.pb.cc msg.pb.cc
	g++ -g $^ -o $@ -lprotobuf -lzit -ldl