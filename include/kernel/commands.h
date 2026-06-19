
#include <stdbool.h>

#define MAX_PARTITIONS 10

typedef int (*command_func_t)(char *args);
typedef struct {
  const char *name;
  const char *description;
  command_func_t function;
} shell_command_t;

#define MAX_ENV_VARS 20
#define VAR_NAME_LEN 32
#define VAR_VAL_LEN 64

typedef struct {
  char name[VAR_NAME_LEN];
  char value[VAR_VAL_LEN];
  bool active;
} env_var_t;

extern shell_command_t commands_table[];
extern void list_matches(char *prefix);

#define MAX_ALIAS 20
#define ALIAS_NAME_LEN 16
#define ALIAS_CMD_LEN 64

typedef struct {
  char name[ALIAS_NAME_LEN];
  char command[ALIAS_CMD_LEN];
  bool active;
} alias_t;

typedef struct {
  char name[8];
  uint32_t start_lba;
  uint32_t sectors;
  uint8_t type;
  bool active;
} partition_t;
