TARGET=MojiGene
CFLAGS = -O2 -Wall

all: $(TARGET)

$(TARGET): MojiGene.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGET)
