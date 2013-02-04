#ifndef __DEF_EVENT
#define __DEF_EVENT

#include <motsim.h>

struct event_t {
   int    type;
   double period;  // Pour les événements périodiques
   double date;
   void * data;
   void (*run)(void * data);

   // Pour le chainage
   struct event_t * prev;
   struct event_t * next;
};

#define EVENT_PERIODIC 0x00000001

/*
 * Les mesures suivantes pourraient être faites par des sondes
 */
extern unsigned long event_nbCreate;
extern unsigned long event_nbMalloc;
extern unsigned long event_nbReuse;
extern unsigned long event_nbFree;

typedef void (*eventAction_t)(void *);

/*
 * Création d'un événement qui devra être exécuté à la date passée en
 * paramètre. A cette date, la fonction 'run' sera invoquée avec le
 * paramêtre 'data' en paramètre.
 * ATTENTION, il faut l'insérer dans la liste du simulateur
 */
struct event_t * event_create(void (*run)(void *data),
			      void * data,
			      double date);

/*
 * La même, avec insersion dans le simulateur
 */
void event_add(void (*run)(void *data),
	       void * data,
	       double date);

/*
 * Création d'un événement périodique
 */
struct event_t * event_periodicCreate(void (*run)(void *data), void * data, double date, double period);

/*
 * La même, avec insersion dans le simulateur
 */
void event_periodicAdd(void (*run)(void *data), void * data, double date, double period);

double event_getDate(struct event_t * event);

void event_run(struct event_t * event);

#endif
