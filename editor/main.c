#include <allegro5/allegro_primitives.h>
#include "editor.h"


pthread_t console_thread;

void* console(void* args)
{
    char line[100];

    printf("type \"listcmds\" to get a list of commands\n then type \"help <command name>\" to get information about that command\n");

    while (1)
    {
        pthread_mutex_lock(&running_lock);
        if(!shouldrun)
        {
            pthread_mutex_unlock(&running_lock);
            break;
        }
        pthread_mutex_unlock(&running_lock);
        fgets(line,100,stdin);
        line[strcspn(line,"\n")] = '\0';
        parse_input(line);
    }
    return NULL;
}

int main(int argc, char** argv)
{
    //init mutexes

    if(pthread_mutex_init(&running_lock,NULL)!= 0)
    {
        printf("failed to create running_lock\n");
        return 1;
    }

    if(pthread_mutex_init(&rooms_lock,NULL)!= 0)
    {
        printf("failed to create running_lock\n");
        return 1;
    }

    if(pthread_mutex_init(&current_room_lock,NULL)!= 0)
    {
        printf("failed to create current_room_lock\n");
        return 1;
    }

    if(pthread_mutex_init(&current_transition_lock,NULL)!= 0)
    {
        printf("failed to create current_transition_lock\n");
        return 1;
    }

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

    init_cursors(disp);

    //begin register event sources

    al_register_event_source(queue,al_get_mouse_event_source());
    al_register_event_source(queue,al_get_display_event_source(disp));
    al_register_event_source(queue,al_get_timer_event_source(timer));

    //end register event sources

    bool redraw = true;
    ALLEGRO_EVENT event;

    al_start_timer(timer);

    create_command_hash();

    int thread_err;
    if((thread_err = pthread_create(&console_thread,NULL,&console,NULL)) != 0)
    {
        printf("error creating thread: %s\n",strerror(thread_err));
        return 1;
    }
    
    while(1)
    {
        pthread_mutex_lock(&running_lock);
        if(!shouldrun)
        {
            pthread_mutex_unlock(&running_lock);
            break;
        }
        pthread_mutex_unlock(&running_lock);
        
        al_wait_for_event(queue,&event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
            redraw = true;
            break;
        
        case ALLEGRO_EVENT_MOUSE_AXES:
            x = event.mouse.x;
            y = event.mouse.y;
            break;
        
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            pthread_mutex_lock(&running_lock);
            shouldrun = false;
            pthread_mutex_unlock(&running_lock);
            break;
        }

        pthread_mutex_lock(&current_transition_lock);

        switch (editor_mode)
        {
        case MODE_VIEWING:
            switch (event.type)
            {
                case ALLEGRO_EVENT_MOUSE_AXES:
                if(current_room != NULL)
                {
                    TransitionBox* box = check_transition(x,y,current_room);
                    if(box != NULL)
                    {
                        al_set_mouse_cursor(disp,box->cursor);
                    }
                    else
                    {
                        al_set_system_mouse_cursor(disp,ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
                    }
                    }
                break;

                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                if (event.mouse.button == 1)
                {
                    if(current_room != NULL)
                    {
                        TransitionBox* box = check_transition(x,y,current_room);
                        if (box != NULL)
                        {
                            transition(current_room,box->room);
                            al_set_system_mouse_cursor(disp,ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
                        }
                    }
                }
                break;
            }
        break;
        case MODE_ED_TRANSITION:
            switch (event.type)
            {
            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                if (event.mouse.button == 1)
                {
                    switch(points_set)
                    {
                        case 0:
                            current_transition->x1 = x;
                            current_transition->y1 = y;
                            points_set++;
                            printf("point added\n");
                        break;
                        case 1:
                            current_transition->x2 = x;
                            current_transition->y2 = y;
                            points_set++;
                        break;
                        case 2:
                            editor_mode = MODE_VIEWING;
                            points_set = 0;
                        break;
                    }
                }
                else if (event.mouse.button == 2)
                {
                    switch(points_set)
                    {
                        case 2:
                        case 1:
                            points_set--;
                        break;
                        case 0:
                            destroy_transition(current_room,current_transition);
                            editor_mode = MODE_VIEWING;
                        break;
                    }
                }
            break;
            }
            break;
        }

        pthread_mutex_unlock(&current_transition_lock);

        if(redraw && al_is_event_queue_empty(queue))
        {
            
            al_clear_to_color(al_map_rgb(0,0,0));

            pthread_mutex_lock(&current_room_lock);
            
            if(current_room != NULL)
            {
                if(current_room->image != NULL)
                {
                    al_draw_bitmap(current_room->image,(SCREEN_WIDTH/2)-(al_get_bitmap_width(current_room->image)/2),(SCREEN_HEIGHT/2)-(al_get_bitmap_height(current_room->image)/2),0);
                }
            }

            if(editor_mode == MODE_ED_TRANSITION)
            {
                switch (points_set)
                {
                case 1:
                    al_draw_rectangle(current_transition->x1,current_transition->y1,x,y,TRANSITIONBOX_COLOR,3.0f);
                    break;
                case 2:
                    al_draw_rectangle(current_transition->x1,current_transition->y1,current_transition->x2,current_transition->y2,TRANSITIONBOX_COLOR,3.0f);
                }
            }

            pthread_mutex_unlock(&current_room_lock);
            
            al_flip_display();

            redraw = false;

        }

    }

    printf("exiting\n");

    current_room = NULL;

    pthread_join(console_thread,NULL);
    pthread_mutex_destroy(&running_lock);
    pthread_mutex_destroy(&rooms_lock);
    pthread_mutex_destroy(&current_room_lock);

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