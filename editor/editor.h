#ifndef EDITOR_H
#define EDITOR_H

//idea: use terminal and graphics
/*terminal commands: listrooms, listtransitions, listcmds, newroom <room name>, newtransition <transition name>, 
setroom <room name>, setimage <path from /images>, setaudio <path from /audio>, delroom <name of room>, 
deltransition <transition name>, help <command name>*/

#include <stdio.h>
#include "game.h"

typedef bool(*CmdFuncPtr)(int argcount,char** args);
#define cmdfunc(name) bool name(int argcount,char** args)

enum EDITOR_MODE {MODE_VIEWING, MODE_ED_TRANSITION};

extern enum EDITOR_MODE editor_mode;

typedef struct command_t{
    char* command;
    char* usage;
    char* desc;
    CmdFuncPtr func;
    UT_hash_handle hh;
} Command;

extern Command* commands;

void create_command_hash();

void parse_input(char* input);

cmdfunc(listrooms);
cmdfunc(listtransitions);
cmdfunc(listcmds);
cmdfunc(newroom);
cmdfunc(newtransition);
cmdfunc(setroom);
cmdfunc(setname);
cmdfunc(setimage);
cmdfunc(setaudio);
cmdfunc(delroom);
cmdfunc(deltransition);
cmdfunc(help);
cmdfunc(quit);

#endif