#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#include <stdlib.h>

#define DTBNAME ".flashcard.db"
#define TBNAME "flashcard"
#define MAXINPUTLEN 200
#define MAXSQLLEN 500

sqlite3 *db;
char *err_msg;
sqlite3_stmt *res;
int rc;
char sql[MAXSQLLEN]; 

// Initialize: create database and tables
void init(void) {
	if (sqlite3_open(DTBNAME, &db)) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(EXIT_FAILURE);
	}

	sprintf(sql, "CREATE TABLE IF NOT EXISTS %s(id INTEGER PRIMARY KEY, fside TEXT, sside TEXT)", TBNAME);
	if (sqlite3_exec(db, sql, 0, 0, &err_msg)) {
		fprintf(stderr, "Cannot create table: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
		exit(EXIT_FAILURE);
	}
	sqlite3_free(err_msg);
}

// read number
long read_number() {
	char input[MAXINPUTLEN], *endptr;
	fgets(input, MAXINPUTLEN, stdin);
	fflush(stdin);
	long val = strtol(input, &endptr, 10);
	return val;
}

// read text 
void read_text(char *input, int len) {
	fgets(input, len, stdin);
	fflush(stdin);
	if (input[strlen(input)-1] = '\n') {
		input[strlen(input)-1] = '\0';
	}
}

// Callback 
static int display_callback(void *notused, int argc, char **argv, char **azcolname) {
	int i;
	for (int i = 0; i < argc; ++i) {
		printf("%s\t\t%s\n", azcolname[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

// Display flashcard by id
int display_id(long id) {
	sprintf(sql, "SELECT fside, sside FROM %s WHERE id = %ld", TBNAME, id);
	if (sqlite3_exec(db, sql, display_callback, 0, &err_msg)) {
		fprintf(stderr, "Failed to display flashcards: %s\n", err_msg);
		sqlite3_free(err_msg);
		return 1;
	}
	sqlite3_free(err_msg);
	return 0;
}

// Add new flashcard
int add(void) {
	char fside[MAXINPUTLEN], sside[MAXINPUTLEN];
	printf("First side: ");
	read_text(fside, MAXINPUTLEN);
	printf("\nSecond side: ");
	read_text(sside, MAXINPUTLEN);
	printf("\n");

	sprintf(sql, "INSERT INTO %s VALUES(NULL, ?, ?)", TBNAME);
	if (sqlite3_prepare_v2(db, sql, -1, &res, 0) || sqlite3_bind_text(res, 1, fside, -1, SQLITE_STATIC) || sqlite3_bind_text(res, 2, sside, -1, SQLITE_STATIC) || sqlite3_step(res) != SQLITE_DONE) {
		fprintf(stderr, "Failed to add new card: %s\n", sqlite3_errmsg(db));
		sqlite3_finalize(res);
		return 1;
	}
	printf("New card added!\n");
	sqlite3_finalize(res);
	return 0;
}

// Delete flashcard by id
int delete(void) {
	long id = 0;
	printf("Flashcard ID: ");
	id = read_number();
	printf("\n");

	if (id != 0) {
		display_id(id);
		printf("Delete flashcard (1/0)? ");
		long confirm = read_number();

		if (confirm == 1) {
			sprintf(sql, "DELETE FROM %s WHERE id = %d", TBNAME, id);
			if (sqlite3_exec(db, sql, 0, 0, &err_msg)) {
				fprintf(stderr, "Failed to deleted flashcard %d: %s\n", id, err_msg);
				sqlite3_free(err_msg);
				return 1;
		}
		printf("Flashcard %d deleted!\n", id);
		sqlite3_free(err_msg);
		}
	}

	return 0;
}

// Edit flashcard
int update(void) {
	long id;
	char fside[MAXINPUTLEN], sside[MAXINPUTLEN];

	printf("Flashcard ID: ");
	id = read_number();
	if (id != 0) {
		display_id(id);

		printf("\nNew first side: ");
		read_text(fside, MAXINPUTLEN);
		printf("\nNew second side: ");
		read_text(sside, MAXINPUTLEN);
		printf("\n");

		sprintf(sql, "UPDATE %s SET fside = ?, sside = ? WHERE id = ?", TBNAME);
		if (sqlite3_prepare_v2(db, sql, -1, &res, 0) || sqlite3_bind_text(res, 1, fside, -1, SQLITE_STATIC) || sqlite3_bind_text(res, 2, sside, -1, SQLITE_STATIC) || sqlite3_bind_int(res, 3, id) || sqlite3_step(res) != SQLITE_DONE) {
			fprintf(stderr, "Failed update flashcard %d: %s\n", id, sqlite3_errmsg(db));
			sqlite3_finalize(res);
			return 1;
		}
		printf("Flashcard %d updated!\n", id);
		sqlite3_free(err_msg);
	}
	return 0;
}

// Random pickup flashcard to learn
int random_pick(void) {
	sprintf(sql, "SELECT fside, sside FROM %s WHERE id IN (SELECT id FROM %s ORDER BY RANDOM() LIMIT 1)", TBNAME, TBNAME);
	if (sqlite3_exec(db, sql, display_callback, 0, &err_msg)) {
		fprintf(stderr, "Failed to pickup flashcard: %s\n", err_msg);
		sqlite3_free(err_msg);
		return 1;
	}
	sqlite3_free(err_msg);
	return 0;
}

// Display all flashcard
int display(void) {
	printf("YOUR FLASHCARDS\n\n");
	sprintf(sql, "SELECT * FROM %s", TBNAME);
	if (sqlite3_exec(db, sql, display_callback, 0, &err_msg)) {
		fprintf(stderr, "Failed to display flashcards: %s\n", err_msg);
		sqlite3_free(err_msg);
		return 1;
	}
	sqlite3_free(err_msg);
	return 0;
}

// Program start here
int main() {
	char command[MAXINPUTLEN];
	init();
	while (1) {
		printf("\n-------------------------------------\n");
		printf("/da\t\tDisplay all flashcards\n");
		printf("/r\t\tPick a random flashcard\n");
		printf("/a\t\tAdd new flashcard\n");
		printf("/e\t\tEdit flashcard\n");
		printf("/d\t\tDelete flashcard\n");
		printf("/q\t\tQuit\n\n");
		
		read_text(command, MAXINPUTLEN);
		printf("-------------------------------------\n");
	
		if (strcmp("/a", command) == 0) {
			add();
		} else if (strcmp("/e", command) == 0) {
			update();
		} else if (strcmp("/d", command) == 0) {
			delete();
		} else if (strcmp("/da", command) == 0) {
			display();
		} else if (strcmp("/r", command) == 0) {
			random_pick();
		} else if (strcmp("/q", command) == 0) {
			break;
		}
	}

	sqlite3_close(db);
	exit(EXIT_SUCCESS);
}
