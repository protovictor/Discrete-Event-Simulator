/*
 * Un serveur generique. Il prend des PDU en entree, les sert
 * en un temps defini par une loi (cte, exp, ...) et les envoie
 * en sortie.
 */
#include <assert.h>

#include <motsim.h>
#include <event.h>
#include <date-generator.h>
#include <srv-gen.h>

#include <stdlib.h>    // Malloc, NULL, exit...

enum srvState_t {
  srvStateIdle,
  srvStateBusy
};

struct srvGen_t {
   enum srvState_t          srvState;
   struct PDU_t           * currentPDU;   // The PDU being served
   float                    serviceStartTime;
   struct dateGenerator_t * dateGenerator;
  
   // Gestion du destinataire
   void * destination;          // L'objet auquel sont destinées les PDUs
   processPDU_t destProcessPDU; // La fonction permettant d'envoyer la PDU   

   // WARNING : les attributs suivants ne nous permettent pas de
   // préparer la réception de plusieurs PDU !!
   void * source ;  // L'objet susceptible de nous fournir une PDU
   getPDU_t getPDU; // La méthode d'obtention  de cette PDU

   // Les sondes
   struct probe_t * serviceProbe;
};

/*
 * Creation et initialisation d'un serveur
 */
struct srvGen_t * srvGen_create(void * destination,
                                processPDU_t destProcessPDU)
{
   struct srvGen_t * result = sim_malloc(sizeof(struct srvGen_t));

   result->srvState = srvStateIdle;
   result->currentPDU = NULL;

   // La destination des PDU servies
   result->destination = destination;
   result->destProcessPDU = destProcessPDU;

   // Le mode de calcul du temps de traitement
   result->dateGenerator = dateGenerator_createExp(10.0);

   // WARNING A modifier (cf struct)
   result->source = NULL;

   result->serviceProbe = NULL;

   return result;
}


void srvGen_terminateProcess(struct srvGen_t * srv);
/*
 * Début de traitement d'une PDU
 */
void srvGen_startService(struct srvGen_t * srv, struct PDU_t * pdu)
{
   struct event_t * event;
   double date ; 

   assert(pdu != NULL);

   // Si le destinataire n'a pas récupéré la précédente, trop tard !
   if (srv->currentPDU != NULL) {
      PDU_free(srv->currentPDU);
   }

   srv->srvState = srvStateBusy;
   srv->serviceStartTime = motSim_getCurrentTime();
   srv->currentPDU = pdu;

   //Déterminer une date de fin en fonction du temps de traitement
   date = dateGenerator_nextDate(srv->dateGenerator, motSim_getCurrentTime());

   printf_debug(DEBUG_SRV, " PDU %d from %6.3f to %6.3f\n", PDU_id(pdu), motSim_getCurrentTime(), date);

   // On crée un événement pour cette date
   event = event_create((eventAction_t)srvGen_terminateProcess, srv, date);

   // On ajoute cet événement au simulateur
   motSim_addEvent(event);
}

/*
 * Affectation d'une sonde sur le temps de service
 */
void srvGen_setServiceProbe(struct srvGen_t * srv, struct probe_t * serviceProbe)
{
  dateGenerator_setInterArrivalProbe(srv->dateGenerator, serviceProbe);
}

/*
 * Fin de traitement d'une PDU
 */
void srvGen_terminateProcess(struct srvGen_t * srv)
{
   struct PDU_t * pdu;

   srv->srvState = srvStateIdle;

   printf_debug(DEBUG_SRV, " end of service\n");

   if (srv->serviceProbe) {
      probe_sample(srv->serviceProbe, motSim_getCurrentTime() - srv->serviceStartTime);
   }

   // On propose la PDU à la destination
   srv->destProcessPDU(srv->destination, srvGen_getPDU, srv);

   // On va chercher la prochaine s'il y en a une
   // WARNING : et s'il y en a deux ?
   if (srv->source) {
      printf_debug(DEBUG_SRV, " looking for a new PDU\n");
      pdu = srv->getPDU(srv->source);
      // Est-elle encore prete ?
      if (pdu) {
         srvGen_startService(srv, pdu);
      } else { // Si elle ne l'est plus, inutile d'y revenir pour le moment
         srv->source = NULL; 
      }
   }
}

/*
 * Notification de la présence d'une PDU
 */
void srvGen_processPDU(struct srvGen_t * server,
                       getPDU_t getPDU, void * source)
{
   struct PDU_t * pdu;

   printf_debug(DEBUG_SRV, " server processing\n");

   // Si je suis dispo, je vais la chercher et la traiter
   if (server->srvState == srvStateIdle){
      printf_debug(DEBUG_SRV, " server will serve PDU\n");

      // On va chercher une PDU puisqu'il y en a une de prête
      pdu = getPDU(source);
      srvGen_startService(server, pdu);

   // Sinon, je note qu'elle est dispo
   } else {
      printf_debug(DEBUG_SRV, " server unavailable\n");
      if (server->source != NULL) {
	 if ((server->source != source) || (server->getPDU != getPDU)) {
            printf_debug(DEBUG_ALWAYS, " server can not handle multiple sources.\n");
	    exit(1);
	 }
      } else {
        server->source = source;
        server->getPDU = getPDU;
      }
   }
}

/*
 * Obtention de la dernière PDU servie (éventuellement NULL si trop tard !)
 */
struct PDU_t * srvGen_getPDU(struct srvGen_t * srv)
{
   struct PDU_t * pdu = srv->currentPDU;
   
   srv->currentPDU = NULL;

   return pdu;
}

/*----------------------------------------------------------------------*/
/*   Traitement en un temps constant                                    */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*   Traitement en un temps dépendant linéairement de la taille         */
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*   Traitement en un temps dépendant linéairement de la taille         */
/*----------------------------------------------------------------------*/
