/*     Un serveur générique */

#include <pdu.h>
#include <motsim.h>

struct srvGen_t;

struct srvGen_t * srvGen_create(void * destination,
                                processPDU_t destProcessPDU);

/*
 * La fonction utilisée par le destinataire pour prendre une PDU servie
 */
struct PDU_t * srvGen_getPDU(struct srvGen_t * srv);

/*
 * La fonction de consommation d'une PDU
 */
void srvGen_processPDU(struct srvGen_t * srv,
		       getPDU_t getPDU, void * source);

/*
 * Obtention de la dernière PDU servie (éventuellement NULL si trop tard !)
 */
struct PDU_t * srvGen_getPDU(struct srvGen_t * srv);

/*
 * Fonction de calcul du temps de service
 */
enum serviceTime_t {
   serviceTimeCst,
   serviceTimeExp,
   serviceTimeProp
} ;

void srvGen_setServiceTime(struct srvGen_t * srv,
			   enum serviceTime_t st,
			   double parameter);

/*
 * Affectation d'une sonde sur le temps de service
 */
void srvGen_setServiceProbe(struct srvGen_t * srv, struct probe_t * serviceProbe);
