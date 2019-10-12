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

// read id as long integer
long read_id() {
	char input[MAXINPUTLEN], *endptr;
	fgets(input, MAXINPUTLEN, stdin);
	long val = strtol(input, &endptr, 10);
	return val;
}

// Callback 
static int callback(void *notused, int argc, char **argv, char **azcolname) {
	int i;
	for (int i = 0; i < argc; ++i) {
		printf("%s\t\t%s\n", azcolname[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

// Add new flashcard
int add(void) {
	char fside[MAXINPUTLEN], sside[MAXINPUTLEN];
	printf("First side: ");
	fgets(fside, MAXINPUTLEN, stdin);
	fflush(stdin);
	printf("\nSecond side: ");
	fgets(sside, MAXINPUTLEN, stdin);
	printf("\n");

	if (fside[strlen(fside)-1] == '\n') {
		fside[strlen(fside)-1] = '\0';
	}
	if (sside[strlen(sside)-1] == '\n') {
		sside[strlen(sside)-1] = '\0';
	}

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
	long id;
	char input[MAXINPUTLEN];
	printf("Flashcard ID: ");
	id = read_id();
	printf("\n");

	sprintf(sql, "DELETE FROM %s WHERE id = %d", TBNAME, id);
	if (sqlite3_exec(db, sql, 0, 0, &err_msg)) {
		fprintf(stderr, "Failed to deleted flashcard %d: %s\n", id, err_msg);
		sqlite3_free(err_msg);
		return 1;
	}
	printf("Flashcard %d deleted!\n", id);
	sqlite3_free(err_msg);
	return 0;
}

// Edit flashcard
int update(void) {
	long id;
	char fside[MAXINPUTLEN], sside[MAXINPUTLEN];

	printf("Flashcard ID: ");
	id = read_id();
	printf("\nNew first side: ");
	fgets(fside, MAXINPUTLEN, stdin);
	printf("\nNew second side: ");
	fgets(sside, MAXINPUTLEN, stdin);
	printf("\n");

	if (fside[strlen(fside)-1] == '\n') {
		fside[strlen(fside)-1] = '\0';
	}
	if (sside[strlen(sside)-1] == '\n') {
		sside[strlen(sside)-1] = '\0';
	}

	sprintf(sql, "UPDATE %s SET fside = '%s', sside = '%s' WHERE id = %d", TBNAME, fside, sside, id);
	if (sqlite3_exec(db, sql, 0, 0, &err_msg)) {
		fprintf(stderr, "Failed update flashcard %d: %s\n", id, err_msg);
		sqlite3_free(err_msg);
		return 1;
	}
	printf("Flashcard %d updated!\n", id);
	sqlite3_free(err_msg);
	return 0;
}

// Retrieve flashcard by id

// Search for flashcards

// Random pickup flashcard to learn

// Display all flashcard
int display(void) {
	printf("YOUR FLASHCARDS\n\n");
	sprintf(sql, "SELECT * FROM %s", TBNAME);
	if (sqlite3_exec(db, sql, callback, 0, &err_msg)) {
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
		printf("/a\t\tAdd new flashcard\n");
		printf("/e\t\tEdit flashcard\n");
		printf("/d\t\tDelete flashcard\n");
		printf("/q\t\tQuit\n\n");
		
		fgets(command, MAXINPUTLEN, stdin);
		printf("-------------------------------------\n");
	
		if (command[strlen(command)-1] == '\n') {
			command[strlen(command)-1] = '\0';
		}

		if (strcmp("/a", command) == 0) {
			add();
		} else if (strcmp("/e", command) == 0) {
			update();
		} else if (strcmp("/d", command) == 0) {
			delete();
		} else if (strcmp("/da", command) == 0) {
			display();
		} else if (strcmp("/q", command) == 0) {
			break;
		}
	}

	sqlite3_close(db);
	exit(EXIT_SUCCESS);
}
