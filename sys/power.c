#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "power.h"

#define POWER_DIR "/sys/class/power_supply/"

bool ac_is_online() {
	// Instead of hardcoding, iterate through the dir and check each item
	// is type of "Mains". Similar to how we do battery listing.
	char *flist[] = {
		POWER_DIR "AC/online",
		POWER_DIR "ACAD/online",
	};
	for (int i = 0; i < 2; i++) {
		bool online = read_file_bool(flist[i]);
		if (online)
			return true;
	}
	return false;
}

static int bat_filter(const struct dirent *d) {
	if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
		return 0;
	char *a = strjoin(d->d_name, "/type\0");
	char *str = strjoin(POWER_DIR, a);
	free(a);
	int valid = 0;
	//printf("%s\n", str);
	char *buf = read_file(str);
	if (buf) {
		//printf("buf: %s\n", buf);
		if (strcmp(buf, "Battery\n") == 0) {
			valid = 1;
		}
		free(buf);
	}
	free(str);
	//printf("entry %s\n", d->d_name);
	return valid;
}

char **batteries_list(unsigned int *size) {
	char **bats = NULL;
	struct dirent **eps;
	int n = scandir(POWER_DIR, &eps, bat_filter, alphasort);
	if (n >= 0) {
		//printf("got %d entries\n", n);
		*size = (unsigned int)n;
		bats = calloc(n, sizeof(char*));
		for (int i = 0; i < n; i++) {
			bats[i] = strjoin(POWER_DIR, eps[i]->d_name);
		}
	}
	return bats;
}

int battery_get_capacity(const char *filename) {
	char *str = strjoin(filename, "/capacity\0");
	char *buf = read_file(str);
	//printf("cap %s\n", buf);
	free(str);
	if (buf == NULL) {
		return -1;
	}
	int value = atoi(buf);
	free(buf);
	return value;
}

enum battery_status battery_get_status(const char *filename) {
	char *str = strjoin(filename, "/status\0");
	char *buf = read_file(str);
	//printf("%s: '%s'\n", str, buf);
	free(str);
	enum battery_status s = BS_UNKNOWN;
	if (!buf)
		return s;
	if (strcmp(buf, "Not charging\n") == 0)
		s = BS_NOT_CHARGING;
	if (strcmp(buf, "Charging\n") == 0)
		s = BS_CHARGING;
	if (strcmp(buf, "Discharging\n") == 0)
		s = BS_DISCHARGING;
	if (strcmp(buf, "Full\n") == 0)
		s = BS_FULL;
	free(buf);
	return s;
}

char *battery_get_name(const char *filename) {
	char *str = strjoin(filename, "/model_name\0");
	char *buf = read_file(str);
	free(str);
	return trim(buf);
}

extern bool read_file_bool(const char *filename) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
		return false;
	unsigned char c = fgetc(fp);
	fclose(fp);
	return c == '1';
}

extern char *read_file(const char *filename) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		return NULL;
	}
	char *buf = NULL;
	size_t len;
	ssize_t bytes_read = getdelim(&buf, &len, '\0', fp);
	fclose(fp);
	if (bytes_read == -1) {
		// Perhaps check if not null and free buf?
		return NULL;
	}
	return buf;
}

extern char *strjoin(const char *a, const char *b) {
	char *str = malloc(strlen(a) + strlen(b) + 1);
	strcpy(str, a);
   	strcat(str, b);
	return str;
}

char *ltrim(char *s) {
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s) {
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s) {
    return rtrim(ltrim(s));
}
