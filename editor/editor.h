#ifndef EDITOR_H
#define EDITOR_H

//idea: use terminal and graphics
/*terminal commands: listrooms, listtransitions, listcmds, newroom <room name>, newtransition <transition name>, 
setroom <room name>, setimage <path from /images>, setaudio <path from /audio>, delroom <name of room>, 
deltransition <transition name>, help <command name>*/

#include <stdio.h>
#include "../game.h"

typedef bool(*CmdFuncPtr)(char*);
#define cmdfunc(name) bool name(char* arg)

typedef struct command_t{
    char* command;
    char* usage;
    char* desc;
    CmdFuncPtr func;
    UT_hash_handle hh;
} Command;

extern Command* commands;

void create_command_hash();

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

#endif