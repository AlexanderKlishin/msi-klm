
CC=gcc
CXX=g++
COBJS=main.o
CFLAGS+=-Wall
LDFLAGS=-lhidapi-libusb
PROG=msi-klm

$(PROG): $(COBJS)
	$(CXX) $(CFLAGS) $< $(LDFLAGS) -o $@

$(COBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(PROG)

.PHONY: rpm
rpm: $(PROG)
	rpmbuild --define "_builddir $(shell pwd)" -bb msi-klm.spec

