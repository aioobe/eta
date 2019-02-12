#ifndef ETA_H
#define ETA_H

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define SUCCESS 0
#define INVALID_COMMAND_LINE_ARGUMENTS 1
#define COMMAND_EXECUTION_FAILED 2
#define COULD_NOT_PARSE_COMMAND_OUTPUT 3

typedef long long value_t;

extern char *program_name;

#endif /* ETA_H */
