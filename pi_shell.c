#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pi_shell.h"
#include "commands.h"
#include <stdbool.h>
#include <math.h>
#include "screen.h"

#define MAX_ARGS 64

shell_io_t *shell_io = NULL;
static bool shell_running = false;
bool shell_home_visible = false;

static const int pi_logo[12][50] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 9, 9, 9, 9, 9, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 8, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 9, 9, 0},
    {0, 9, 9, 8, 0, 9, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 8, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 9, 0, 0},
    {0, 9, 9, 8, 0, 9, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 9, 9, 0},
    {0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 8, 0, 0, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 8, 0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 0},
    {0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 8, 0, 9, 9, 9, 8, 0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 8, 0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 0},
    {0, 9, 9, 8, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 9, 9, 8, 0, 9, 8, 0, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 0},
    {0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 8, 0, 9, 9, 8, 0, 9, 8, 0, 9, 9, 8, 0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 0},
    {0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 8, 0, 9, 8, 0, 9, 9, 8, 0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 9, 9, 9, 8, 0, 0},
    {0, 9, 9, 8, 0, 0, 0, 0, 9, 9, 8, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 8, 0, 9, 8, 0, 9, 9, 8, 0, 9, 9, 9, 9, 9, 8, 0, 9, 9, 9, 9, 9, 8, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

// Global definitions for history variables
char history[HISTORY_MAX][LINE_MAX];
int history_count = 0; // total stored
int history_pos = 0;   // current navigation index

void draw_logo(int x, int y)
{
    for (int _y = 0; _y < LOGO_H; _y++)
        for (int _x = 0; _x < LOGO_W; _x++)
            set_pixel(shell_io->vm->screen, x + _x, y + _y, pi_logo[_y][_x]);
}

void draw_logoWave(int x, int y, double phase)
{
    for (int _x = 0; _x < LOGO_W; _x++)
    {
        int offset = (int)lround(sin(phase + (_x * 0.35)) * 2.0);
        for (int _y = 0; _y < LOGO_H; _y++)
            set_pixel(shell_io->vm->screen, x + _x, y + _y + offset, pi_logo[_y][_x]);
    }
}

void shell_stop(void)
{

    printf("Shutting down...\n");
    shell_running = false;
    screen_close(shell_io->vm->screen);
    free_vm(shell_io->vm);
    free(shell_io);
}

void shell_run(shell_io_t *io)
{
    shell_io = io;
    shell_running = true;

    char line[MAX_SHELL_INPUT_LENGTH];
    char *args[MAX_ARGS];
    int argc;

    Screen *screen = shell_io->vm->screen;

    draw_logo(LOGO_X, LOGO_Y);
    shell_home_visible = true;

    screen->cursor_y = 18;

    shell_io->out("PISHELL ", SHELL_COLOR, COLOR_BRIGHT_RED, SHELL_CURSOR_X, 4, SHELL_END);
    shell_io->out("TYPE 'HELP' FOR A List\n", SHELL_COLOR, COLOR_DARK_GRAY, SHELL_END);
    shell_io->out("OF COMMANDS.\n", SHELL_COLOR, COLOR_DARK_GRAY, SHELL_CURSOR_X, 4, SHELL_END);
    shell_io->out("created by Roland Brake.\n", SHELL_COLOR, COLOR_BRIGHT_BLUE, SHELL_CURSOR_X, 4, SHELL_END);

    while (shell_running)
    {
        shell_io->in(line, MAX_SHELL_INPUT_LENGTH);

        // If shell was stopped during input, exit loop
        if (!shell_running)
            break;

        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;

        // Skip empty lines
        if (strlen(line) == 0)
            continue;

        // Tokenize input
        argc = 0;
        char *token = strtok(line, " \t\r\n\a");
        while (token != NULL && argc < MAX_ARGS - 1)
        {
            args[argc++] = token;
            token = strtok(NULL, " \t\r\n\a");
        }
        args[argc] = NULL;

        if (argc == 0)
            continue;

        // Find and execute the command
        int found = 0;
        for (int i = 0; i < num_commands(); i++)
        {
            if (strcmp(args[0], commands[i].name) == 0)
            {
                shell_home_visible = false;
                commands[i].fn(argc, args);
                found = 1;
                break;
            }
        }

        if (!found)
        {
            char buffer[128];
            shell_io->out("Unknown command: ", SHELL_COLOR, 8, SHELL_END);
            snprintf(buffer, sizeof(buffer), "'%s'.\ntype 'help' for a list of\ncommands.\n", args[0]);
            shell_io->out(buffer);
        }
    }
    shell_io = NULL; // cleanup
}
