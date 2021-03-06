#include "game.h"

bool shouldrun = true;

Room* current_room = NULL;

Room* rooms = NULL;

int x = 0;
int y = 0;

ALLEGRO_BITMAP* cursorbmps[4];
ALLEGRO_MOUSE_CURSOR* leftcursor = NULL;
ALLEGRO_MOUSE_CURSOR* rightcursor = NULL;
ALLEGRO_MOUSE_CURSOR* upcursor = NULL;
ALLEGRO_MOUSE_CURSOR* downcursor = NULL;

void init_cursors(ALLEGRO_DISPLAY* disp)
{
    cursorbmps[0] = al_load_bitmap("images/cursors/forward.png");
    for(int i = 1; i < 4; i++)
    {
        cursorbmps[i] = al_create_bitmap(32,32);
        al_set_target_bitmap(cursorbmps[i]);
        al_draw_rotated_bitmap(cursorbmps[0],16,16,16,16,1.5708*i,0);
    }
    al_set_target_backbuffer(disp);

    leftcursor = al_create_mouse_cursor(cursorbmps[3],0,16);
    must_init(leftcursor,"leftcursor");
    rightcursor = al_create_mouse_cursor(cursorbmps[1],32,16);
    must_init(rightcursor,"rightcursor");
    upcursor = al_create_mouse_cursor(cursorbmps[0],16,0);
    must_init(upcursor,"upcursor");
    downcursor = al_create_mouse_cursor(cursorbmps[2],16,32);
    must_init(downcursor,"downcursor");
}

bool load_game(const char* path)
{
    ALLEGRO_CONFIG* conf = al_load_config_file(path);
    if(conf == NULL) return false;

    ALLEGRO_CONFIG_SECTION* iterator;

    for(char* section = (char*)al_get_first_config_section(conf,&iterator); section != NULL; section = (char*)al_get_next_config_section(&iterator))
    {
        if(al_get_config_value(conf,section,"room")==NULL)
        {
            char* imgpath = (char*)al_get_config_value(conf,section,"imagepath");
            char* audiopath = (char*)al_get_config_value(conf,section,"audiopath");

            Room* room = create_room(section,imgpath,audiopath);
            room->editor_room = malloc(sizeof(Ed_room));
            room->editor_room->imagepath = malloc(strlen(imgpath)+1);
            strcpy(room->editor_room->imagepath,imgpath);
            room->editor_room->audiopath = malloc(strlen(audiopath)+1);
            strcpy(room->editor_room->audiopath,audiopath);
        }
    }

    for(char* section = (char*)al_get_first_config_section(conf,&iterator); section != NULL; section = (char*)al_get_next_config_section(&iterator))
    {
        if(al_get_config_value(conf,section,"room")==NULL)
        {
            HASH_FIND_STR(rooms,section,current_room);
        }
        else
        {
            int x1 = strtol(al_get_config_value(conf,section,"x1"),NULL,10);
            int y1 = strtol(al_get_config_value(conf,section,"y1"),NULL,10);
            int x2 = strtol(al_get_config_value(conf,section,"x2"),NULL,10);
            int y2 = strtol(al_get_config_value(conf,section,"y2"),NULL,10);

            Room* room;
            HASH_FIND_STR(rooms,al_get_config_value(conf,section,"room"),room);

            ALLEGRO_MOUSE_CURSOR* cursor;
            switch(al_get_config_value(conf,section,"cursor")[0])
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
            TransitionBox* transition = create_transition(section,x1,y1,x2,y2,cursor,room);
            add_transition(transition,current_room);
        }
    }
    current_room = rooms;

    al_destroy_config(conf);
    return true;
}

void must_init(bool test, const char *description)
{
    if (test) return;

    exit(1);
}

Room* create_room(const char* name, const char* imagepath, const char* musicpath)
{
    Room* rmcheck = NULL;
    HASH_FIND_STR(rooms,name,rmcheck);
    if(rmcheck == NULL)
    {
        Room *room = (Room*)malloc(sizeof(Room));
        *room = (Room){malloc(strlen(name)+1),al_load_bitmap(imagepath),al_load_audio_stream(musicpath,2,2048),0,NULL,NULL};
        strcpy(room->name,name);
        if(room->music != NULL)
        {
            al_set_audio_stream_playmode(room->music, ALLEGRO_PLAYMODE_LOOP);
            al_attach_audio_stream_to_mixer(room->music,al_get_default_mixer());
            al_set_audio_stream_playing(room->music,false);
        }
        HASH_ADD_STR(rooms,name,room);
        return room;
    }
    return NULL;
}
TransitionBox* create_transition(const char* name,int x1, int y1, int x2, int y2, ALLEGRO_MOUSE_CURSOR* cursor, Room* room)
{
    TransitionBox* box = malloc(sizeof(TransitionBox));
    *box = (TransitionBox){malloc(strlen(name)+1),x1,y1,x2,y2,room,cursor};
    strcpy(box->name,name);
    return box;
}
void add_transition(TransitionBox* box,Room* room)
{
    TransitionBox* check = NULL;
    HASH_FIND_STR(room->transitions,box->name,check);
    if(check==NULL)
    {
        HASH_ADD_STR(room->transitions,name,box);
    }
}

void destroy_transition(Room* room,TransitionBox* transition)
{
    HASH_DEL(room->transitions,transition);
    free(transition->name);
    free(transition);
}

void destroy_room(Room* room)
{
    if(room != NULL)
    {
        HASH_DEL(rooms,room);
        al_destroy_audio_stream(room->music);
        al_destroy_bitmap(room->image);
        TransitionBox* transition, *tmptrans;
        HASH_ITER(hh,room->transitions,transition,tmptrans)
        {   
            destroy_transition(room,transition);
        }
        free(room->name);
        free(room->editor_room->imagepath);
        free(room->editor_room->audiopath);
        free(room->editor_room);
        free(room);
    }
}

void destroy_rooms()
{
    Room *room, *tmproom;
    HASH_ITER(hh,rooms,room,tmproom)
    {
        destroy_room(room);
    }
}

TransitionBox* check_transition(int x,int y, Room* room)
{
    for(TransitionBox* box = room->transitions; box != NULL; box = box->hh.next)
    {
        int *x_vals = (box->x1 < box->x2) ? (int[2]){box->x1,box->x2} : (int[2]){box->x2,box->x1};
        int *y_vals = (box->y1 < box->y2) ? (int[2]){box->y1,box->y2} : (int[2]){box->y2,box->y1};

        bool is_x = (x>=x_vals[0] && x <= x_vals[1]);
        bool is_y = (y>=y_vals[0] && y <= y_vals[1]);

        if (is_x && is_y)
        {
            return box;
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