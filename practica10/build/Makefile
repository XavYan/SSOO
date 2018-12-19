CC=g++
CXXFLAGS=-g

all:
	$(CC) $(CXXFLAGS) -std=c++11 ../src/Socket.cpp ../src/main.cpp -o chatsi -pthread

send:
	$(CC) $(CXXFLAGS) -std=c++11 ../src/Socket.cpp ../src/mainsend.cpp -o send -pthread

recv:
	$(CC) $(CXXFLAGS) -std=c++11 ../src/Socket.cpp ../src/mainrecv.cpp -o recv -pthread

clear:
	rm recv send

clean:
	rm -f $(ODIR)/*.o *~ $(INCDIR)/*~
