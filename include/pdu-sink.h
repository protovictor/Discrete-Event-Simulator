#ifndef __DEF_PDU_SINK
#define __DEF_PDU_SINK

#include <pdu.h>

struct PDUSink_t;

struct PDUSink_t * PDUSink_create();

/*
 * La fonction de consommation d'une PDU
 */
int PDUSink_processPDU(void * pduSink, getPDU_t getPDU, void * source);

// WARNING faudrait voir à pouvoir faire ça :
//processPDU_t PDUSink_processPDU;

/*
 * Affectation d'une sonde sur les evenements d'insertion
 */
void PDUSink_setInputProbe(struct PDUSink_t * sink, struct probe_t * insertProbe);

#endif
