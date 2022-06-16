#include <allegro5/allegro_primitives.h>
#include "editor.h"

int main(int argc, char** argv)
{
    //begin init allegro systems

    must_init(al_init(),"allegro");

    must_init(al_install_mouse(),"mouse");

    must_init(al_install_audio(),"audio");

    must_init(al_init_image_addon(),"image addon");

    must_init(al_init_primitives_addon(),"primitives addon");

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

    //begin register event sources

    al_register_event_source(queue,al_get_mouse_event_source());
    al_register_event_source(queue,al_get_display_event_source(disp));
    al_register_event_source(queue,al_get_timer_event_source(timer));

    //end register event sources

    bool redraw = true;
    ALLEGRO_EVENT event;

    al_start_timer(timer);

    create_command_hash();
    
    printf("type \"listcmds\" to get a list of commands\n then type \"help <command name>\" to get information about that command\n");
    char line[100];
    while(shouldrun)
    {
        al_wait_for_event(queue,&event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
            redraw = true;
            break;
        
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            shouldrun = false;
            break;
        }

        switch (editor_mode)
        {
        case MODE_VIEWING:
            fgets(line,100,stdin);
            line[strcspn(line,"\n")] = '\0';
            parse_input(line);
            break;
        case MODE_ED_TRANSITION:
            editor_mode = MODE_VIEWING;
            break;
        }

        if(redraw && al_is_event_queue_empty(queue))
        {
            
            al_clear_to_color(al_map_rgb(0,0,0));
            if(current_room != NULL)
            {
                if(current_room->image != NULL)
                {
                    al_draw_bitmap(current_room->image,(SCREEN_WIDTH/2)-(al_get_bitmap_width(current_room->image)/2),(SCREEN_HEIGHT/2)-(al_get_bitmap_height(current_room->image)/2),0);
                }
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
    al_shutdown_primitives_addon();

    return 0;
}