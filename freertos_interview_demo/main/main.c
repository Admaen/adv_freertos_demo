/**
 * This is an example application to demonstrate the assignment
 * with three tasks to execute sequentially.
 *
 * @author Adrian De Vera
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define TASK_STACK_SIZE 2048
#define MESSAGE_HANDLER_STACK_SIZE 2048

#define MAX_MESSAGE_INDEX 20
#define MAX_MESSAGE_LEN 100

#define COMMON_TASK_PRIORITY 1

SemaphoreHandle_t semA, semB, semC;
TaskHandle_t TaskAHandle = NULL;
TaskHandle_t TaskBHandle = NULL;
TaskHandle_t TaskCHandle = NULL;
void taskA(void *);
void taskB(void *);
void taskC(void *);

void taskMessageHandler(void *);
QueueHandle_t MessageQueueHandle = NULL;

typedef struct {
	uint32_t size;
	char * data;
} message;

static inline message create_message(const char * string_input)
{
	if (string_input == NULL) {return (message){0};}

	message my_message;
	my_message.size = strlen(string_input) + 1;
	my_message.data = pvPortMalloc(my_message.size);
	if (my_message.data == NULL)
	{
		my_message.size = 0;
	}
	else
	{
		memset(my_message.data, 0, my_message.size);
		memcpy(my_message.data, string_input, my_message.size);
	}
	return my_message;
}

static inline void destroy_message(message * msg)
{
	if (msg == NULL) {return;}	
	vPortFree(msg->data);
	msg->data = NULL;
}


void app_main(void)
{
	semA = xSemaphoreCreateBinary();
    semB = xSemaphoreCreateBinary();
    semC = xSemaphoreCreateBinary();

	xSemaphoreGive(semA);


	MessageQueueHandle = xQueueCreate(MAX_MESSAGE_INDEX, sizeof(message));
	xTaskCreate(taskA,
				"task a",
				TASK_STACK_SIZE,
				NULL,
				COMMON_TASK_PRIORITY,
				&TaskAHandle);

	xTaskCreate(taskB,
				"task b",
				TASK_STACK_SIZE,
				NULL,
				COMMON_TASK_PRIORITY,
				&TaskBHandle);

	xTaskCreate(taskC,
				"task c",
				TASK_STACK_SIZE,
				NULL,
				COMMON_TASK_PRIORITY,
				&TaskCHandle);

	xTaskCreate(taskMessageHandler,
				"message handler task",
				MESSAGE_HANDLER_STACK_SIZE,
				NULL,
				COMMON_TASK_PRIORITY,
				NULL);
}

void taskA(void * params)
{
	while (1)
	{
		xSemaphoreTake(semA, portMAX_DELAY);
		message msg = create_message("From Task A\n");
		if (msg.data != NULL) {
			if (xQueueSend(MessageQueueHandle, &msg, portMAX_DELAY) != pdTRUE)
				destroy_message(&msg);
		}
		xSemaphoreGive(semB);
	}
}

void taskB(void * params)
{
	while (1)
	{
		xSemaphoreTake(semB, portMAX_DELAY);
		message msg = create_message("From Task B\n");
		if (msg.data != NULL) {
			if (xQueueSend(MessageQueueHandle, &msg, portMAX_DELAY) != pdTRUE)
				destroy_message(&msg);
		}
		xSemaphoreGive(semC);
	}
}
void taskC(void * params)
{
	while (1)
	{
		xSemaphoreTake(semC, portMAX_DELAY);
		message msg = create_message("From Task C\n");
		if (msg.data != NULL) {
			if (xQueueSend(MessageQueueHandle, &msg, portMAX_DELAY) != pdTRUE)
				destroy_message(&msg);	
		}
		xSemaphoreGive(semA);
	}
}

void taskMessageHandler(void * params)
{
	message msg_buffer = {0};

	while (1)
	{
		if (xQueueReceive(MessageQueueHandle, &msg_buffer, portMAX_DELAY))
		{
			printf("%s", msg_buffer.data);
			destroy_message(& msg_buffer);	
			msg_buffer.size = 0;
		}
	}	
}