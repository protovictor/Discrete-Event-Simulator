/**
 * Le type PDU, initialement censé représenter les PDU, sert
 * maintenant à manipuler un peu tout ce qui est dynamique dans le
 * système. Il faudra sûrement le renommer lorsque je trouverai un
 * nom pertinent et que je créerai des PDU !
 *
 * A faire : typer les PDU
 */
#ifndef __DEF_PDU
#define __DEF_PDU

#include <motsim.h>

struct PDU_t ;

/*
 * Création d'une PDU de taille fournie. Elle peut contenir
 * un pointeur vers des donnees privees.
 */
extern struct PDU_t * PDU_create(int size, void * private);

/*
 * Consultation de la taille d'une PDU
 */
extern int PDU_size(struct PDU_t * PDU);

extern int PDU_id(struct PDU_t * PDU);

/*
 * Obtention de la date de création
 */
extern motSimDate_t PDU_getCreationDate(struct PDU_t * PDU);

void * PDU_private(struct PDU_t * PDU);

/*
 * Destruction d'une PDU. Les donnees privees doivent avoir
 * ete detruites par l'appelant.
 */
void PDU_free(struct PDU_t * pdu);

/*
 * Le type des fonctions utilisées entre les producteurs
 * et consommateurs de PDU.
 */
typedef struct PDU_t * (*getPDU_t)(void * source);

typedef int (*processPDU_t)(void * receiver,
			    getPDU_t getPDU,
			    void * source);


/*************************************************************************
   Chaining functions
 */

/**
 * @brief Get next PDU
 * @param pdu non NULL
 */
struct PDU_t * PDU_getNext(struct PDU_t * pdu);

/**
 * @brief Get next PDU
 * @param pdu non NULL
 */
struct PDU_t * PDU_getPrev(struct PDU_t * pdu);

/**
 * @brief Get next PDU
 * @param pdu non NULL
 * @param next can be null
 */
void PDU_setNext(struct PDU_t * pdu, struct PDU_t * next);

/**
 * @brief Get next PDU
 * @param pdu non NULL
 * @param prev can be null
 */
void PDU_setPrev(struct PDU_t * pdu, struct PDU_t * prev);


/*
 * Les sondes systeme WARNING : a déclarer ici ?
 */
struct probe_t;

extern struct probe_t * PDU_createProbe;
extern struct probe_t * PDU_reuseProbe;
extern struct probe_t * PDU_mallocProbe;
extern struct probe_t * PDU_releaseProbe;


#endif
