.PHONY: all game editor clean

inc = -I./include
CC = gcc


all:game editor

game:
	$(CC) ./game/game.c ./game/main.c $(inc) -lallegro -lallegro_image -lallegro_audio -lallegro_acodec -o game

editor:
	$(CC) ./editor/editor.c ./editor/main.c ./game/game.c $(inc) -lallegro -lallegro_image -lallegro_audio -lallegro_acodec -lallegro_primitives -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -o editor

clean:
	rm *.exe