/**
 * @file file_pdu.h
 * @brief D√©finition de la gestion des files de PDUs
 *
 * Par d√©faut, une liste √† une capacit√© non limit√©e et fonctionne
 * selon une politique FIFO.
 */
#ifndef __DEF_LISTE_PDU
#define __DEF_LISTE_PDU

#include <pdu.h>
#include <probe.h>

struct filePDU_t;

/**
 * Type de la strat√©gie de perte en cas d'insersion dans une file
 * pleine. Attention, ins√©rer une PDU de taille t dans une file de
 * capacit√© max < t n'est pas une erreur, mais engendre simplement un
 * √©v√©nement d'overflow.
 */
enum filePDU_dropStrategy {
  filePDU_dropHead,
  filePDU_dropTail // Strat√©gie par d√©faut
};

/** @brief CrÈation d'une file.
 * 
 *  @param destination l'entitÈ aval (ou NULL ai aucune)
 *  @param destProcessPDU la fonction de traitement de l'entitÈ aval
 *  (ou NULL si aucune entitÈ)
 *  @return Une strut filePDU_t * allouÈe et initialisÈe
 *
 *  Il est possible de ne pas fournir d'entitÈ aval en paramËtre, car
 *  une file peut Ítre utilisÈe Ègalement comme un simple outil de
 *  gestion mÈmoire, sans entrer dans un modËle de rÈseau. On
 *  utilisera alors simplement les fonctions d'insertion et d'extraction
 */
struct filePDU_t * filePDU_create(void * destination,
			    processPDU_t destProcessPDU);

/*
 * D√©finition d'une capacit√© maximale en octets. Une valeur nulle
 * signifie pas de limite.
 */
void filePDU_setMaxSize(struct filePDU_t * file, unsigned long maxSize);
unsigned long filePDU_getMaxSize(struct filePDU_t * file);

void filePDU_setMaxLength(struct filePDU_t * file, unsigned long maxLength);
unsigned long filePDU_getMaxLength(struct filePDU_t * file);


/*
 * Choix de la strat√©gie de perte en cas d'insersion dans une file
 * pleine. Attention, ins√©rer une PDU de taille t dans une file de
 * capacit√© max < t n'est pas une erreur, mais engendre simplement un
 * √©v√©nement d'overflow.
 */
void filePDU_setDropStrategy(struct filePDU_t * file, enum filePDU_dropStrategy dropStrategy);

/**
 * @brief Insertion d'une PDU dans la file
 * @param file la file dans laquelle on insËre la PDU
 * @param PDU la PDU ‡ insÈrer ‡ la fin de la file
 *
 * Si une destination a ÈtÈ affectÈe ‡ la file, alors la fonction de
 * traitement de cette destination est invoquÈe.
 */
void filePDU_insert(struct filePDU_t * file,
		    struct PDU_t * PDU);

/*
 * Une fonction permettant la conformit√© au mod√®le d'√©change
 */
int filePDU_processPDU(void * file,
		       getPDU_t getPDU,
		       void * source);

/**
 * @brief Extraction d'une PDU depuis la file
 * @param file la file depuis laquelle on souhaite extraire la
 * premiËre PDU
 * @return la premiËre PDU ou NULL si la file est vide
 */
struct PDU_t * filePDU_extract(struct filePDU_t * file);

/**
 * @brief Extraction d'une PDU depuis la file
 * @param file la file depuis laquelle on souhaite extraire la
 * premiËre PDU
 * @return la premiËre PDU ou NULL si la file est vide
 * 
 * Ici la signature est directement compatible avec le mod√®le
 * d'entrËe-sortie de NDES.
 */
struct PDU_t * filePDU_getPDU(void * file);

/*
 * Nombre de PDU dans la file
 */
int filePDU_length(struct filePDU_t * file);

int filePDU_size(struct filePDU_t * file);

/**
 * @brief Taille cumulÈe des n premiËres PDUs
 * @param file la file 
 * @param n le nombre (positif ou nul) de PDUs
 * @return le cumul des tailles des n premiËres PDUs de la file
 */
int filePDU_size_n_PDU(struct filePDU_t * file, int n);

/*
 * Taille du enieme paquet de la file (n>=1)
 */
int filePDU_size_PDU_n(struct filePDU_t * file, int n);
int filePDU_id_PDU_n(struct filePDU_t * file, int n);

/*
 * Affectation d'une sonde sur la taille des PDUs ins√©r√©es.
 */
void filePDU_addInsertSizeProbe(struct filePDU_t * file, struct probe_t * insertProbe);

/*
 * Ajo√ªt d'une sonde sur la taille des PDU sortantes
 */
void filePDU_addExtractSizeProbe(struct filePDU_t * file, struct probe_t * extractProbe);

/*
 * Ajo√ªt d'une sonde sur la taille des PDU jet√©es
 */
void filePDU_addDropSizeProbe(struct filePDU_t * file, struct probe_t * dropProbe);

/*
 * Affectation d'une sonde sur le temps de s√©jour
 */
void filePDU_addSejournProbe(struct filePDU_t * file, struct probe_t * sejournProbe);

/*
 * Mesure du d√©bit d'entr√©e sur les n-1 derni√®res PDUs, o√π n est le
 * nombre de PDUs pr√©sentes. Le d√©bit est alors obtenu en divisant la
 * somme des tailles des n-1 derni√®res PDUs par la dur√©e entre les
 * dates d'arriv√©e de la premi√®re et la derni√®re.
 * S'il n'y a pas assez de PDUs, le r√©sultat est nul
 *
 * WARNING, a virer, non ?
 */
double filePDU_getInputThroughput(struct filePDU_t * file);
/*
 *
 */
void filePDU_setThroughputInMode();



/*
 * Un affichage un peu moche de la file. Peut √™tre utile dans des
 * phases de d√©bogage.
 */
void filePDU_dump(struct filePDU_t * file);

/*
 * R√©initialisation dans un √©tat permettant de lancer une nouvelle
 * simulation. Ici il suffit de vider la file de tous ses √©l√©ments.
 */
void filePDU_reset(struct filePDU_t * file);

#endif
