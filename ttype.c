#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CTRL(x) (x&037)
#define CEOF CTRL('d')

static const int NUMWORDS = 10;
static const int NUMREPS  = 3;

struct node_ {
	char *item;
	struct node_ *next;
};

typedef struct node_ node;

int cons(char *str, node **list) {
	node *new = malloc(sizeof(node));
	new->item = malloc(strlen(str)+1);
	strcpy(new->item, str);
	new->next = *list;
	*list = new;
	return 1;
}

int freelist(node *list) {
	node *curr = list;
	node *tmp;
	while (curr != NULL) {
		tmp = curr->next;
		free(curr->item);
		free(curr);
		curr = tmp;
	}
	return 1;
}

int printlist(node *list) {
	if (list == NULL)
		printf("(empty)\n");
	node *curr;
	for (curr = list; curr != NULL; curr = curr->next)
		printf("%s ", curr->item);
	return 1;
}

int one_of(char t, int n, char *target) {
	int i;
	for (i = 0; i < n; i++)
		if (target[i] == t)
			return 1;
	return 0;
}

int allfrom(char *target, char *input) {
	int i, len, targlen;
	len = strlen(input);
	targlen = strlen(target);

	for (i = 0; i < len; i++)
		if (!one_of(input[i], targlen, target))
			return 0;
	return 1;
}

char *nth(int n, node *list) {
	int i;
	node *curr = list;
	for (curr = list, i = 0; curr != NULL; curr = curr->next, i++)
		if (i == n)
			return curr->item;
	return NULL;
}

int exit_cleanly() {
	echo();
	keypad(stdscr, 0);
	endwin();
	exit(EXIT_SUCCESS);
}
/* return a string that is produced by concatenating 'words' together with
 * char 'c' interposed between them
 */
char *intercalate(char c, int total_len, int numwords, char **words) {
	int msglen = total_len + numwords;	  /* words + 'c's + null */
	int i = 0, wi = 0, j;
	char *msg = malloc(msglen);
	for (j = 0; j < msglen-1; j++) {
		if (words[wi][i] == '\0') {
			wi++;
			i = 0;
			msg[j] = c;			 /* 'c's between words */
		}
		else {
			msg[j] = words[wi][i];
			i++;
		}
	}
	msg[j] = '\0';
	return msg;
}

int main(int argc, char **argv) {
	FILE *f;
	if (argc > 1)
		f = fopen(*++argv, "r");
	else
		f = fopen("/usr/share/dict/words", "r");
	size_t n = 0;
	ssize_t numread = 0;
	char *line = NULL;
	node *list = NULL;
	int count = 0;

	/* collect candidate words from the dictionary */
	while ((numread = getline(&line, &n, f)) > 0) {
		line[numread-1] = '\0';
		if (allfrom("abcdefgh", line) && numread > 3) {
			count++;
			cons(line, &list);
		}
	}
	free(line);
	fclose(f);

	/* choose NUMWORDS at random from candidates */
	srandom((unsigned int)time(NULL));
	char *words[NUMWORDS];
	int j, total_len = 0;
	for (j = 0; j < NUMWORDS; j++) {
		words[j] = nth(random()%count, list);
		total_len += strlen(words[j]);
	}

	char *msg = intercalate(' ', total_len, NUMWORDS, words);
	freelist(list);
	int msglen = strlen(msg);

	/* now set up some numbers used for metrics */
	int total_msglen = NUMREPS * msglen;
	int num_words = total_len * NUMREPS / 5;		   /* def. of WPM */
	int miss = 0;
	int num_keystrokes = 0;
	int start;
	int end;
	int secs;
	int curr_reps = 0;
	int curr_char = 0;
	int ch;
	float wpm;
	float kspc;

	/* set up the curses interface */
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	/* show the message and start the clock */
	attron(A_BOLD);
	printw(msg);
	attroff(A_BOLD);
	printw("\n");
	start = time(NULL);
	for (curr_reps = 0; curr_reps < NUMREPS; curr_reps++) {
		while (curr_char < msglen) {
			ch = getch();
	  if (ch == CTRL('d')) {
		printw("Exiting\n");
		exit_cleanly();
	  } else if (ch == msg[curr_char]) {
				addch(ch);
				curr_char++;
			}
			else {
				miss++;
			}
			refresh();
			num_keystrokes++;
		}
		curr_char = 0;
		printw("\n");
	}
	end = time(NULL);

	/* calculate metrics */
	secs = end - start;
	wpm = 1.0 * num_words / (secs / 60.0);
	kspc = 1.0 * num_keystrokes / total_msglen;

	/* report */
	printw("\n");
	attron(A_REVERSE | A_BOLD);
	printw("SUCCESS! (press any key to quit)\n");
	attroff(A_REVERSE | A_BOLD);
	printw("Elapsed: %d s\n", secs);
	printw("Words/min: %f\n", wpm);
	printw("Misses: %d\n", miss);
	printw("Keystrokes/char: %f\n", kspc);
	getch();

	/* cleanup */
	free(msg);
  exit_cleanly();
}
