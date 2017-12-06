lib: uuid tool redisRPC redisUtil
	ar rcs libxglhiredis.a util.o tool.o rpc.o uuid.o
	g++ -fPIC -shared ./include/uuid/uuid4.cpp common_tool.cpp RedisRPC.cpp Redis_Utils.cpp  -o libxglhiredis.so -lhiredis -lpthread -levent -std=c++11
uuid: ./include/uuid/uuid4.cpp
	g++ -c ./include/uuid/uuid4.cpp -o uuid.o -std=c++11
tool: common_tool.cpp
	g++ -c common_tool.cpp -o tool.o -std=c++11
redisRPC: RedisRPC.cpp
	g++ -c RedisRPC.cpp -o rpc.o -std=c++11
redisUtil: Redis_Utils.cpp
	g++ -c Redis_Utils.cpp -o util.o -std=c++11

clean:
	rm *.o libxglhiredis.a libxglhiredis.so