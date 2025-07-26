TARGET=MojiGene
OBJ = rng.o utf8_table.o utf8.o MojiGene.o
CFLAGS = -O2 -Wall -c -fdata-sections -ffunction-sections
LFLAGS = -Wl,--gc-sections
LDLIBS = 

ifeq ($(rng), c)
  RNG = rng.c
else
  RNG = rng.cpp
  LDLIBS += -lstdc++
endif

all: $(TARGET)

rng.o: $(RNG)
	$(CC) $(CFLAGS) $< -o $@

utf8_table.o: utf8_table.c
	$(CC) $(CFLAGS) $< -o $@

utf8.o: utf8.c
	$(CC) $(CFLAGS) $< -o $@

MojiGene.o: MojiGene.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) $(OBJ)
