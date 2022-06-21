#ifndef GAME_H
#define GAME_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include <uthash.h>

#include <stdbool.h>

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512

extern int x;
extern int y;

extern bool shouldrun;

extern ALLEGRO_BITMAP* cursorbmps[4];
extern ALLEGRO_MOUSE_CURSOR* leftcursor = NULL;
extern ALLEGRO_MOUSE_CURSOR* rightcursor = NULL;
extern ALLEGRO_MOUSE_CURSOR* upcursor = NULL;
extern ALLEGRO_MOUSE_CURSOR* downcursor = NULL;

typedef struct room_t Room;
typedef struct transbox_t TransitionBox;

struct transbox_t{ //heehoo the box is trans
    char* name;
    int x1,y1,x2,y2;
    Room* room;
    ALLEGRO_MOUSE_CURSOR* cursor;
    UT_hash_handle hh;
};

typedef struct editor_room_t{
    char* imagepath;
    char* audiopath;
}Ed_room;

struct room_t
{
    char* name;
    ALLEGRO_BITMAP* image;
    ALLEGRO_AUDIO_STREAM* music;
    TransitionBox* transitions;

    UT_hash_handle hh;
    Ed_room* editor_room;
};

extern Room* current_room;

extern Room* rooms;

void init_cursors(ALLEGRO_DISPLAY* disp);

void must_init(bool test, const char *description);

Room* create_room(const char* name, const char* imagepath, const char* musicpath);

TransitionBox* create_transition(const char* name, int x1, int y1, int x2, int y2, ALLEGRO_MOUSE_CURSOR* cursor, Room* room);

void add_transition(TransitionBox* box,Room* room);

void destroy_transition(Room* room,TransitionBox* transition);

void destroy_room(Room* room);

void destroy_rooms();

TransitionBox* check_transition(int x,int y, Room* room);

void transition(Room* oldRoom,Room* newRoom); //hee hee hee hoo hee hee hoo hee hoo hee hee hee hoo

#endif