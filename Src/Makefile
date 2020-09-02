IDIR =.
CC=g++
CFLAGS=-std=c++17 -Og -g -Wall -I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = Kernel/Types.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = Logger/LogPolicies.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/Logger/%.o: Logger/%.cpp Logger/%.hpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -ILogger

$(ODIR)/Kernel/%.o: Kernel/%.cpp Kernel/%.hpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -ILogger -IKernel

$(ODIR)/%.o: %.cpp %.hpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -ILogger -IKernel -IMath

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
	rm -f $(ODIR)/Logger/*.o *~ core $(INCDIR)/*~ 