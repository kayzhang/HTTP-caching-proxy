SRC=$(wildcard *.cpp)
HEADER=$(wildcard *.h)

proxy: $(SRC) $(HEADER)
	g++ -o proxy *.cpp -lpthread

.Phony: clean

clean:
	rm -f *~ *.o *.out proxy
