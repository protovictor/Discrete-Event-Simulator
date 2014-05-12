#include <stdlib.h>    // Malloc, NULL, ...
#include <assert.h>

#include <stdio.h>     // printf, ...

#include <event.h>
#include <pdu.h>

struct eventFile_t {
   int nombre;
   struct event_t * premier;
   struct event_t * dernier;
};

struct eventFile_t * eventFile_create()
{
   struct eventFile_t * result = (struct eventFile_t *) sim_malloc(sizeof(struct eventFile_t));

   result->nombre = 0;
   result->premier = NULL;
   result->dernier = NULL;

   return result;
}

// WARNING : il faut trier !!
void eventFile_insert(struct eventFile_t * file, struct event_t * event)
{
   struct event_t * precedent = file->dernier;

   // On cherche sa place
   while ((precedent) && (event_getDate(precedent) > event_getDate(event))){
      precedent = precedent->prev;
   }

   // precedent est null ou represente le dernier evenement AVANT event

   // Si precedent == NULL, event est le premier
   if (precedent == NULL) {
      event->prev = NULL;
      event->next = file->premier;
      // Si ce n'est pas le seul
      if (file->premier) {
         file->premier->prev = event;
      } else { // S'il est seul, il est aussi dernier
         file->dernier = event;
      }
      file->premier = event;
   } else {
      event->next = precedent->next;
      precedent->next = event;
      event->prev = precedent;
      if (event->next) {
         event->next->prev = event;
      } else { // C'est le dernier
         file->dernier = event;
      }
   }

   file->nombre++;

 
}

struct event_t * eventFile_extract(struct eventFile_t * file)
{  
   
  
  struct event_t * premier = NULL;

   if (file->premier) {
      premier = file->premier;
      file->premier = premier->next;
      // Si c'était le seul
      if (file->dernier == premier) { 
         assert(premier->next == NULL);
	// assert(file->nombre == 1);
         if(file->nombre ==1 )
         file->dernier = NULL;
      } else { // Il en reste un
	 assert(file->premier != NULL);
         file->premier->prev = NULL;
      }
      file->nombre --;
   }
   
   return premier;
}

/*
 * Consultation (sans extraction) du prochain
 */
struct event_t * eventFile_nextEvent(struct eventFile_t * file)
{
   if (file->premier) {
      return file->premier;
   }

   return NULL;

}


int eventFile_length(struct eventFile_t * file)
{
   return file->nombre;
}

void eventFile_dump(struct eventFile_t * file)
{
   struct event_t * el;

   for (el = file->premier; el != NULL; el = el->next) {
     //   for (el = file->premier; el != file->dernier; el = el->suivant) {
      printf("(%p : %6.3f) ", el, event_getDate(el));
   }
   printf("\n");
}
