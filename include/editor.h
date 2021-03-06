#ifndef EDITOR_H
#define EDITOR_H

//idea: use terminal and graphics
/*terminal commands: listrooms, listtransitions, listcmds, newroom <room name>, newtransition <transition name>, 
setroom <room name>, setimage <path from /images>, setaudio <path from /audio>, delroom <name of room>, 
deltransition <transition name>, help <command name>*/

#include <stdio.h>
#include <pthread.h>
#include "game.h"

extern pthread_mutex_t running_lock;
extern pthread_mutex_t rooms_lock;
extern pthread_mutex_t current_room_lock;
extern pthread_mutex_t current_transition_lock;

extern uint8_t points_set;
extern TransitionBox* current_transition;

typedef bool(*CmdFuncPtr)(int argcount,char** args);
#define cmdfunc(name) bool name(int argcount,char** args)

#define TRANSITIONBOX_COLOR al_map_rgb(255,0,255)

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


cmdfunc(save);
cmdfunc(load);
cmdfunc(listrooms);
cmdfunc(listtransitions);
cmdfunc(listcmds);
cmdfunc(newroom);
cmdfunc(newtransition);
cmdfunc(setroom);
cmdfunc(getroom);
cmdfunc(setname);
cmdfunc(setimage);
cmdfunc(setaudio);
cmdfunc(delroom);
cmdfunc(deltransition);
cmdfunc(help);
cmdfunc(quit);

#endif