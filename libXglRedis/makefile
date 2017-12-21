lib: uuid tool redisRPC redisUtil log cacheUtils clientCacheUtils
	ar rcs libxglhiredis.a util.o tool.o rpc.o uuid.o log.o cacheUtils.o clientCacheUtils.o
	g++ -fPIC -shared ./include/log/MyLogger.cpp ./include/uuid/uuid4.cpp CommonTools.cpp RedisRPC.cpp RedisUtils.cpp ClientCacheUtils.cpp CacheUtils.cpp -o libxglhiredis.so -lhiredis -lpthread -levent -llog4cplus -std=c++11
uuid: ./include/uuid/uuid4.cpp
	g++ -c ./include/uuid/uuid4.cpp -o uuid.o -std=c++11
tool: CommonTools.cpp
	g++ -c CommonTools.cpp -o tool.o -std=c++11
redisRPC: RedisRPC.cpp
	g++ -c RedisRPC.cpp -o rpc.o -std=c++11
redisUtil: RedisUtils.cpp
	g++ -c RedisUtils.cpp -o util.o -std=c++11
log: ./include/log/MyLogger.cpp
	g++ -c ./include/log/MyLogger.cpp -o log.o -std=c++11
cacheUtils:
	g++ -c CacheUtils.cpp -o cacheUtils.o -std=c++11
clientCacheUtils:
	g++ -c ClientCacheUtils.cpp -o clientCacheUtils.o -std=c++11

install:
	cp libxglhiredis.a libxglhiredis.so /usr/local/lib

clean:
	rm *.o libxglhiredis.a libxglhiredis.so