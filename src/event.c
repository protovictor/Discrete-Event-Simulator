#include <stdio.h>     // printf
#include <stdlib.h>    // Malloc, NULL, exit...
#include <assert.h>

#include <event.h>
#include <motsim.h>

/*
 * Une file d'événements libres
 */
struct event_t * freeEvent = NULL;

unsigned long event_nbCreate = 0;
unsigned long event_nbMalloc = 0;
unsigned long event_nbReuse = 0;
unsigned long event_nbFree = 0;

struct event_t * event_create(void (*run)(void *data), void * data, double date)
{
   struct event_t * result;

   if (freeEvent) {
      result = freeEvent;
      freeEvent = result->next;
      event_nbReuse++;
   } else {
      result = (struct event_t *)sim_malloc(sizeof(struct event_t));
      event_nbMalloc ++;
   }
   assert(result);
   event_nbCreate ++;

   result->type = 0;
   result->period = 0.0;

   result->run = run;
   result->data = data;
   result->date = date;

   result->prev = NULL;
   result->next = NULL;

   return result;
}

/*
 * La même, avec insersion dans le simulateur
 */
void event_add(void (*run)(void *data), void * data, double date)
{
   motSim_addEvent(event_create(run, data, date));
}


struct event_t * event_periodicCreate(void (*run)(void *data), void * data, double date, double period)
{
   struct event_t * result;

   result = event_create(run, data, date);

   result->type = EVENT_PERIODIC;
   result->period = period;

   return result;
}

/*
 * La même, avec insersion dans le simulateur
 */
void event_periodicAdd(void (*run)(void *data), void * data, double date, double period)
{
  motSim_addEvent(event_periodicCreate(run, data, date, period));
}

void free_event(struct event_t * ev)
{
  event_nbFree++;
   ev->next = freeEvent;
   freeEvent = ev;
}

void event_run(struct event_t * event)
{
   printf_debug(DEBUG_EVENT, " running ev %p at %f\n", event, event->date);
   event->run(event->data);
   if (event->type &EVENT_PERIODIC)
   {
      event->date += event->period;
      motSim_addEvent(event);
   }
   else {
      free_event(event);
   }

}

double event_getDate(struct event_t * event)
{
   return event->date;
}

