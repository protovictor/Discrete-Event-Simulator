#ifndef __DEF_EVENT_FILE
#define __DEF_EVENT_FILE

#include <event.h>

struct eventFile_t;

struct eventFile_t * eventFile_create();

void eventFile_insert(struct eventFile_t * file, struct event_t * event);

struct event_t * eventFile_extract(struct eventFile_t * file);

/*
 * Consultation (sans extraction) du prochain. NULL si file vide
 */
struct event_t * eventFile_nextEvent(struct eventFile_t * file);

int eventFile_length(struct eventFile_t * file);

#endif
