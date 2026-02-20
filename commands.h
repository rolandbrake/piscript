#ifndef COMMANDS_H
#define COMMANDS_H

// Command handler function pointer
typedef void (*cmd_fn)(int argc, char **argv);

// Structure to represent a shell command
typedef struct
{
    const char *name;
    const char *description;
    const char *detailed_description;
    cmd_fn fn;
} command_t;

// Command handlers
void cmd_help(int argc, char **argv);
void cmd_exit(int argc, char **argv);
void cmd_clear(int argc, char **argv);
void cmd_run(int argc, char **argv);
void cmd_dir(int argc, char **argv);
void cmd_about(int argc, char **argv);

// Array of available commands
extern const command_t commands[];

// Function to get the number of commands
int num_commands();

#endif // COMMANDS_H
