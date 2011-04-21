#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static const int NUMWORDS = 10;

struct node_ {
	char *item;
	struct node_ *next;
};

typedef struct node_ node;

int cons(char *str, int n, node **list) {
	node *new = malloc(sizeof(node));
	new->item = malloc(n);
	strcpy(new->item, str);
	new->next = *list;
	*list = new;
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

/* return a string that is produced by concatenating 'words' together with
 * char 'c' interposed between them
 */
char *intercalate(char c, int total_len, int numwords, char **words) {
	int msglen = total_len + numwords;      /* words + spaces + null */
	int i = 0, wi = 0, j;
	char *msg = malloc(msglen); 
	for (j = 0; j < msglen-1; j++) {
		if (words[wi][i] == '\0') {
			wi++;
			i = 0;
			msg[j] = ' ';           /* space between words */
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
	if (argc > 1) {
		f = fopen(*++argv, "r");
	}
	else
		f = fopen("/usr/share/dict/words", "r");
	size_t n = 0;
	ssize_t numread = 0;
	char *line = NULL;
	node *list = NULL;
	int count = 0;

	while ((numread = getline(&line, &n, f)) > 0) {
		line[numread-1] = '\0';
		if (allfrom("abcdefgh", line) && numread > 3) {
			count++;
			cons(line, numread-1, &list);
		}
	}
	
	/* choose several random words */
	srandom((unsigned int)time(NULL));
	char *words[NUMWORDS]; 
	int j, total_len = 0;
	for (j = 0; j < NUMWORDS; j++) {
		words[j] = nth(random()%count, list);
		total_len += strlen(words[j]);
	}
	
	char *msg = intercalate(' ', total_len, NUMWORDS, words);
	printf("[%s]\n", msg);
	
	return 0;
}
