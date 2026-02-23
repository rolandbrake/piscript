#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <windows.h>
#include <shlwapi.h>
#include "commands.h"
#include "pi_shell.h"
#include "pi_compiler.h"
#include "pi_lex.h"
#include "pi_parser.h"
#include "pi_token.h"
#include "pi_vm.h"
#include "cart.h"
#include "builtin/pi_audio.h"

// Forward declaration to avoid implicit declaration warning
int num_commands();
void cmd_run(int argc, char **argv);

void cmd_help(int argc, char **argv)
{
    if (argc > 1)
    {
        for (int i = 0; i < num_commands(); i++)
        {
            if (strcmp(argv[1], commands[i].name) == 0)
            {
                char buffer[512];
                snprintf(buffer, sizeof(buffer), "%s - %s\n\n%s\n", commands[i].name, commands[i].description, commands[i].detailed_description);
                shell_io->out(buffer, SHELL_END);
                return;
            }
        }
        shell_io->out("Unknown command. Type 'help' for a list of commands.\n", SHELL_COLOR, 8, SHELL_END);
    }
    else
    {
        shell_io->vm->screen->cursor_y += 2;
        char buffer[256];
        shell_io->out("PISHELL - a simple shell for\npiscript\n\n", SHELL_COLOR, COLOR_BRIGHT_RED, SHELL_END);
        shell_io->out("Available commands:\n", SHELL_END);
        for (int i = 0; i < num_commands(); i++)
        {
            shell_io->out(commands[i].name, SHELL_COLOR, COLOR_BRIGHT_BLUE, SHELL_END);
            snprintf(buffer, sizeof(buffer), "  %s\n", commands[i].description);
            shell_io->out(buffer, SHELL_END);
        }
    }
}

void cmd_exit(int argc, char **argv)
{

    shell_io->out("exiting pishell.\n", SHELL_END);
    shell_stop();
}

/**
 * Clear the shell screen.
 *
 * This command will clear the shell screen if supported by the
 * current shell implementation.
 */
void cmd_clear(int argc, char **argv)
{

    if (shell_io && shell_io->clear)
        shell_io->clear(COLOR_BLACK);
}

void cmd_about(int argc, char **argv)
{

    uint8_t site_qr[50][50] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0},
        {0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0},
        {0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0},
        {0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
        {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
        {0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0},
        {6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6},
        {6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6},
        {6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6},
        {6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6},
        {0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6},
        {0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6},
        {6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6},
        {6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6},
        {6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0},
        {6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0},
        {6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6},
        {6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0},
        {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6},
        {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6},
        {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0},
        {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6},
        {0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 6, 6},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6},
        {0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6},
        {0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6},
        {0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 0, 0, 6, 6, 6, 6},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0},
    };

    uint8_t pi_icon[20][20] = {
        {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0, 9, 9},
        {9, 9, 0, 0, 0, 0, 0, 0, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0, 9, 9},
        {9, 9, 0, 0, 9, 9, 9, 0, 0, 9, 9, 0, 0, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 0, 0, 9, 9, 9, 0, 0, 9, 9, 0, 0, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 0, 0, 9, 9, 9, 0, 0, 9, 9, 0, 0, 9, 9, 9, 9, 9, 9, 9},
        {9, 9, 0, 0, 9, 9, 9, 0, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0, 9, 9},
        {9, 9, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 9, 0, 0, 0, 9, 9},
        {9, 9, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0, 0, 9, 9},
        {9, 9, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0, 0, 9, 9},
        {9, 9, 0, 0, 9, 9, 9, 9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 9, 9},
        {9, 9, 0, 0, 9, 9, 9, 9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 9, 9},
        {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
    };

    shell_io->clear(COLOR_BLACK);
    Screen *screen = shell_io->vm->screen;

    screen->cursor_y += 10;
    shell_io->out("PiScript Shell v0.1.0\n", SHELL_COLOR, COLOR_BRIGHT_RED, SHELL_END);
    shell_io->out("Created by: Roland Brake\n", SHELL_COLOR, COLOR_BRIGHT_BLUE, SHELL_END);
    shell_io->out("License: MIT\n", SHELL_COLOR, COLOR_WHITE, SHELL_END);
    shell_io->out("GitHub: https://github.com/\nrolandbrake \n", SHELL_COLOR, COLOR_WHITE, SHELL_END);

    int x = 64;
    int y = screen->cursor_y;
    for (int _y = 0; _y < 50; _y++)
        for (int _x = 0; _x < 50; _x++)
            set_pixel(screen, x + _x, y + _y, site_qr[_y][_x]);

    draw_rect(screen, x - 2, y - 2, 54, 54, COLOR_WHITE);

    screen->cursor_y += 60;
    shell_io->out("Scan the QR code to visit the\nproject website!\n", SHELL_COLOR, COLOR_BRIGHT_BLUE, SHELL_END);
    screen->cursor_y += 2;
}

void cmd_cd(int argc, char **argv)
{
    if (argc < 2)
    {
        char exe_path[MAX_PATH];
        GetModuleFileName(NULL, exe_path, MAX_PATH);
        PathRemoveFileSpec(exe_path); // Remove the executable name

        char cwd[MAX_PATH];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {

            char *path = (char *)malloc(MAX_PATH);
            if (!PathRelativePathTo(path, exe_path, FILE_ATTRIBUTE_DIRECTORY, cwd, FILE_ATTRIBUTE_DIRECTORY))
                shell_io->out("Error: Could not get path.\n", SHELL_COLOR, COLOR_BRIGHT_RED, SHELL_END);
            else
            {
                shell_io->out(path, SHELL_END);
                shell_io->out("\n", SHELL_END);
            }
            free(path);
        }
        else
            shell_io->out("Error: Could not get current dir.\n", SHELL_COLOR, COLOR_BRIGHT_RED, SHELL_END);
    }
    else
    {
        if (chdir(argv[1]) != 0)
            shell_io->out("Error: Could not change dir.\n", SHELL_COLOR, COLOR_BRIGHT_RED, SHELL_END);
    }
}

void cmd_dir(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : ".";
    DIR *dir = opendir(path);
    if (!dir)
    {
        shell_io->out("Error: Could not open directory.\n", SHELL_COLOR, COLOR_BRIGHT_RED, SHELL_END);
        return;
    }

    int lines = 0;
    int max_lines = (SCREEN_HEIGHT / CHAR_HEIGHT);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        const char *ext = strrchr(entry->d_name, '.');
        if (ext && (strcmp(ext, ".pi") == 0 || strcmp(ext, ".px") == 0))
        {
            char buffer[512];
            struct stat st;
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
            stat(fullpath, &st);
            long size_kb = (long)((st.st_size + 1023) / 1024);

            snprintf(buffer, sizeof(buffer), "%-18s ", entry->d_name);
            shell_io->out(buffer, SHELL_END);
            snprintf(buffer, sizeof(buffer), "%ld kB\n", size_kb);
            shell_io->out(buffer, SHELL_COLOR, COLOR_BRIGHT_RED, SHELL_END);
            lines++;

            if (lines >= max_lines)
            {
                shell_io->out("-- Press Enter for more --\n", SHELL_COLOR, COLOR_BRIGHT_BLUE, SHELL_END);
                SDL_Event event;
                bool waiting = true;
                while (waiting)
                {
                    while (SDL_PollEvent(&event))
                    {
                        if (event.type == SDL_KEYDOWN &&
                            (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER))
                        {
                            waiting = false;
                        }
                        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                        {
                            waiting = false;
                            closedir(dir); // Close the directory before returning
                            return;        // Exit the function immediately
                        }
                    }
                    SDL_Delay(10); // Small delay to prevent busy-waiting
                }
                lines = 0;
            }
        }
    }
    closedir(dir);
}

static void *vm_run(void *arg)
{
    vm_t *vm = (vm_t *)arg;
    clock_t start_time = clock();
    run(vm);
    pthread_mutex_lock(&vm->lock);
    vm->running = false;
    pthread_mutex_unlock(&vm->lock);
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;
    printf("Execution Time: %.4f ms\n", time_taken);
    return NULL;
}

#include <setjmp.h>

// Global jump buffer for REPL error handling
static jmp_buf repl_error_jump;

static void shell_loading(const char *label, int duration_ms)
{
    if (!shell_io || !shell_io->vm || !shell_io->vm->screen)
        return;

    (void)label;

    Screen *screen = shell_io->vm->screen;
    const int rect_w = 80;
    const int rect_h = 6;
    const int rect_x = (SCREEN_WIDTH - rect_w) / 2;
    const int rect_y = (SCREEN_HEIGHT - rect_h) / 2;

    shell_io->clear(COLOR_BRIGHT_BLUE);
    shell_io->out("loading...\n", SHELL_CURSOR_X, 40, SHELL_CURSOR_Y, 50, SHELL_END);
    draw_fillRect(screen, rect_x, rect_y, rect_w, rect_h, COLOR_WHITE);
    screen_update(screen);

    const int fill_w = rect_w - 2;
    const int fill_h = rect_h - 2;
    const int steps = fill_w > 0 ? fill_w : 1;
    const int step_delay = steps > 0 ? (duration_ms / steps) : 0;
    SDL_Delay(duration_ms);

    for (int i = 1; i <= steps; i++)
    {
        draw_fillRect(screen, rect_x + 1, rect_y + 1, i, fill_h, COLOR_BRIGHT_RED);
        screen_update(screen);
        if (step_delay > 0)
            SDL_Delay(step_delay);
    }
}

/**
 * @brief Custom error handler for the REPL.
 *
 * This function is called by the parser when an error occurs in REPL mode.
 * It prints the error message to the shell and uses longjmp to return
 * control back to the main REPL loop, preventing the application from exiting.
 *
 * @param message The error message from the parser.
 * @param line The line number where the error occurred.
 * @param column The column number where the error occurred.
 */
static void repl_error(const char *message, int line, int column)
{
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "Error: %s\n", message);
    shell_io->out(buffer, SHELL_COLOR, 8, SHELL_END);
    longjmp(repl_error_jump, 1);
    // return;
}

static void run_error(const char *message, int line, int column)
{
    (void)line;
    (void)column;

    fprintf(stderr, "Runtime error: %s\n", message);

    if (shell_io && shell_io->vm)
    {
        pthread_mutex_lock(&shell_io->vm->lock);
        shell_io->vm->running = false;
        pthread_mutex_unlock(&shell_io->vm->lock);
    }

    pthread_exit(NULL);
}

static void run_repl(vm_t *vm)
{

    uint8_t repl_icon[12][100] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 12, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 8, 8, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 8, 8, 8, 6, 1, 8, 8, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 6, 1, 8, 8, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 6, 1, 8, 8, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 6, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 8, 8, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 8, 8, 8, 6, 1, 8, 8, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 6, 1, 12, 12, 12, 6, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 6, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 8, 8, 8, 6, 1, 1, 8, 8, 6, 1, 8, 8, 8, 8, 8, 6, 1, 8, 8, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 12, 12, 6, 1, 12, 6, 1, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 1, 1, 1, 8, 8, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 12, 12, 6, 1, 12, 6, 1, 12, 12, 6, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 1, 1, 1, 8, 8, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 6, 1, 12, 6, 1, 12, 12, 6, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 12, 12, 12, 6, 1, 1, 1, 1, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 1, 1, 1, 8, 8, 8, 8, 8, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 12, 12, 6, 1, 1, 1, 1, 1, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 6, 1, 12, 6, 1, 12, 12, 6, 1, 12, 12, 12, 12, 12, 6, 1, 12, 12, 12, 12, 12, 6, 1, 1, 1, 1, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 8, 8, 6, 1, 1, 1, 1, 8, 8, 8, 8, 8, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    };
    char input[MAX_SHELL_INPUT_LENGTH];
    bool running = true;

    // Set the custom error handler for the REPL session
    set_errorHandler(repl_error);

    compiler_t *comp = init_compiler();
    parser_t *parser = NULL;

    /* save previous shell history */
    char prev_history[HISTORY_MAX][LINE_MAX];
    int prev_history_count = history_count;
    int prev_history_pos = history_pos;

    memcpy(prev_history, history, sizeof(history));

    /* start fresh history for REPL */
    history_count = 0;
    history_pos = 0;
    memset(history, 0, sizeof(history));

    shell_io->clear(COLOR_BLACK);

    draw_rect(vm->screen, 2, 2, SCREEN_WIDTH - 6, 35, COLOR_BRIGHT_RED);
    draw_fillRect(vm->screen, 3, 3, SCREEN_WIDTH - 8, 34, COLOR_DARK_BLUE);
    draw_matrix(vm->screen, 14, 4, 100, 12, repl_icon);

    shell_io->out("Type 'exit' to exit REPL.\n", SHELL_CURSOR_X, 6, SHELL_CURSOR_Y, 20, SHELL_COLOR, COLOR_WHITE, SHELL_END);
    shell_io->out("Type 'cls' to clear\n", SHELL_CURSOR_X, 6, SHELL_COLOR, COLOR_WHITE, SHELL_END);
    vm->screen->cursor_y = 40;
    while (running)
    {
        if (setjmp(repl_error_jump))
        {
            if (parser)
            {
                free_parser(parser);
                parser = NULL;
            }
            continue;
        }

        shell_io->in(input, sizeof(input), REPL_MODE, true, SHELL_END);
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0)
        {
            shell_io->clear(COLOR_BLACK);
            running = false;
            continue;
        }
        else if (strcmp(input, "cls") == 0)
        {
            shell_io->clear(COLOR_BLACK);
            continue;
        }
        else if (strcmp(input, "") == 0)
            continue;

        // reset_compiler(comp);
        init_scanner(input);
        token_t *tokens = scan();
        compiler_t *comp = init_compiler();
        parser = init_parser(comp, tokens, MODE_REPL);
        parse(parser);

        vm_reset(vm, comp);
        run(vm);

        if (vm->sp > vm->bp)
        {
            Value result = vm->stack[--vm->sp]; // Pop result
            char *_result = as_string(result);
            shell_io->out(_result, SHELL_COLOR, 6, SHELL_END);
            shell_io->out("\n", SHELL_END);
            free(_result);
        }

        free_parser(parser);
        parser = NULL;
    }

    /* restore previous shell history */
    memcpy(history, prev_history, sizeof(history));
    history_count = prev_history_count;
    history_pos = prev_history_pos;

    // free_compiler(comp);
    set_errorHandler(NULL);
}

void cmd_run(int argc, char **argv)
{
    if (argc < 2)
    {
        run_repl(shell_io->vm);
        return;
    }

    char *filename = argv[1];
    const char *ext = strrchr(filename, '.');
    char *source = NULL;
    bool is_cart = false;

    if (shell_io->vm->cart)
    {
        cart_free(shell_io->vm->cart);
        shell_io->vm->cart = NULL;
    }

    if (ext && strcmp(ext, ".px") == 0)
    {
        is_cart = true;
        Cart *cart = cart_load(filename);
        if (!cart)
        {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "Error: Could not open or read cartridge '%s'.\n", filename);
            shell_io->out(buffer, SHELL_COLOR, 8, SHELL_END);
            return;
        }
        source = (char *)cart->code;
        shell_io->vm->cart = cart;
    }
    else if (ext && strcmp(ext, ".pi") == 0)
    {
        FILE *file = fopen(filename, "rb");
        if (!file)
        {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "Error: Could not open file '%s'.\n", filename);
            shell_io->out(buffer, SHELL_COLOR, 8, SHELL_END);
            return;
        }

        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);

        source = malloc(length + 1);
        if (!source)
        {
            shell_io->out("Error: Not enough memory to read file.\n", SHELL_COLOR, 8, SHELL_END);
            fclose(file);
            return;
        }

        fread(source, 1, length, file);
        source[length] = '\0';
        fclose(file);
    }
    else
    {
        shell_io->out("Error: File must have a .pi or .px extension.\n", SHELL_COLOR, 8, SHELL_END);
        return;
    }

    shell_loading("Loading", 200);

    // Compile the source code
    init_scanner(source);
    token_t *tokens = scan();
    compiler_t *comp = init_compiler();
    parser_t *parser = init_parser(comp, tokens, MODE_FILE);
    parse(parser);

    // Reset the existing VM instead of creating a new one
    vm_t *vm = shell_io->vm;
    vm_reset(vm, comp);

    set_errorHandler(run_error);

    // Remove loading UI before execution so non-rendering scripts don't appear stuck.
    shell_io->clear(COLOR_BLACK);

    // Clear stale keyboard/text events from the previous run screen.
    SDL_FlushEvent(SDL_KEYDOWN);
    SDL_FlushEvent(SDL_KEYUP);
    SDL_FlushEvent(SDL_TEXTINPUT);

    pthread_t vm_thread;
    vm->running = true;
    pthread_create(&vm_thread, NULL, vm_run, vm);

    int frame_count = 0;
    double fps = 0.0;
    Uint32 last_time = SDL_GetTicks();
    SDL_Event event;

    clock_t start_time;
    clock_t end_time;
    start_time = clock();

    /* shell-side event loop */
    while (1)
    {
        pthread_mutex_lock(&vm->lock);
        bool is_running = vm->running;
        pthread_mutex_unlock(&vm->lock);
        // Keep pumping events until audio playback completes, even if VM code has finished.
        if (!is_running && !audio_isPlaying())
            break;

        Uint32 frame_start = SDL_GetTicks();

        if (SDL_WaitEventTimeout(&event, 1000 / TARGET_FPS))
        {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
            {
                pthread_mutex_lock(&vm->lock);
                vm->running = false;                     
                pthread_mutex_unlock(&vm->lock);
                audio_stopAll();       
                SDL_Delay(1000 / TARGET_FPS);
            }
        }

        frame_count++;
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_time >= 1000)
        {
            fps = frame_count / ((current_time - last_time) / 1000.0);
            frame_count = 0;
            last_time = current_time;
            vm->fps = fps;
        }

        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < 1000 / TARGET_FPS)
            SDL_Delay((1000 / TARGET_FPS) - frame_time);
    }

    end_time = clock();
    double time_taken = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;

    pthread_join(vm_thread, NULL);
    set_errorHandler(NULL);

    uint8_t cart[30][30] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 0, 0, 0, 0},
        {0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 22, 22, 22, 0, 0, 0},
        {0, 0, 9, 9, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 22, 22, 22, 0, 0, 0},
        {0, 0, 9, 6, 6, 6, 6, 6, 6, 6, 8, 8, 12, 12, 12, 12, 5, 12, 12, 5, 8, 8, 9, 9, 22, 22, 22, 0, 0, 0},
        {0, 0, 9, 6, 5, 5, 5, 6, 5, 6, 8, 8, 12, 5, 8, 12, 5, 8, 8, 8, 8, 8, 9, 9, 22, 22, 22, 0, 0, 0},
        {0, 0, 9, 6, 6, 6, 6, 6, 6, 6, 8, 8, 12, 12, 12, 12, 5, 12, 12, 5, 8, 8, 9, 9, 22, 22, 22, 0, 0, 0},
        {0, 0, 9, 6, 5, 5, 5, 5, 5, 6, 8, 8, 12, 12, 5, 8, 8, 12, 12, 5, 8, 8, 9, 9, 22, 22, 22, 0, 0, 0},
        {0, 0, 9, 6, 6, 6, 6, 6, 6, 6, 8, 8, 12, 12, 5, 8, 8, 12, 12, 5, 8, 8, 9, 9, 22, 22, 22, 0, 0, 0},
        {0, 0, 9, 6, 6, 6, 6, 6, 6, 6, 8, 8, 12, 12, 5, 8, 8, 12, 12, 5, 8, 8, 9, 9, 22, 22, 22, 0, 0, 0},
        {0, 0, 9, 9, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 22, 22, 22, 0, 0, 0},
        {0, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 22, 22, 22, 0, 0, 0},
        {0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 22, 22, 22, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

    shell_io->clear(COLOR_BLACK);

    const int cart_x = 49;
    const int cart_y = 20;
    int cart_offset = 0;
    int cart_dir = 1;
    const int cart_bounce = 2;
    Uint32 last_cart_anim = SDL_GetTicks();
    const Uint32 cart_anim_interval_ms = 120;

    for (int y = 0; y < 30; y++)
        for (int x = 0; x < 30; x++)
            set_pixel(vm->screen, x + cart_x, y + cart_y + cart_offset, cart[y][x]);
    screen_update(vm->screen);

    vm->screen->cursor_y = 55;

    // Clear the screen before returning to the shell prompt
    shell_io->out("Program terminated.\n", SHELL_CURSOR_X, 20, SHELL_COLOR, COLOR_WHITE, SHELL_END);
    char time_buffer[64];
    snprintf(time_buffer, sizeof(time_buffer), "Execution time: %.4f ms\n", time_taken);
    shell_io->out(time_buffer, SHELL_CURSOR_X, 10, SHELL_COLOR, COLOR_BRIGHT_RED, SHELL_END);

    vm->screen->cursor_y += 30;
    shell_io->out("Press any key to continue...\n", SHELL_CURSOR_X, 10, SHELL_COLOR, COLOR_BRIGHT_BLUE, SHELL_END);

    bool waiting = true;

    while (waiting)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN || event.type == SDL_TEXTINPUT)
            {
                waiting = false;
                shell_io->clear(COLOR_BLACK);
            }
            else if (event.type == SDL_QUIT)
                return;
        }

        Uint32 now = SDL_GetTicks();
        if (now - last_cart_anim >= cart_anim_interval_ms)
        {
            draw_fillRect(vm->screen, cart_x, cart_y + cart_offset, 30, 30, COLOR_BLACK);
            cart_offset += cart_dir;
            if (cart_offset >= cart_bounce || cart_offset <= -cart_bounce)
                cart_dir = -cart_dir;

            for (int y = 0; y < 30; y++)
                for (int x = 0; x < 30; x++)
                    set_pixel(vm->screen, x + cart_x, y + cart_y + cart_offset, cart[y][x]);
            screen_update(vm->screen);
            last_cart_anim = now;
        }
        SDL_Delay(10);
    }

    // Avoid carrying the "continue" key press into the next shell prompt/run.
    SDL_FlushEvent(SDL_KEYDOWN);
    SDL_FlushEvent(SDL_KEYUP);
    SDL_FlushEvent(SDL_TEXTINPUT);

    // Cleanup
    free_parser(parser);
    if (is_cart)
    {
        // The cart owns the source, so we free the cart
        if (shell_io->vm->cart)
        {
            cart_free(shell_io->vm->cart);
            shell_io->vm->cart = NULL;
        }
    }
    else
        // We allocated source with malloc for .pi files
        free(source);
}

const command_t commands[] = {
    {"help", "shows this help message.", "Usage: help [command]", cmd_help},
    {"exit", "exits the shell.", "This command will terminate the piscript shell.", cmd_exit},
    {"clear", "clears the screen.", "This command will clear all text from the screen.", cmd_clear},
    {"run", "runs a .pi/.px script/REPL.", "Usage: run <filename.pi | filename.px>\nThis command will execute a piscript file or cartridge.", cmd_run},
    {"about", "info about the shell.", "Shows information about the piscript shell.", cmd_about},
    {"dir", "lists .pi/.px files.", "Usage: dir [directory]\nLists all files with the .pi extension in the current directory, or the specified directory.", cmd_dir},
    {"cd", "changes/prints the current\ndirectory.", "Usage: cd [directory]\nChanges the current working directory or prints it if no directory is provided.", cmd_cd},
};

int num_commands()
{
    return sizeof(commands) / sizeof(command_t);
}
