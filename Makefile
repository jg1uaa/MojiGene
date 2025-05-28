TARGET=MojiGene
OBJ = utf8_table.o MojiGene.o
CFLAGS = -O2 -Wall -c -fdata-sections -ffunction-sections
LFLAGS = -Wl,--gc-sections
LDLIBS = 

all: $(TARGET)

utf8_table.o: utf8_table.c
	$(CC) $(CFLAGS) $< -o $@

MojiGene.o: MojiGene.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) $(OBJ)
