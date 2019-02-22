#------------------------------------------------------------------------------
SOURCE=main.cpp benchmarking.h benchmarking.cpp util.cpp util.h
MYPROGRAM=simplebench
CC=g++ 
LDLIBS=-lpthread  
#------------------------------------------------------------------------------
all: $(MYPROGRAM)

$(MYPROGRAM): $(SOURCE)
	        $(CC) $(SOURCE) -o $(MYPROGRAM) $(LDLIBS)
clean:
	        rm -f $(MYPROGRAM)

