#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdbool.h>
#include <stdio.h>

#include "game.h"

#define KEY_SEEN 1
#define KEY_RELEASED 2

unsigned char key[ALLEGRO_KEY_MAX];

int main(int argc, char** argv)
{
    memset(key,0,sizeof(key));

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

    init_cursors(disp);

    load_game("gamedata");

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
            if (event.mouse.button == 1)
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

    al_shutdown_image_addon();

    return 0;
}