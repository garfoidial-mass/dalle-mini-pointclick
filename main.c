#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdbool.h>
#include <stdio.h>

#include "game.h"

#define KEY_SEEN 1
#define KEY_RELEASED 2

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512

unsigned char key[ALLEGRO_KEY_MAX];

int x = 0;
int y = 0;

int main(int argc, char** argv)
{
    memset(key,0,sizeof(key));

    bool shouldrun = true;

    //begin init allegro systems

    must_init(al_init(),"allegro");

    must_init(al_install_keyboard(),"keyboard");

    must_init(al_install_mouse(),"mouse");

    must_init(al_install_audio(),"audio");

    must_init(al_init_image_addon(),"image addon");

    must_init(al_init_acodec_addon(),"audio codecs");
    must_init(al_reserve_samples(16),"samples");
    
    //end init allegro systems


    //begin create event sources
    ALLEGRO_TIMER* timer = al_create_timer(1.0/30.0);
    must_init(timer,"timer");
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue,"queue");
    ALLEGRO_DISPLAY* disp = al_create_display(SCREEN_WIDTH,SCREEN_HEIGHT);
    must_init(disp,"display");
    //end create event sources

    //begin init cursors
    ALLEGRO_BITMAP* cursorbmps[4];
    cursorbmps[0] = al_load_bitmap("images/cursors/forward.png");
    for(int i = 1; i < 4; i++)
    {
        cursorbmps[i] = al_create_bitmap(32,32);
        al_set_target_bitmap(cursorbmps[i]);
        al_draw_rotated_bitmap(cursorbmps[0],16,16,16,16,1.5708*i,0);
    }
    al_set_target_backbuffer(disp);

    ALLEGRO_MOUSE_CURSOR* leftcursor = al_create_mouse_cursor(cursorbmps[3],0,16);
    must_init(leftcursor,"leftcursor");
    ALLEGRO_MOUSE_CURSOR* rightcursor = al_create_mouse_cursor(cursorbmps[1],32,16);
    must_init(rightcursor,"rightcursor");
    ALLEGRO_MOUSE_CURSOR* upcursor = al_create_mouse_cursor(cursorbmps[0],16,0);
    must_init(upcursor,"upcursor");
    ALLEGRO_MOUSE_CURSOR* downcursor = al_create_mouse_cursor(cursorbmps[2],16,32);
    must_init(downcursor,"downcursor");
    
    //end init cursors


    //start room and transition box definitions

    Room* hallway1 = create_room("hallway 1","images/hallways/hallway6.png","");
    must_init(hallway1,"hallway1");
    Room* hallway2 = create_room("hallway 2","images/hallways/hallway3.png","");
    must_init(hallway2,"hallway2");

    TransitionBox* hallway1to2 = create_transition("",208,200,270,310,upcursor,hallway2);
    add_transition(hallway1to2,hallway1);
    TransitionBox* hallway2to1 = create_transition("",0,412,512,512,downcursor,hallway1);
    add_transition(hallway2to1,hallway2);

    Room* bathroom = create_room("bathroom","images/bathrooms/bathroom1.png","");
    TransitionBox* hallway2tobathroom = create_transition("",350,130,375,330,rightcursor,bathroom);
    add_transition(hallway2tobathroom,hallway2);
    TransitionBox* bathroomtohallway2 = create_transition("",0,450,512,512,downcursor,hallway2);
    add_transition(bathroomtohallway2,bathroom);

    Room* toilet = create_room("toilet","images/bathrooms/toilet.png","");
    TransitionBox* bathroomtotoilet = create_transition("",320,387,355,410,downcursor,toilet);
    add_transition(bathroomtotoilet,bathroom);
    TransitionBox* toilettobathroom = create_transition("",0,412,512,512,downcursor,bathroom);
    add_transition(toilettobathroom,toilet);

    Room* hallway_b1 = create_room("toilet hallway 1","images/hallways/hallway7.png","");
    TransitionBox* toilettohallway_b1 = create_transition("",253,145,387,257,downcursor,hallway_b1);
    add_transition(toilettohallway_b1,toilet);
    
    Room* hallway_b2 = create_room("toilet hallway 2", "images/hallways/hallway9.png","");
    TransitionBox* hallway_b1tohallway_b2 = create_transition("",208,200,251,264,upcursor,hallway_b2);
    add_transition(hallway_b1tohallway_b2,hallway_b1);
    TransitionBox* hallway_b2tohallway_b1 = create_transition("",0,412,512,512,downcursor,hallway_b1);
    add_transition(hallway_b2tohallway_b1,hallway_b2);

    Room* office = create_room("office", "images/office.png","");
    TransitionBox* hallway_b2tooffice = create_transition("",227,225,284,285,upcursor,office);
    add_transition(hallway_b2tooffice,hallway_b2);
    TransitionBox* officetohallway_b2 = create_transition("",0,412,512,512,downcursor,hallway_b2);
    add_transition(officetohallway_b2,office);

    Room* credits = create_room("credits", "images/credits.png","");
    TransitionBox* officetocredits = create_transition("",10,75,63,305,leftcursor,credits);
    add_transition(officetocredits,office);

    Room* mineshaft1 = create_room("mineshaft 1", "images/mineshafts/mineshaft1.png","");
    TransitionBox* hallway2tomineshaft1 = create_transition("",191,168,240,253,upcursor,mineshaft1);
    add_transition(hallway2tomineshaft1,hallway2);
    TransitionBox* mineshaft1tohallway2 = create_transition("",0,412,512,512,downcursor,hallway2);
    add_transition(mineshaft1tohallway2,mineshaft1);

    Room* mineshaft2 = create_room("mineshaft 2", "images/mineshafts/mineshaft2.png","");
    TransitionBox* mineshaft1tomineshaft2 = create_transition("",300,70,380,280,upcursor,mineshaft2);
    add_transition(mineshaft1tomineshaft2,mineshaft1);
    TransitionBox* mineshaft2tomineshaft1 = create_transition("",0,412,512,512,downcursor,mineshaft1);
    add_transition(mineshaft2tomineshaft1,mineshaft2);

    Room* mineshaft3 = create_room("mineshaft 3", "images/mineshafts/mineshaft3.png","");
    TransitionBox* mineshaft3tomineshaft2 = create_transition("",0,412,512,512,downcursor,mineshaft2);
    add_transition(mineshaft3tomineshaft2,mineshaft3);
    TransitionBox* mineshaft2tomineshaft3 = create_transition("",160,170,220,270,upcursor,mineshaft3);
    add_transition(mineshaft2tomineshaft3,mineshaft2);

    Room* mineshaft4 = create_room("mineshaft 4", "images/mineshafts/mineshaft4.png","");
    TransitionBox* mineshaft4tomineshaft3 = create_transition("",0,412,512,512,downcursor,mineshaft3);
    add_transition(mineshaft4tomineshaft3,mineshaft4);
    TransitionBox* mineshaft3tomineshaft4 = create_transition("",215,135,325,200,upcursor,mineshaft4);
    add_transition(mineshaft3tomineshaft4,mineshaft3);

    Room* atrium = create_room("atrium", "images/atriums/atrium1.png","");
    TransitionBox* atriumtomineshaft4 = create_transition("",0,412,512,512,downcursor,mineshaft4);
    add_transition(atriumtomineshaft4,atrium);
    TransitionBox* mineshaft4toatrium = create_transition("",215,90,270,185,upcursor,atrium);
    add_transition(mineshaft4toatrium,mineshaft4);

    //end room and transition box definition

    //begin register event sources

    al_register_event_source(queue,al_get_keyboard_event_source());
    al_register_event_source(queue,al_get_mouse_event_source());
    al_register_event_source(queue,al_get_display_event_source(disp));
    al_register_event_source(queue,al_get_timer_event_source(timer));

    //end register event sources

    //ALLEGRO_AUDIO_STREAM* music = al_load_audio_stream("audio/music/sleeper.opus",2,2048);
    //al_set_audio_stream_playmode(music, ALLEGRO_PLAYMODE_LOOP);
    //al_attach_audio_stream_to_mixer(music,al_get_default_mixer());

    bool redraw = true;
    ALLEGRO_EVENT event;

    al_grab_mouse(disp);

    al_start_timer(timer);

    current_room = hallway1;

    transition(current_room,current_room);
    while(shouldrun)
    {
        al_wait_for_event(queue,&event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:

            if(key[ALLEGRO_KEY_ESCAPE])
                al_ungrab_mouse();

            for(int i = 0; i < ALLEGRO_KEY_MAX; i++)
            {
                key[i] &= KEY_SEEN;
            }

            redraw = true;
            break;
        case ALLEGRO_EVENT_KEY_DOWN:
            key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
            break;
        case ALLEGRO_EVENT_KEY_UP:
            key[event.keyboard.keycode] &= KEY_RELEASED;
            break;
        case ALLEGRO_EVENT_MOUSE_AXES:
            x = event.mouse.x;
            y = event.mouse.y;
            //printf("x:%i y:%i\n",x,y); // used this to get coordinates for transition boxes
            TransitionBox* box = check_transition(x,y,current_room);
            if(box != NULL)
            {
                al_set_mouse_cursor(disp,box->cursor);
            }
            else
            {
                al_set_system_mouse_cursor(disp,ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
            }
            break;
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (event.mouse.button | 1)
            {
                al_grab_mouse(disp);
                TransitionBox* box = check_transition(x,y,current_room);
                if (box != NULL)
                {
                    transition(current_room,box->room);
                    al_set_system_mouse_cursor(disp,ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
                }
            }
            break;
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            shouldrun = false;
            break;
        default:
            break;
        }

        if(redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0,0,0));
            if(current_room->image != NULL)
            {
                al_draw_bitmap(current_room->image,(SCREEN_WIDTH/2)-(al_get_bitmap_width(current_room->image)/2),(SCREEN_HEIGHT/2)-(al_get_bitmap_height(current_room->image)/2),0);
            }
            
            al_flip_display();

            redraw = false;

        }
    }

    current_room = NULL;

    destroy_rooms();

    al_destroy_mouse_cursor(leftcursor);
    al_destroy_mouse_cursor(rightcursor);
    al_destroy_mouse_cursor(upcursor);
    al_destroy_mouse_cursor(downcursor);

    for(int i = 0; i < 4; i++)
    {
        al_destroy_bitmap(cursorbmps[i]);
    }

    //al_destroy_audio_stream(music);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}