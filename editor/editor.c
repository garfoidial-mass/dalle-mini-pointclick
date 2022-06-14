#include "editor.h"

Command cmdarr[] = {
    {"listrooms","listrooms","lists existing rooms",&listrooms},
    {"listtransitions","listtransitions","lists transitions in current room",&listtransitions},
    {"listcmds","listcmds","list console commands",&listcmds},
    {"newroom","newroom <room name>","creates a room with the name <room name>",&newroom},
    {"newtransition","newtransition <transition name>","creates a transition with the name <transition name> in the currently edited room",&newtransition},
    {"setroom","setroom <room name>","starts editing the room with name <room name>",&setroom},
    {"setname","setname <room name>","changes the currently edited room's name to <room name>",&setname},
    {"setimage","setimage <image path>","sets the room's image to the image at <image path>. <image path> is relative to the game executable's folder, not the editor folder",&setimage},
    {"setaudio","setaudio <audio path>","sets the room's audio track to the audio file at <audio path>",&setaudio},
    {"delroom","delroom <room name>","deletes the room with name <room name>",&delroom},
    {"deltransition","deltransition <transition name>","deletes the transition with name <transition name> from the currently edited room.",&deltransition},
    {"help","help <command name>","displays info about command <command name>",&help}
};

Ed_room* editor_rooms = NULL;
Command* commands = NULL;

void create_command_hash()
{
    int numcmd = sizeof(commands)/sizeof(Command);

    for(int i = 0; i < numcmd; i++)
    {
        HASH_ADD_STR(commands,command,&cmdarr[i]);
    }
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
    Room* room = create_room(arg,"","");
    room->editor_room = malloc(sizeof(Ed_room));
    HASH_ADD_STR(rooms,name,room);
    return true;
}
cmdfunc(newtransition);
cmdfunc(setroom)
{
    Room* room;
    HASH_FIND_STR(rooms,arg, room);
    current_room = room;
    return true;
}
cmdfunc(setname)
{
    HASH_DEL(rooms,current_room);
    current_room->name = arg;
    HASH_ADD_STR(rooms,name,current_room);
    return true;
}
cmdfunc(setimage)
{
    ALLEGRO_BITMAP* tmpimg = al_load_bitmap(arg);
    if(tmpimg != NULL)
    {
        al_destroy_bitmap(current_room->image);
        current_room->image = tmpimg;
        current_room->editor_room->imagepath = arg;
        return true;
    }
    al_destroy_bitmap(tmpimg);
    return false;
}
cmdfunc(setaudio)
{
    ALLEGRO_AUDIO_STREAM* audio = al_load_audio_stream(arg,2,2048);
    if(audio != NULL)
    {
        al_destroy_audio_stream(current_room->music);
        current_room->music = audio;
        current_room->editor_room->audiopath = arg;
        al_set_audio_stream_playmode(current_room->music, ALLEGRO_PLAYMODE_LOOP);
        al_attach_audio_stream_to_mixer(current_room->music,al_get_default_mixer());
        return true;
    }
    al_destroy_audio_stream(audio);
    return false;
}
cmdfunc(delroom)
{
    Room* room = NULL;
    HASH_FIND_STR(rooms,arg,room);
    if(room != NULL)
    {
        destroy_room(room);
        return true;
    }

    return false;
}
cmdfunc(deltransition)
{
    TransitionBox* transition = NULL;
    HASH_FIND_STR(current_room->transitions,arg,transition);
    if(transition != NULL)
    {
        destroy_transition(current_room,transition);
    }
}
cmdfunc(help)
{
    Command* cmd = NULL;
    HASH_FIND_STR(commands,arg,cmd);

    if(cmd != NULL)
    {
        printf("%s\n%s\n",cmd->usage,cmd->desc);
        return true;
    }
    return false;
}