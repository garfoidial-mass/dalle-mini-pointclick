#include "editor.h"

pthread_mutex_t running_lock;
pthread_mutex_t rooms_lock;
pthread_mutex_t current_room_lock;
pthread_mutex_t current_transition_lock;

TransitionBox* current_transition;

Command cmdarr[] = {
    {"save","save <filename>","saves rooms to file specified by filename",&save},
    {"load","load <filename>","loads rooms from file specified by filename",&load},
    {"listrooms","listrooms","lists existing rooms",&listrooms},
    {"listtransitions","listtransitions","lists transitions in current room",&listtransitions},
    {"listcmds","listcmds","list console commands",&listcmds},
    {"newroom","newroom <room name> <image path> <audio path>","creates a room with the name <room name>, image at <image path>, and audio file at <audio path>",&newroom},
    {"newtransition","newtransition <transition name> <room name> <cursor name>","creates a transition with the name <transition name> which transitions to the room with the name <room name> in the currently edited room. the cursor can be either 'up', 'down','left',or 'right'.",&newtransition},
    {"setroom","setroom <room name>","starts editing the room with name <room name>",&setroom},
    {"getroom","getroom <room name>","displays the name of the currently edited room",&getroom},
    {"setname","setname <room name>","changes the currently edited room's name to <room name>",&setname},
    {"setimage","setimage <image path>","sets the room's image to the image at <image path>. <image path> is relative to the game executable's folder, not the editor folder",&setimage},
    {"setaudio","setaudio <audio path>","sets the room's audio track to the audio file at <audio path>",&setaudio},
    {"delroom","delroom <room name>","deletes the room with name <room name>",&delroom},
    {"deltransition","deltransition <transition name>","deletes the transition with name <transition name> from the currently edited room.",&deltransition},
    {"help","help <command name>","displays info about command <command name>",&help},
    {"quit","quit","quits the program",&quit}
};

uint8_t points_set = 0; // for checking which points have been set in a transition (should never be greater than 2)

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
    Command* cmd = NULL;
    if(args[0] == NULL){ 
        free(args);
        return;
    }
    HASH_FIND_STR(commands,args[0],cmd);
    if(cmd == NULL)
    {
        printf("could not find command \"%s\"\n",args[0]);
        free(args);
        return;
    }

    if((cmd->func)(argcount,args) == false)
    {
        printf("%s could not execute correctly. Make sure the arguments and paths are correct\n",args[0]);
    }

    free(args);
    return;
}

cmdfunc(save)
{
    if(argcount < 2)
    {
        return false;
    }

    ALLEGRO_CONFIG* conf = al_create_config();
    char num[512];
    for(Room* room = rooms; room != NULL; room = room->hh.next)
    {
        al_add_config_section(conf,room->name);
        al_set_config_value(conf,room->name,"isroom","true");
        al_set_config_value(conf,room->name,"imagepath",room->editor_room->imagepath);
        al_set_config_value(conf,room->name,"audiopath",room->editor_room->audiopath);
        for(TransitionBox* transition = room->transitions; transition != NULL; transition = transition->hh.next)
        {
            al_add_config_section(conf,transition->name);
            al_set_config_value(conf,transition->name,"isroom","false");
            al_set_config_value(conf,transition->name,"x1",itoa(transition->x1,num,10));
            al_set_config_value(conf,transition->name,"y1",itoa(transition->y1,num,10));
            al_set_config_value(conf,transition->name,"x2",itoa(transition->x2,num,10));
            al_set_config_value(conf,transition->name,"y2",itoa(transition->y2,num,10));
            al_set_config_value(conf,transition->name,"room",transition->room->name);
            //add cursor here
        }
        
    }
    al_destroy_config(conf);
    return true;
}
cmdfunc(load)
{
    if(argcount < 2)
    {
        return false;
    }
    return true;
}

cmdfunc(listrooms)
{
    pthread_mutex_lock(&rooms_lock);
    for(Room* room = rooms; room != NULL; room = room->hh.next)
    {
        printf(room->name);
    }
    pthread_mutex_unlock(&rooms_lock);
    return true;
}

cmdfunc(listtransitions)
{
    pthread_mutex_lock(&current_room_lock);
    for(TransitionBox* transition = current_room->transitions; transition != NULL; transition = transition->hh.next)
    {
        printf("%s\n",transition->name);
    }
    pthread_mutex_unlock(&current_room_lock);
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
    pthread_mutex_lock(&rooms_lock);
    Room* room = create_room(roomname,imagepath,audiopath);
    room->editor_room = malloc(sizeof(Ed_room));
    pthread_mutex_unlock(&rooms_lock);
    return true;
}
cmdfunc(newtransition)
{
    if(argcount < 4)
    {
        return false;
    }
    Room* room;
    pthread_mutex_lock(&rooms_lock);

    ALLEGRO_MOUSE_CURSOR* cursor;

    switch(args[3][0])
    {
        case 'u':
            cursor = upcursor;
        break;
        case 'd':
            cursor = downcursor;
        break;
        case 'l':
            cursor = leftcursor;
        break;
        case 'r':
            cursor = rightcursor;
        break;
    }

    HASH_FIND_STR(rooms,args[2],room);
    if(room != NULL && room != current_room)
    {
        TransitionBox* box = create_transition(args[1],0,0,0,0,cursor,room);
        add_transition(box,current_room);
        current_transition = box;
        editor_mode = MODE_ED_TRANSITION;
        pthread_mutex_unlock(&rooms_lock);
        return true;
    }
    pthread_mutex_unlock(&rooms_lock);
    return false;
}
cmdfunc(setroom)
{
    if(argcount < 2)
    {
        return false;
    }
    Room* room = NULL;
    pthread_mutex_lock(&rooms_lock);
    HASH_FIND_STR(rooms,args[1], room);
    if(room != NULL)
    {
        current_room = room;
        pthread_mutex_unlock(&rooms_lock);
        return true;
    }
    pthread_mutex_unlock(&rooms_lock);
    return false;
}
cmdfunc(getroom)
{
    pthread_mutex_lock(&current_room_lock);
    if(current_room != NULL)
    {
        printf("%s\n",current_room->name);
        pthread_mutex_unlock(&current_room_lock);
        return true;
    }
    pthread_mutex_unlock(&current_room_lock);
    return false;
}
cmdfunc(setname)
{
    if(argcount < 2)
    {
        return false;
    }
    pthread_mutex_lock(&rooms_lock);
    HASH_DEL(rooms,current_room);
    current_room->name = args[1];
    HASH_ADD_STR(rooms,name,current_room);
    pthread_mutex_unlock(&rooms_lock);
    return true;
}
cmdfunc(setimage)
{
    if(argcount < 2)
    {
        return false;
    }
    ALLEGRO_BITMAP* tmpimg = al_load_bitmap(args[1]);
    pthread_mutex_lock(&current_room_lock);
    if(tmpimg != NULL)
    {
        al_destroy_bitmap(current_room->image);
        current_room->image = tmpimg;
        current_room->editor_room->imagepath = args[1];
        pthread_mutex_unlock(&current_room_lock);
        return true;
    }
    pthread_mutex_unlock(&current_room_lock);
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
        pthread_mutex_lock(&current_room_lock);
        al_destroy_audio_stream(current_room->music);
        current_room->music = audio;
        current_room->editor_room->audiopath = args[1];
        al_set_audio_stream_playmode(current_room->music, ALLEGRO_PLAYMODE_LOOP);
        al_attach_audio_stream_to_mixer(current_room->music,al_get_default_mixer());
        pthread_mutex_unlock(&current_room_lock);
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
    pthread_mutex_lock(&rooms_lock);
    HASH_FIND_STR(rooms,args[1],room);
    if(room != NULL)
    {
        destroy_room(room);
        pthread_mutex_unlock(&rooms_lock);
        return true;
    }
    pthread_mutex_unlock(&rooms_lock);

    return false;
}
cmdfunc(deltransition)
{
    if(argcount < 2)
    {
        return false;
    }
    TransitionBox* transition = NULL;
    pthread_mutex_lock(&current_room_lock);
    HASH_FIND_STR(current_room->transitions,args[1],transition);
    if(transition != NULL)
    {
        destroy_transition(current_room,transition);
        pthread_mutex_unlock(&current_room_lock);
        return true;
    }
    pthread_mutex_unlock(&current_room_lock);
    return false;
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
    pthread_mutex_lock(&running_lock);
    shouldrun = false;
    pthread_mutex_unlock(&running_lock);
    return true;
}