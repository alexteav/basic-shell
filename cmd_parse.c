// teav@pdx.edu
// Student: Alex Teav
// Lab 3

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "cmd_parse.h"

// I have this a global so that I don't have to pass it to every
// function where I might want to use it. Yes, I know global variables
// are frowned upon, but there are a couple useful uses for them.
// This is one.
unsigned short isVerbose = 0;

int 
process_user_input_simple(void)
{
    char str[MAX_STR_LEN];
    char *ret_val;
    char *raw_cmd;
    cmd_list_t *cmd_list = NULL;
    int cmd_count = 0;
    char prompt[30];
    char *history[HISTORY_SIZE] = {0};
    memset(history, 0, sizeof(history));

    // Set up a cool user prompt.
    sprintf(prompt, PROMPT_STR " %s β: ", getenv("LOGNAME"));
    for ( ; ; ) {
        fputs(prompt, stdout);
        memset(str, 0, MAX_STR_LEN);
        ret_val = fgets(str, MAX_STR_LEN, stdin);

        if (NULL == ret_val) {
            // end of input, a control-D was pressed.
            // Bust out of the input loop and go home.
            break;
        }

        // STOMP on the pesky trailing newline returned from fgets().
        if (str[strlen(str) - 1] == '\n') {
            str[strlen(str) - 1] = '\0';
        }
        if (strlen(str) == 0) {
            // An empty command line.
            // Just jump back to the promt and fgets().
            // Don't start telling me I'm going to get cooties by
            // using continue.
            continue;
        }

        update_history(history, str);

        if (strcmp(str, QUIT_CMD) == 0) {
            // Pickup your toys and go home. I just hope there are not
            // any memory leaks. ;-)
            break;
        }
        // Basic commands are pipe delimited.
        // This is really for Stage 2.
        raw_cmd = strtok(str, PIPE_DELIM);

        cmd_list = (cmd_list_t *) calloc(1, sizeof(cmd_list_t));

        // This block should probably be put into its own function.
        cmd_count = 0;
        while (raw_cmd != NULL ) {
            cmd_t *cmd = (cmd_t *) calloc(1, sizeof(cmd_t));

            cmd->raw_cmd = strdup(raw_cmd);
            cmd->list_location = cmd_count++;

            if (cmd_list->head == NULL) {
                // An empty list.
                cmd_list->tail = cmd_list->head = cmd;
            }
            else {
                // Make this the last in the list of cmds
                cmd_list->tail->next = cmd;
                cmd_list->tail = cmd;
            }
            cmd_list->count++;

            // Get the next raw command.
            raw_cmd = strtok(NULL, PIPE_DELIM);
        }
        // Now that I have a linked list of the pipe delimited commands,
        // go through each individual command.
        parse_commands(cmd_list);

        // This is a really good place to call a function to exec the
        // the commands just parsed from the user's command line.
        exec_commands(cmd_list, history);

        // We (that includes you) need to free up all the stuff we just
        // allocated from the heap. That linked list of linked lists looks
        // like it will be nasty to free up, but just follow the memory.
        free_list(cmd_list);
        cmd_list = NULL;
    }

    for (int i = 0; i < HISTORY_SIZE; ++i) // free history memory
    {
        if (history[i] != NULL)
        {
            free(history[i]);
        }
    }
        
    return(EXIT_SUCCESS);
}

void 
simple_argv(int argc, char *argv[] )
{
    int opt;

    while ((opt = getopt(argc, argv, "hv")) != -1) {
        switch (opt) {
        case 'h':
            // help
            // Show something helpful
            fprintf(stdout, "You must be out of your Vulcan mind if you think\n"
                    "I'm going to put helpful things in here.\n\n");
            exit(EXIT_SUCCESS);
            break;
        case 'v':
            // verbose option to anything
            // I have this such that I can have -v on the command line multiple
            // time to increase the verbosity of the output.
            isVerbose++;
            if (isVerbose) {
                fprintf(stderr, "verbose: verbose option selected: %d\n"
                        , isVerbose);
            }
            break;
        case '?':
            fprintf(stderr, "*** Unknown option used, ignoring. ***\n");
            break;
        default:
            fprintf(stderr, "*** Oops, something strange happened <%c> ... ignoring ...***\n", opt);
            break;
        }
    }
}

void 
exec_commands(cmd_list_t *cmds, char *history_list[]) 
{
    cmd_t *cmd = cmds->head;

    if (1 == cmds->count) {
        if (!cmd->cmd) {
            // if it is an empty command, bail.
            return;
        }
        if (0 == strcmp(cmd->cmd, CD_CMD)) {
            char str[MAXPATHLEN];
            
            if (0 == cmd->param_count) {
                // Just a "cd" on the command line without a target directory
                // need to cd to the HOME directory.

                // Is there an environment variable, somewhere, that contains
                // the HOME directory that could be used as an argument to
                // the chdir() fucntion?

                chdir(getenv("HOME"));
                getcwd(str, MAXPATHLEN); 
                printf(" " CWD_CMD ": %s\n", str);
            }
            else {
                // try and cd to the target directory. It would be good to check
                // for errors here.

                if (0 == chdir(cmd->param_list->param)) {
                    // a happy chdir!  ;-)
                    printf("\n" CWD_CMD ": %s\n", getcwd(str, MAXPATHLEN));
                }
                else {
                    printf("\nDirectory not found\n");
                }
            }
        }
        else if (0 == strcmp(cmd->cmd, CWD_CMD)) {
            char str[MAXPATHLEN];

            // Fetch the Current Working Wirectory (CWD).
            // aka - get country western dancing
            getcwd(str, MAXPATHLEN); 
            printf(" " CWD_CMD ": %s\n", str);
        }
        else if (0 == strcmp(cmd->cmd, ECHO_CMD)) {
            // insert code here
            // insert code here
            // Is that an echo?
            param_t *current = cmd->param_list;

            while (current)
            {
                printf("%s ", current->param);
                current = current->next;
            }

            printf("\n");
        }
        else if (0 == strcmp(cmd->cmd, "history")) {
            // insert code here for history command
            int counter = 1;

            for (int i = 0; i < HISTORY_SIZE; ++i)
            {
                if (history_list[i] != 0)
                {
                    printf("\n%d\t%s", counter, history_list[i]);
                    ++counter;
                }
            }

            printf("\n");
        }
        else {
            // A single command to create and exec
            // If you really do things correctly, you don't need a special call
            // for a single command, as distinguished from multiple commands.
            char *command = cmd->cmd;
            char *command_parameters[cmd->param_count + 1 + 1];
            param_t *current = cmd->param_list;
            int i = 1;
            
            memset(command_parameters, 0, sizeof(command_parameters));
            command_parameters[0] = strdup(cmd->cmd);

            while (current)
            {
                command_parameters[i] = strdup(current->param);
                current = current->next;
                ++i;
            }

            if (fork() == 0)
            {

                if (cmd->output_dest == REDIRECT_FILE)
                {
                    int redirect_output = open(cmd->output_file_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);

                    if (redirect_output < 0)
                    {
                        fprintf(stderr, "output redirection failed\n");
                        exit(7);
                    }

                    dup2(redirect_output, STDOUT_FILENO);
                    close(redirect_output);
                }

                if (cmd->input_src == REDIRECT_FILE)
                {
                    int redirect_input = open(cmd->input_file_name, O_RDONLY);

                    if (redirect_input < 0)
                    {
                        fprintf(stderr, "input redirection failed\n");
                        exit(7);
                    }

                    dup2(redirect_input, STDIN_FILENO);
                    close(redirect_input);
                }

                if (execvp(command, command_parameters) == -1)
                {
                    //fprintf(stderr, "ERROR: could not execute %s\n", command); // use perror instead
                    perror("ERROR: could not execute program");
                    _exit(EXIT_FAILURE);
                }
            }
            
            else
            {
                wait(NULL);

                for (int j = 0; j < cmd->param_count + 1 + 1; ++j) // free memory
                {
                    if (command_parameters[j])
                    {
                        free(command_parameters[j]);
                        command_parameters[j] = NULL;
                    }
                }
            }

            if (command_parameters[0])
            {
                free(command_parameters[0]);
                command_parameters[0] = NULL;
            }
        }
    }
    else {
        // Stage 2 piping

        int two_pipes[2]; // 1 for reading and 1 for writing
        pid_t process_id;
        int p_trail = STDIN_FILENO;

        while(cmd)
        {
            if (cmd != cmds->tail)
            {
                pipe(two_pipes);
            }

            process_id = fork();

            if (process_id < 0) // error checking
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            else if (process_id == 0)
            {
                // build ragged array here////////////////////////
                char *command = cmd->cmd;
                char *command_parameters[cmd->param_count + 1 + 1];
                param_t *current = cmd->param_list;
                int i = 1;
            
                memset(command_parameters, 0, sizeof(command_parameters));
                command_parameters[0] = strdup(cmd->cmd);

                while (current)
                {
                    command_parameters[i] = strdup(current->param);
                    current = current->next;
                    ++i;
                }
                ///////////////////////////////////////////////

                if (cmd != cmds->head)
                {
                    dup2(p_trail, STDIN_FILENO);
                }

                if (cmd->output_dest == REDIRECT_FILE)
                {
                    int redirect_output = open(cmd->output_file_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);

                    if (redirect_output < 0)
                    {
                        fprintf(stderr, "output redirection failed\n");
                        exit(7);
                    }

                    dup2(redirect_output, STDOUT_FILENO);
                    close(redirect_output);
                }

                if (cmd->input_src == REDIRECT_FILE)
                {
                    int redirect_input = open(cmd->input_file_name, O_RDONLY);

                    if (redirect_input < 0)
                    {
                        fprintf(stderr, "input redirection failed\n");
                        exit(7);
                    }

                    dup2(redirect_input, STDIN_FILENO);
                    close(redirect_input);
                }

                if (cmd->next)
                {
                    dup2(two_pipes[STDOUT_FILENO], STDOUT_FILENO);
                    close(two_pipes[STDOUT_FILENO]);
                    close(two_pipes[STDIN_FILENO]);
                }

                if (execvp(command, command_parameters) == -1)
                {
                    //fprintf(stderr, "ERROR: could not execute %s\n", command); // use perror instead
                    perror("ERROR: could not execute program");
                    exit(EXIT_FAILURE);
                }
            }

            //else // else statement not required
            //{
                if (cmd != cmds->head)
                {
                    close(p_trail);
                }

                if (cmd != cmds->tail)
                {
                    close(two_pipes[STDOUT_FILENO]);
                    p_trail = two_pipes[STDIN_FILENO];
                }
            //}

            cmd = cmd->next;
        }

        while((wait(NULL)) >= 0);
    }
}

void
update_history(char *history_list[], char *command)
{
    for (int i = 0; i < HISTORY_SIZE; ++i)
    {
        if (history_list[i] == 0)
        {
            history_list[i] = strdup(command);
            return;
        }
    }

    free(history_list[0]);
    memmove(&(history_list[0]), &(history_list[1]), (HISTORY_SIZE - 1) * sizeof(char *));
    history_list[9] = strdup(command);

}

void
free_list(cmd_list_t *cmd_list)
{
    // Proof left to the student.
    // You thought I was going to do this for you! HA! You get
    // the enjoyment of doing it for yourself.
    cmd_t *current = cmd_list->head;
    cmd_t *temp = NULL;

    while (current != NULL)
    {
        free_cmd(current);
        temp = current->next;
        free(current);
        current = NULL;
        current = temp;
    }

    if (cmd_list)
    {
        free(cmd_list);
    }
}


void
free_cmd (cmd_t *cmd)
{
    // Proof left to the student.
    // Yep, on yer own.
    // I beleive in you.
    if (cmd->raw_cmd)
    {
        free(cmd->raw_cmd);
    }

    if (cmd->cmd)
    {
        free(cmd->cmd);
    }

    if (cmd->input_file_name)
    {
        free(cmd->input_file_name);
    }

    if (cmd->output_file_name)
    {
        free(cmd->output_file_name);
    }

    if (cmd->param_list)
    {
        param_t *current = cmd->param_list;
        param_t *temp = NULL;

        while (current != NULL)
        {
            if (current->param)
            {
                free(current->param);
            }

            temp = current->next;
            free(current);
            current = NULL;
            current = temp;
        }
    }
}

void print_list(cmd_list_t *cmd_list)
{
    cmd_t *cmd = cmd_list->head;

    while (NULL != cmd)
    {
        print_cmd(cmd);
        cmd = cmd->next;
    }
}

// Oooooo, this is nice. Show the fully parsed command line in a nice
// easy to read and digest format.
void
print_cmd(cmd_t *cmd)
{
    param_t *param = NULL;
    int pcount = 1;

    fprintf(stderr,"raw text: +%s+\n", cmd->raw_cmd);
    fprintf(stderr,"\tbase command: +%s+\n", cmd->cmd);
    fprintf(stderr,"\tparam count: %d\n", cmd->param_count);
    param = cmd->param_list;

    while (NULL != param) {
        fprintf(stderr,"\t\tparam %d: %s\n", pcount, param->param);
        param = param->next;
        pcount++;
    }

    fprintf(stderr,"\tinput source: %s\n"
            , (cmd->input_src == REDIRECT_FILE ? "redirect file" :
               (cmd->input_src == REDIRECT_PIPE ? "redirect pipe" : "redirect none")));
    fprintf(stderr,"\toutput dest:  %s\n"
            , (cmd->output_dest == REDIRECT_FILE ? "redirect file" :
               (cmd->output_dest == REDIRECT_PIPE ? "redirect pipe" : "redirect none")));
    fprintf(stderr,"\tinput file name:  %s\n"
            , (NULL == cmd->input_file_name ? "<na>" : cmd->input_file_name));
    fprintf(stderr,"\toutput file name: %s\n"
            , (NULL == cmd->output_file_name ? "<na>" : cmd->output_file_name));
    fprintf(stderr,"\tlocation in list of commands: %d\n", cmd->list_location);
    fprintf(stderr,"\n");
}

// Remember how I told you that use of alloca() is
// dangerous? You can trust me. I'm a professional.
// And, if you mention this in class, I'll deny it
// ever happened. What happens in stralloca stays in
// stralloca.
#define stralloca(_R,_S) {(_R) = alloca(strlen(_S) + 1); strcpy(_R,_S);}

void
parse_commands(cmd_list_t *cmd_list)
{
    cmd_t *cmd = cmd_list->head;
    char *arg;
    char *raw;

    while (cmd) {
        // Because I'm going to be calling strtok() on the string, which does
        // alter the string, I want to make a copy of it. That's why I strdup()
        // it.
        // Given that command lines should not be tooooo long, this might
        // be a reasonable place to try out alloca(), to replace the strdup()
        // used below. It would reduce heap fragmentation.
        //raw = strdup(cmd->raw_cmd);

        // Following my comments and trying out alloca() in here. I feel the rush
        // of excitement from the pending doom of alloca(), from a macro even.
        // It's like double exciting.
        stralloca(raw, cmd->raw_cmd);

        arg = strtok(raw, SPACE_DELIM);
        if (NULL == arg) {
            // The way I've done this is like ya'know way UGLY.
            // Please, look away.
            // If the first command from the command line is empty,
            // ignore it and move to the next command.
            // No need free with alloca memory.
            //free(raw);
            cmd = cmd->next;
            // I guess I could put everything below in an else block.
            continue;
        }
        // I put something in here to strip out the single quotes if
        // they are the first/last characters in arg.
        if (arg[0] == '\'') {
            arg++;
        }
        if (arg[strlen(arg) - 1] == '\'') {
            arg[strlen(arg) - 1] = '\0';
        }
        cmd->cmd = strdup(arg);
        // Initialize these to the default values.
        cmd->input_src = REDIRECT_NONE;
        cmd->output_dest = REDIRECT_NONE;

        while ((arg = strtok(NULL, SPACE_DELIM)) != NULL) {
            if (strcmp(arg, REDIR_IN) == 0) {
                // redirect stdin
                //
                // If the input_src is something other than REDIRECT_NONE, then
                // this is an improper command.
                //

                // If this is anything other than the FIRST cmd in the list,
                // then this is an error.

                cmd->input_file_name = strdup(strtok(NULL, SPACE_DELIM));
                cmd->input_src = REDIRECT_FILE;
            }
            else if (strcmp(arg, REDIR_OUT) == 0) {
                // redirect stdout
                //
                // If the output_dest is something other than REDIRECT_NONE, then
                // this is an improper command.
                //

                // If this is anything other than the LAST cmd in the list,
                // then this is an error.

                cmd->output_file_name = strdup(strtok(NULL, SPACE_DELIM));
                cmd->output_dest = REDIRECT_FILE;
            }
            else {
                // add next param
                param_t *param = (param_t *) calloc(1, sizeof(param_t));
                param_t *cparam = cmd->param_list;

                cmd->param_count++;
                // Put something in here to strip out the single quotes if
                // they are the first/last characters in arg.
                if (arg[0] == '\'') {
                    arg++;
                }
                if (arg[strlen(arg) - 1] == '\'') {
                    arg[strlen(arg) - 1] = '\0';
                }
                param->param = strdup(arg);
                if (NULL == cparam) {
                    cmd->param_list = param;
                }
                else {
                    // I should put a tail pointer on this.
                    while (cparam->next != NULL) {
                        cparam = cparam->next;
                    }
                    cparam->next = param;
                }
            }
        }
        // This could overwite some bogus file redirection.
        if (cmd->list_location > 0) {
            cmd->input_src = REDIRECT_PIPE;
        }
        if (cmd->list_location < (cmd_list->count - 1)) {
            cmd->output_dest = REDIRECT_PIPE;
        }

        // No need free with alloca memory.
        //free(raw);
        cmd = cmd->next;
    }

    if (isVerbose > 0) {
        print_list(cmd_list);
    }
}
