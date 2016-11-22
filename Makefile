cc=g++
LIB=-L/usr/local/lib/ -lhiredis
target: sayhi client

sayhi: utility.o sayhi.o
	$(cc) -std=c++11  -o sayhi sayhi.o utility.o $(LIB)
sayhi.o: sayhi.cpp
	$(cc) -std=c++11 -c sayhi.cpp $(LIB)
utility.o:utility.cpp	
	$(cc) -std=c++11 -c utility.cpp $(LIB)

client:client.o
	$(cc) -std=c++11 -o client client.o
client.o:client.cpp
	$(cc) -std=c++11 -c client.cpp

clean:
	rm *.o sayhi
