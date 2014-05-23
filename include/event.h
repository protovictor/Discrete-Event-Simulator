/**
 * @file event.h
 * @brief Définition des événements de NDES
 *
 * Un événement est décrit par une fonction, un pointeur et une
 * date. A la date choisie, la fonction est invoquée avec le pointeur
 * en guise de paramêtre.
 */
#ifndef __DEF_EVENT
#define __DEF_EVENT

#include "motsim.h"

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

/**
 * @brief  Création d'un événement
 * Cet événement devra être exécuté à la date passée en
 * paramètre.
 * A cette date, la fonction 'run' sera invoquée avec le
 * paramêtre 'data' en paramètre.
 * ATTENTION, il faut l'insérer dans la liste du simulateur, sinon
 * l'événement ne sera jamais exécuté. Pour cela, on utilisera la
 * fonction motSim_addEvent.
 *
 * @param run La fonction à invoquer lors de l'occurence de l'événement
 * @param data Un pointeur (ou NULL) passé en paramètre à run
 * @param date Date à laquelle exécuter l'événement
 * @return L'événement créé
 */
struct event_t * event_create(void (*run)(void *data),
			      void * data,
			      double date);

/**
 * @brief  Création et insertion d'un événement
 * Cet événement devra être exécuté à la date passée en
 * paramètre.
 * A cette date, la fonction 'run' sera invoquée avec le
 * paramêtre 'data' en paramètre.
 *
 * @param run La fonction à invoquer lors de l'occurence de l'événement
 * @param data Un pointeur (ou NULL) passé en paramètre à run
 * @param date Date à laquelle exécuter l'événement
 */
void event_add(void (*run)(void *data),
	       void * data,
	       double date);

/**
 * @brief  Création d'un événement périodique
 * Cet événement devra être exécuté à partir de la date passée en
 * paramètre de façon périodique.
 * A chaquee date, la fonction 'run' sera invoquée avec le
 * paramêtre 'data' en paramètre.
 * ATTENTION, il faut l'insérer dans la liste du simulateur, sinon
 * l'événement ne sera jamais exécuté. Pour cela, on utilisera la
 * fonction motSim_addEvent.
 *
 * @param run La fonction à invoquer lors de l'occurence de l'événement
 * @param data Un pointeur (ou NULL) passé en paramètre à run
 * @param date Date de la première occurence de l'événement
 * @param period Période d'exécution
 * @return L'événement créé
 */
struct event_t * event_periodicCreate(void (*run)(void *data),
				      void * data,
				      double date,
				      double period);

/**
 * @brief  Création et insertion d'un événement périodique
 * Cet événement devra être exécuté à partir de la date passée en
 * paramètre de façon périodique.
 * A chaquee date, la fonction 'run' sera invoquée avec le
 * paramêtre 'data' en paramètre.
 * @param run La fonction à invoquer lors de l'occurence de l'événement
 * @param data Un pointeur (ou NULL) passé en paramètre à run
 * @param date Date de la première occurence de l'événement
 * @param period Période d'exécution
 */
void event_periodicAdd(void (*run)(void *data),
		       void * data,
		       double date,
		       double period);

double event_getDate(struct event_t * event);

void event_run(struct event_t * event);

#endif
