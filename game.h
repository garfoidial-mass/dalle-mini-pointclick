#ifndef GAME_H
#define GAME_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdbool.h>

typedef struct room_t Room;
typedef struct transbox_t TransitionBox;

struct transbox_t{ //heehoo the box is trans
    int x1,y1,x2,y2;
    Room* room;
    ALLEGRO_MOUSE_CURSOR* cursor;
};

struct room_t
{
    char* name;
    ALLEGRO_BITMAP* image;
    ALLEGRO_AUDIO_STREAM* music;
    
    int transition_count;
    TransitionBox* transitions;
};

Room* current_room = NULL;

int room_count = 0;
Room** rooms = NULL;

void must_init(bool test, const char *description)
{
    if (test) return;

    exit(1);
}

Room* create_room(const char* name, const char* imagepath, const char* musicpath)
{
    room_count++;
    if (rooms == NULL)
    {
        rooms = malloc(sizeof(Room*));
    }
    else
    {
        rooms = realloc(rooms,room_count* sizeof(Room*));
    }
    Room *room = malloc(sizeof(Room));
    *room = (Room){malloc(strlen(name)+1),al_load_bitmap(imagepath),al_load_audio_stream(musicpath,2,2048),0,NULL};
    strcpy(room->name,name);
    must_init(room->image,"room image");
    if(room->music != NULL)
    {
        al_set_audio_stream_playmode(room->music, ALLEGRO_PLAYMODE_LOOP);
        al_attach_audio_stream_to_mixer(room->music,al_get_default_mixer());
        al_set_audio_stream_playing(room->music,false);
    }
    rooms[room_count-1] = room;
    return room;
}
TransitionBox create_transition(int x1, int y1, int x2, int y2, ALLEGRO_MOUSE_CURSOR* cursor, Room* room)
{
    return (TransitionBox){x1,y1,x2,y2,room,cursor};
}
void add_transition(TransitionBox box,Room* room)
{
    room->transition_count++;
    if(room->transitions == NULL)
    {
        room->transitions = malloc(sizeof(TransitionBox));
    }
    else
    {
        room->transitions = realloc(room->transitions,room->transition_count*sizeof(TransitionBox));
    }
    room->transitions[room->transition_count-1] = box;
}

void destroy_rooms()
{
    for (int i = 0; i < room_count; i++)
    {
        al_destroy_audio_stream(rooms[i]->music);
        al_destroy_bitmap(rooms[i]->image);
        free(rooms[i]->transitions);
        free(rooms[i]->name);
        free(rooms[i]);
    }
    free(rooms);
}

TransitionBox* check_transition(int x,int y, Room* room)
{
    for (int i = 0; i < room->transition_count; i++)
    {
        TransitionBox box = room->transitions[i];
        int *x_vals = (box.x1 < box.x2) ? (int[2]){box.x1,box.x2} : (int[2]){box.x2,box.x1};
        int *y_vals = (box.y1 < box.y2) ? (int[2]){box.y1,box.y2} : (int[2]){box.y2,box.y1};

        bool is_x = (x>=x_vals[0] && x <= x_vals[1]);
        bool is_y = (y>=y_vals[0] && y <= y_vals[1]);

        if (is_x && is_y)
        {
            return &(room->transitions[i]);
        }
    }
    return NULL;
}

void transition(Room* oldRoom,Room* newRoom) //hee hee hee hoo hee hee hoo hee hoo hee hee hee hoo
{
    current_room = newRoom;
    if(oldRoom->music != NULL)
    {
        al_set_audio_stream_playing(oldRoom->music,false);
    }
    if(current_room->music != NULL)
    {
        al_set_audio_stream_playing(current_room->music,true);
    }
}

#endif