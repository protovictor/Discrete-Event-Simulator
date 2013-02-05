/*
 * Gestion des files de PDUs. Capacite non limitee par défaut.
 */
#ifndef __DEF_LISTE_PDU
#define __DEF_LISTE_PDU

#include <pdu.h>
#include <probe.h>

struct filePDU_t;

/*
 * Type de la stratégie de perte en cas d'insersion dans une file
 * pleine. Attention, insérer une PDU de taille t dans une file de
 * capacité max < t n'est pas une erreur, mais engendre simplement un
 * événement d'overflow.
 */
enum filePDU_dropStrategy {
  filePDU_dropHead,
  filePDU_dropTail // Stratégie par défaut
};

/*
 * Une file doit être associée à une destination, à laquelle elle
 * transmet chaque paquet reçu par la fonction send().
 */
struct filePDU_t * filePDU_create(void * destination,
			    processPDU_t destProcessPDU);

/*
 * Définition d'une capacité maximale en octets. Une valeur nulle
 * signifie pas de limite.
 */
void filePDU_setMaxSize(struct filePDU_t * file, unsigned long maxSize);
unsigned long filePDU_getMaxSize(struct filePDU_t * file);

void filePDU_setMaxLength(struct filePDU_t * file, unsigned long maxLength);


/*
 * Choix de la stratégie de perte en cas d'insersion dans une file
 * pleine. Attention, insérer une PDU de taille t dans une file de
 * capacité max < t n'est pas une erreur, mais engendre simplement un
 * événement d'overflow.
 */
void filePDU_setDropStrategy(struct filePDU_t * file, enum filePDU_dropStrategy dropStrategy);

/*
 * Insertion d'une PDU dans la file
 */
void filePDU_insert(struct filePDU_t * file,
		    struct PDU_t * PDU);

/*
 * Une fonction permettant la conformité au modèle d'échange
 */
int filePDU_processPDU(void * file,
		       getPDU_t getPDU,
		       void * source);

/*
 * Extraction d'une PDU depuis la file. Ici la signature est
 * directement compatible avec le modèle.
 */
struct PDU_t * filePDU_extract(struct filePDU_t * file);
struct PDU_t * filePDU_getPDU(void * file);

/*
 * Nombre de PDU dans la file
 */
int filePDU_length(struct filePDU_t * file);

int filePDU_size(struct filePDU_t * file);

/*
 * Taille cumulée des n premières PDUs
 */

int filePDU_size_n_PDU(struct filePDU_t * file, int n);

/*
 * Taille du enieme paquet de la file (n>=1)
 */
int filePDU_size_PDU_n(struct filePDU_t * file, int n);
int filePDU_id_PDU_n(struct filePDU_t * file, int n);

/*
 * Affectation d'une sonde sur la taille des PDUs insérées.
 */
void filePDU_addInsertSizeProbe(struct filePDU_t * file, struct probe_t * insertProbe);

/*
 * Ajoût d'une sonde sur la taille des PDU sortantes
 */
void filePDU_addExtractSizeProbe(struct filePDU_t * file, struct probe_t * extractProbe);

/*
 * Ajoût d'une sonde sur la taille des PDU jetées
 */
void filePDU_addDropSizeProbe(struct filePDU_t * file, struct probe_t * dropProbe);

/*
 * Affectation d'une sonde sur le temps de séjour
 */
void filePDU_addSejournProbe(struct filePDU_t * file, struct probe_t * sejournProbe);

/*
 * Mesure du débit d'entrée sur les n-1 dernières PDUs, où n est le
 * nombre de PDUs présentes. Le débit est alors obtenu en divisant la
 * somme des tailles des n-1 dernières PDUs par la durée entre les
 * dates d'arrivée de la première et la dernière.
 * S'il n'y a pas assez de PDUs, le résultat est nul
 *
 * WARNING, a virer, non ?
 */
double filePDU_getInputThroughput(struct filePDU_t * file);
/*
 *
 */
void filePDU_setThroughputInMode();



/*
 * Un affichage un peu moche de la file. Peut être utile dans des
 * phases de débogage.
 */
void filePDU_dump(struct filePDU_t * file);

/*
 * Réinitialisation dans un état permettant de lancer une nouvelle
 * simulation. Ici il suffit de vider la file de tous ses éléments.
 */
void filePDU_reset(struct filePDU_t * file);

#endif
