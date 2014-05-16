#include <pdu-source.h>
#include <file_pdu.h>
#include <date-generator.h>
#include <random-generator.h>
#include <srv-gen.h>
#include <pdu-sink.h>

#define MTU 1500        /* maximum transmission unit = 1500 bytes */


struct TCP_client_t;
struct TCP_server_t;
struct getRequest_t;


struct getRequest_t* getRequest_Create(double mainsz, int embnr, struct randomGenerator_t *embsz);

struct TCP_client_t* TCP_clientCreate(struct dateGenerator_t *dg,
                                      struct randomGenerator_t *mainrg,
                                      struct randomGenerator_t *embnr,
                                      struct randomGenerator_t *embsz,
                                      void *destination);
 
struct TCP_server_t* TCP_serverCreate(struct dateGenerator_t *dg, void *destination, processPDU_t destProcessPDU);

void TCP_Send_EmbeddedObjects(struct TCP_server_t *server);

void TCP_session_start(struct TCP_client_t* client, struct TCP_server_t* server);

void HTTP_GET_send(struct TCP_client_t* client, struct TCP_server_t* server);

void TCP_session_SendPage(struct TCP_server_t* server);

struct PDU_t * Server_getPDU(struct TCP_server_t *server);

int client_processPDU( void *destination,
                       getPDU_t getPDU,
                       void *source );

int Http_getEmbeddedNumber(struct getRequest_t *request);
