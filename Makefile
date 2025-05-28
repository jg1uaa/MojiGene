TARGET=MojiGene
OBJ = MojiGene.o
CFLAGS = -O2 -Wall -c -fdata-sections -ffunction-sections
LFLAGS = -Wl,--gc-sections
LDLIBS = 

all: $(TARGET)

MojiGene.o: MojiGene.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) $(OBJ)
