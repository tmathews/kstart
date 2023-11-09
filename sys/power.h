#if !defined(BATT)
#define BATT

#include <stdbool.h>

enum battery_status {
	BS_UNKNOWN,
	BS_NOT_CHARGING,
	BS_DISCHARGING,
	BS_CHARGING,
	BS_FULL,
};

bool ac_is_online();
char **batteries_list(unsigned int *);
enum battery_status battery_get_status(const char *filename);
int battery_get_capacity(const char *filename);
char *battery_get_name(const char *filename);

extern bool read_file_bool(const char *filename);
extern char *read_file(const char *filename);
extern char *strjoin(const char *a, const char *b);
char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);

#endif
