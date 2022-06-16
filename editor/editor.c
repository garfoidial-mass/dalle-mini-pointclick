#include "editor.h"
///BRUH,,,ARGS AINT NULL, BUT IT ENDS UP NULL!!!!!
Command cmdarr[] = {
    {"listrooms","listrooms","lists existing rooms",&listrooms},
    {"listtransitions","listtransitions","lists transitions in current room",&listtransitions},
    {"listcmds","listcmds","list console commands",&listcmds},
    {"newroom","newroom <room name> <image path> <audio path>","creates a room with the name <room name>, image at <image path>, and audio file at <audio path>",&newroom},
    {"newtransition","newtransition <transition name> <room name>","creates a transition with the name <transition name> which transitions to the room with the name <room name> in the currently edited room",&newtransition},
    {"setroom","setroom <room name>","starts editing the room with name <room name>",&setroom},
    {"setname","setname <room name>","changes the currently edited room's name to <room name>",&setname},
    {"setimage","setimage <image path>","sets the room's image to the image at <image path>. <image path> is relative to the game executable's folder, not the editor folder",&setimage},
    {"setaudio","setaudio <audio path>","sets the room's audio track to the audio file at <audio path>",&setaudio},
    {"delroom","delroom <room name>","deletes the room with name <room name>",&delroom},
    {"deltransition","deltransition <transition name>","deletes the transition with name <transition name> from the currently edited room.",&deltransition},
    {"help","help <command name>","displays info about command <command name>",&help},
    {"quit","quit","quits the program",&quit}
};

Command* commands = NULL;

enum EDITOR_MODE editor_mode = MODE_VIEWING;

void create_command_hash()
{
    int numcmd = sizeof(cmdarr)/sizeof(Command);


    for(int i = 0; i < numcmd; i++)
    {
        HASH_ADD_STR(commands,command,&(cmdarr[i]));
    }
}

void parse_input(char* input)
{
    char delim[] = " ";
    int argcount = 1;
    char** args = malloc(sizeof(char*));
    args[0] = strtok(input,delim);

    while(args[argcount-1] != NULL)
    {
        argcount++;
        args = realloc(args, argcount*sizeof(char*));
        args[argcount-1] = strtok(NULL,delim);
    }
    argcount-=1;
    printf("%i\n",argcount);
    for(int i = 0; i < argcount; i++)
    {
        printf("%s\n",args[i]);
    }
    Command* cmd = NULL;
    HASH_FIND_STR(commands,args[0],cmd);
    if(cmd == NULL)
    {
        printf("could not find command \"%s\"\n",args[0]);
        return;
    }

    if((cmd->func)(argcount,args) == false)
    {
        printf("%s could not execute correctly. Make sure the arguments and paths are correct\n",args[0]);
    }

    free(args);
}

cmdfunc(listrooms)
{
    for(Room* room = rooms; room != NULL; room = room->hh.next)
    {
        printf(room->name);
    }
    return true;
}

cmdfunc(listtransitions)
{
    for(TransitionBox* transition = current_room->transitions; transition != NULL; transition = transition->hh.next)
    {
        printf(transition->name);
    }

    return true;
}

cmdfunc(listcmds)
{
    for(Command* command = commands; command != NULL; command = command->hh.next)
    {
        printf("%s\n",command->command);
    }
    return true;
}
cmdfunc(newroom)
{
    char* roomname = "";
    char* imagepath = "";
    char* audiopath = "";
    switch(argcount)
    {
        default:
            audiopath = args[3];
        case 3:
            imagepath = args[2];
        case 2:
            roomname = args[1];
            break;
        case 1:
            return false;
        break;
    }
    Room* room = create_room(roomname,imagepath,audiopath);
    room->editor_room = malloc(sizeof(Ed_room));
    return true;
}
cmdfunc(newtransition)
{
    if(argcount < 3)
    {
        return false;
    }
    Room* room;
    HASH_FIND_STR(rooms,args[2],room);
    if(room != NULL)
    {
        TransitionBox* box = create_transition(args[1],0,0,0,0,NULL,room);
        add_transition(box,current_room);
        editor_mode = MODE_ED_TRANSITION;
        return true;
    }
    return false;
}
cmdfunc(setroom)
{
    if(argcount < 2)
    {
        return false;
    }
    Room* room = NULL;
    for(Room *roomi = rooms; roomi != NULL; roomi = roomi->hh.next)
    {
        printf("%s\n",roomi->name);
    }
    HASH_FIND_STR(rooms,args[1], room);
    if(room != NULL)
    {
        current_room = room;
        return true;
    }
    printf("could not find room\n");
    return false;
}
cmdfunc(getroom)
{
    if(current_room != NULL)
    {
        printf("%s\n",current_room->name);
        return true;
    }
    return false;
}
cmdfunc(setname)
{
    if(argcount < 2)
    {
        return false;
    }
    HASH_DEL(rooms,current_room);
    current_room->name = args[1];
    HASH_ADD_STR(rooms,name,current_room);
    return true;
}
cmdfunc(setimage)
{
    if(argcount < 2)
    {
        return false;
    }
    ALLEGRO_BITMAP* tmpimg = al_load_bitmap(args[1]);
    if(tmpimg != NULL)
    {
        al_destroy_bitmap(current_room->image);
        current_room->image = tmpimg;
        current_room->editor_room->imagepath = args[1];
        return true;
    }
    al_destroy_bitmap(tmpimg);
    return false;
}
cmdfunc(setaudio)
{
    if(argcount < 2)
    {
        return false;
    }
    ALLEGRO_AUDIO_STREAM* audio = al_load_audio_stream(args[1],2,2048);
    if(audio != NULL)
    {
        al_destroy_audio_stream(current_room->music);
        current_room->music = audio;
        current_room->editor_room->audiopath = args[1];
        al_set_audio_stream_playmode(current_room->music, ALLEGRO_PLAYMODE_LOOP);
        al_attach_audio_stream_to_mixer(current_room->music,al_get_default_mixer());
        return true;
    }
    al_destroy_audio_stream(audio);
    return false;
}
cmdfunc(delroom)
{
    if(argcount < 2)
    {
        return false;
    }
    Room* room = NULL;
    HASH_FIND_STR(rooms,args[1],room);
    if(room != NULL)
    {
        destroy_room(room);
        return true;
    }

    return false;
}
cmdfunc(deltransition)
{
    if(argcount < 2)
    {
        return false;
    }
    TransitionBox* transition = NULL;
    HASH_FIND_STR(current_room->transitions,args[1],transition);
    if(transition != NULL)
    {
        destroy_transition(current_room,transition);
    }
}
cmdfunc(help)
{
    if(argcount < 2)
    {
        return false;
    }
    Command* cmd = NULL;
    HASH_FIND_STR(commands,args[1],cmd);

    if(cmd != NULL)
    {
        printf("%s\n%s\n",cmd->usage,cmd->desc);
        return true;
    }
    return false;
}

cmdfunc(quit)
{
    shouldrun = false;
    return true;
}