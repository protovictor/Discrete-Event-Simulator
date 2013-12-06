/*
 * @file ndesObject.h
 * @brief Définition du type "ancètre" de tous les objets du simulateur
 *
 * L'idée est d'avoir un type général qui permette de regrouper
 * toutes les propriétés communes des objets utilisés dans le simulateur
 * afin d'en simplifier l'usage.
 */
#ifndef __DEF_ndesObject
#define __DEF_ndesObject

#include <motsim.h>
#include <probe.h>

/*
 * @brief Le type général de tous les objets
 * Le type est visible car utilisé par différents modules vu que c'est
 * un peu le couteau suisse de l'outil de simulation. Idéalement, il
 * faut utiliser autant que possible les méthodes de manipulation
 * fournies plus bas.
 */
struct ndesObject_t {
   int      id;            //!< Un identifiant général
   motSimDate_t  creationDate;

   void   * data;  //!< Des donnees privées
   struct ndesTypeHelper_t * helper; //!< Les fonctions spécifiques de manipulation
};

/*
 *  @brief Comment manipuler un type d'objet particulier
 */
struct ndesTypeHelper_t {
   char * name;                 //!< Le nom du type 
   void * (*malloc)() ;         //!< Allocation d'une instance
   void * (*free)(void *) ;     //!< Destruction d'une instance
   void   (*init)(void *);      //!< Initialisation
};

/*-----------------------------------------------------------------------
 * Gestion des types d'objets
 */
void ndesObject_addType(char * name, struct ndesTypeHelper_t * helper);


/*
 * Création d'une ndesObject de taille fournie. Elle peut contenir
 * un pointeur vers des donnees privees.
 */
extern struct ndesObject_t * ndesObject_create(int size, void * private);

extern int ndesObject_id(struct ndesObject_t * ndesObject);

/*
 * Obtention de la date de création
 */
extern motSimDate_t ndesObject_getCreationDate(struct ndesObject_t * ndesObject);

void * ndesObject_private(struct ndesObject_t * ndesObject);

/*
 * Destruction d'une ndesObject. Les donnees privees doivent avoir
 * ete detruites par l'appelant.
 */
void ndesObject_free(struct ndesObject_t * pdu);


#endif
