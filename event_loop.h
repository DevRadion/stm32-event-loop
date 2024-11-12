#ifndef _EVENT_LOOP_H
#define _EVENT_LOOP_H

#define MAX_EVENTS 10
#define MAX_HANDLERS_PER_EVENT 5

typedef int EventType;

typedef struct {
	EventType event_type;
	void* data;
} Event;

typedef void(*EventHandler)(Event*);

int event_loop_init(void);

int event_loop_register_handler(EventHandler event_handler, EventType event_type);

int event_loop_unregister_handler(EventHandler event_handler, EventType event_type);

int event_loop_send_event(Event event);

#endif // !_EVENT_LOOP_H
