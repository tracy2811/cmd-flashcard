CC=gcc
FILES=main.c sqlite3.c
OUT=flashcard

flashcard: $(FILES)
	$(CC) -o $(OUT) $(FILES)
