SRCDIR = src
CLIENTDIR = client_data
CLIENTDIRTPL = $(SRCDIR)/client_template
SERVERDIR = server_data
SERVERDIRTPL = $(SRCDIR)/server_template

CXX = g++
CXXFLAGS = -O3 -I$(SRCDIR)/include -w --std=c++11 -lpthread

RM = rm
RMFLAGS = -rf

CP = cp
CPFLAGS = -r

TARGET = client server
CLIENTSRC = $(wildcard $(SRCDIR)/client_*.cpp)
CLIENTOBJ = $(CLIENTSRC:.cpp=.o)
SERVERSRC =$(wildcard $(SRCDIR)/server_*.cpp)
SERVEROBJ = $(SERVERSRC:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

client: $(CLIENTOBJ)
	$(CP) $(CPFLAGS) $(CLIENTDIRTPL) $(CLIENTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

server: $(SERVEROBJ)
	$(CP) $(CPFLAGS) $(SERVERDIRTPL) $(SERVERDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(RM) $(RMFLAGS) $(TARGET)
	$(RM) $(RMFLAGS) $(CLIENTDIR) $(CLIENTOBJ)
	$(RM) $(RMFLAGS) $(SERVERDIR) $(SERVEROBJ)


