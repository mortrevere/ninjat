CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -pthread

OBJS = main.o
TARGET = ninjat-serv

all: CXXFLAGS += -O2
all: $(TARGET)

debug: CXXFLAGS += -g
debug: $(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
