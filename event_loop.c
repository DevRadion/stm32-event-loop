#include "event_loop.h"
#include "FreeRTOS.h"
#include "queue.h"

static QueueHandle_t event_queue;

static EventHandler event_handlers[MAX_EVENTS][MAX_HANDLERS_PER_EVENT] = { 0 };

static void event_loop_handle_task(void* pv_parameters);

int event_loop_init(void)
{
	event_queue = xQueueCreate(MAX_EVENTS, sizeof(Event));
	
	if (event_queue == NULL) 
		return -1;
	
	int task_result = xTaskCreate(event_loop_handle_task, "event_loop_handle_task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	
	if (task_result != 1)
		return -1;
	
	return 0;
}


int event_loop_register_handler(EventHandler event_handler, EventType event_type)
{
	if (event_type >= MAX_EVENTS || event_handler == NULL)
		return -1;
	
	for (int i = 0; i < MAX_HANDLERS_PER_EVENT; i++)
	{
		if (event_handlers[event_type][i] == NULL)
		{
			event_handlers[event_type][i] = event_handler;
			return 0;
		}
	}
	
	return -1;
}

int event_loop_unregister_handler(EventHandler event_handler, EventType event_type)
{
	for (int i = 0; i < MAX_HANDLERS_PER_EVENT; i++) {
		if (event_handlers[event_type][i] == event_handler) {
			event_handlers[event_type][i] = NULL;
			return 0;
		}
	}
	
	return -1;
}

int event_loop_send_event(Event event) {
	// Check if the current context is ISR
	if (xPortIsInsideInterrupt()) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if (xQueueSendFromISR(event_queue, &event, &xHigherPriorityTaskWoken) != pdPASS) {
			return -1;
		}
        
		// Optionally, request a context switch if needed
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else {
		if (xQueueSend(event_queue, &event, portMAX_DELAY) != pdPASS) {
			return -1;
		}
	}

	return 0;
}

static void event_loop_handle_task(void* pv_parameters)
{
	Event event;
	
	while (1)
	{
		if (xQueueReceive(event_queue, &event, portMAX_DELAY) == pdPASS)
		{
			for (int i = 0; i < MAX_HANDLERS_PER_EVENT; i++)
			{
				if (event_handlers[event.event_type][i] != NULL)
				{	
					event_handlers[event.event_type][i](&event);
				}
			}
		}
	}
}