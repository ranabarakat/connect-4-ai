CC = g++

SFML = D:\Downloads\SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit\SFML-2.5.1

CFLAGS = -I$(SFML)/include -L$(SFML)/lib -lsfml-graphics-s-d -lsfml-window-s-d -lsfml-system-s-d -lsfml-main-d -lopengl32 -lfreetype -lwinmm -lgdi32

TARGET = bitboard

all: $(TARGET)

$(TARGET):
	$(CC) -std=c++17 $(TARGET).cpp -o $(TARGET).exe $(CFLAGS)


clean:
	$(RM) $(TARGET).exe

debug:
	$(CC) $(TARGET).cpp -g -o $(TARGET).exe $(CFLAGS)