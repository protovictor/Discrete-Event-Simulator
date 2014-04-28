/*
 * @file dvb-s2-ll.h
 * @brief Modélisation d'une couche de type DVB-S2
 *
 * Attention, ici les tailles sont comptées en bits alors que partout
 * ailleurs c'est en octets. Il faudra peut être revoir ça. A titre
 * préventif, cela apparait dans les noms des variables, et des macros
 * permettent d'envisager une modificiation sans conséquences sur le
 * reste.
 * Autre problème : il faudrait un point d'entrée par MODCOD. Pour le
 * moment, le MODCOD d'une BBFRAME est passé en private de la PDU
 * correspondante, ce qui n'est pas terrible du tout !
 */

#ifndef __DEF_DVBS2_LL
#define __DEF_DVBS2_LL

#include <pdu.h>
#include <probe.h>

#define NB_MODCOD_MAX 4

struct t_modcod;
struct DVBS2ll_t;

/*
 * @brief Création d'une entité DVB-S2 couche 2
 * Attention, elle ne contient
 * aucun MODCOD par défaut, il faut en ajouter
 * @param destination L'entité aval
 * @param destProcessPDU La fonction de traitement par l'aval
 * @param symbolPerSecond Le débit de la ligne, en symboles par seconde
 * @param FECFrameBitLength La taille totale de la BBFRAME
 * @result Un pointeur sur la structure créée ou NULL
 */
struct DVBS2ll_t * DVBS2ll_create(void * destination,
				  processPDU_t destProcessPDU,
				  unsigned long  symbolPerSecond,
				  unsigned int FECFrameBitLength);

#define FEC_FRAME_BITSIZE_LARGE 64800

/*
 *Constantes permettant le paramétrage des MODCODs
 */
#define  C14SIZE  16008
#define  C13SIZE  21408
#define  C25SIZE  25728
#define  C12SIZE  32208
#define  C35SIZE  38688
#define  C23SIZE  43040
#define  C34SIZE  48408
#define  C45SIZE  51648
#define  C56SIZE  53840
#define  C89SIZE  57472
#define  C910SIZE 58192

#define   MQPSK    2
#define   M8PSK    3
#define   M16APSK  4
#define   M32APSK  5

/**
 * @brief Ajout d'un MODCOD
 * Le codage est paramétré par le nombre de bits
 * par BBFRAME et la modulation par le nombre de bits par symbole.
 * La valeur retournée est l'indice de ce nouveau MODCOD.
 */
int DVBS2ll_addModcod(struct DVBS2ll_t * dvbs2ll,
		      unsigned int bbframeBitLength,
		      unsigned int bitsPerSymbol);

/*
 * Modification des propriétés du MODCOD n
 */
void DVBS2ll_setModcod(struct DVBS2ll_t * dvbs2ll,
                       int n,
		       unsigned int bbframeBitLength,
		       unsigned int bitsPerSymbol);
/************************************************************************/
/*    Le modele in/out                                                  */
/************************************************************************/
/*
 * Attribution d'une source. Attention c'est obligatoire ici car c'est
 * l'entité DVBS2ll qui va solliciter la source lorsque le support
 * sera libre
 */
void DVBS2ll_setSource(struct DVBS2ll_t * dvbs2ll, void * source, getPDU_t getPDU);

/*
 * Une fonction permettant la conformité au modèle d'échange
 */
int DVBS2ll_processPDU(struct DVBS2ll_t * dvbs2ll,
                        getPDU_t getPDU,
                        void * source);

/*
 * Emission d'une PDU au travers d'un MODCOD sélectionné. La PDU doit
 * être d'une taille inférieure ou égale à la taille de charge utile du
 * MODCOD choisi.
 */
void DVBS2ll_sendPDU(struct DVBS2ll_t * dvbs2ll, struct PDU_t * pdu);

/************************************************************************/
/*    Les sondes                                                        */
/************************************************************************/
/*
 * Ajout d'une sonde sur la taille de la charge utile des trames émises
 * sur un MODCOD donné
 */
void DVBS2ll_addActualPayloadBitSizeProbe(struct DVBS2ll_t * dvbs2ll, int mc, struct probe_t * pr);

/*
 * Ajout de la probe sur les DUMMY. Elle n'échantillonne aucune
 * valeur, juste les dates d'émission de DUMMY frames.
 */
void DVBS2ll_addDummyFecFrameProbe(struct DVBS2ll_t * dvbs2ll, struct probe_t * pr);

/************************************************************************/
/*
 * Nombre de MODCODs
 */
int DVBS2ll_nbModcod(struct DVBS2ll_t * dvbs2ll);

/*
 * Capacité d'une BBFRAME associée au MODCOD d'indice fourni
 */ 
unsigned int DVBS2ll_bbframePayloadBitSize(struct DVBS2ll_t * dvbs2ll, int mcIdx);

/*
 * Nombre de bits par symbole d'un modcod
 */
unsigned int DVBS2ll_bitsPerSymbol(struct DVBS2ll_t * dvbs2ll, int mcIdx);

/*
 * Temps d'émission d'une BBFRAME associée au MODCOD d'indice fourni
 */ 
double DVBS2ll_bbframeTransmissionTime(struct DVBS2ll_t * dvbs2ll, int mcIdx);

/*
 * Le support est-il disponible ?
 */
int DVBS2ll_available(struct DVBS2ll_t * dvbs2ll);



#endif
