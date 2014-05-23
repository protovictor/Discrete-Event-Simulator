#include "file_pdu.h"
#include "date-generator.h"
#include "random-generator.h"

#define MAX_SIZE    2000000  /* 2MB */
#define MAX_NUMBER  53       /* the maximum number of embedded objects */
#define MTU 1500             /* maximum transmission unit = 1500 bytes => the webpage is splitted */


struct TCP_client_t;
struct TCP_server_t;
struct getRequest_t;
struct PDUList_t;

/**
 *  @brief This function creates a http request based on the given parameters
 *  @param mainsz the size of the main object of a webpage
 *  @param embnr  the number of embedded objects in a webpage
 *  @param embsz  a pointer to the random generator that gives a random size for each embedded object
 */
struct getRequest_t* getRequest_Create(int mainsz, int embnr, struct randomGenerator_t *embsz);



/**
 * @brief The function creates a structure that holds the parameters of a request
 * @param mainszg a pointer to the random generator that gives a random size for the main object
 * @param embnr a pointer to the random generator that gives a random size for the number of embedded objects
 * @param embsz a pointer to the random generator that gives a random size for each embedded object
 * @param destination a pointer to the destination of the received packets
 * @param destProcessPDU the function that will process the received packets
 */
struct TCP_client_t* TCP_clientCreate(//struct dateGenerator_t *dg,
                                      struct randomGenerator_t *maiszg,
                                      struct randomGenerator_t *embnr,
                                      struct randomGenerator_t *embsz,
                                      void *destination,
                                      processPDU_t destProcessPDU
                                      );


/**
 * @brief The function creates a webserver that send a webpage to the client who requested it
 * @param dg a pointer to the date generator that gives random departure dates for each object in the webpage
 * @param destination a pointer to the structure that will hold the packets that need to be transmitted
 * @param destProcessPDU the function that will process the packets from the destination
 */
struct TCP_server_t* TCP_serverCreate(struct dateGenerator_t *dg,
                                       void *destination,
                                       processPDU_t destProcessPDU);

/**
 * @brief The function starts the TCP session (the connection establishment is ignored)
 * @param client the client that wants to read a webpage
 * @param server the server that will respond with a webpage
 */
void TCP_session_start(struct TCP_client_t* client, struct TCP_server_t* server);


/**
 * @brief The function passes to the server the parameters of the requested page
 * @param client the one that "sends" the request
 * @param server the one that receives the request and respons with a webpage
 */
void HTTP_GET_send(struct TCP_client_t* client, struct TCP_server_t* server);


/**
 * @brief The function sends the whole webpage to the source that requested it
 * @param server the one that sends the webpage (composed from main object and
 * multiple embedded objects)
 */
void TCP_session_SendPage(struct TCP_server_t* server);


/**
 * @brief The function sends the embedded objects of a webpage to the destination
 * @param server the server that will send the objects
 */

void TCP_Send_EmbeddedObjects(struct TCP_server_t *server);


/**
 * @brief The function that transmits each object of a webpage to the destination
 * @param server the one that sends each object
 */
void TCP_Transmit_Object(struct TCP_server_t *server);


/**
 * @brief The function fragments an object of a size > MTU in multiple PDUs
 * of maximum size equal to MTU, to prepare them to be sent to the destination
 * @param server the one that sends the objects
 * @param size the size of the object to be fragmented
 */
struct TCP_server_t* TCP_server_PDUList_Create(struct TCP_server_t *server, int size);

/**
 * @brief The function is responsible with getting each PDU from the server
 * @param server the one that sens the webpage
 */
struct PDU_t * Server_getPDU(struct TCP_server_t *server);

/**
 * @brief The function gets the number of embedded objects remained to transmit
 * This number decrements after each transmission
 * @param request the structure that holds parameters of the requested webpage
 */
int TCP_getEmbeddedNumber(struct getRequest_t *request);

/**
 * @brief The function updates the number of remained embedded objects to transmit
 * @param request the structure that holds parameters of the requested webpage
 * @param value the new number of the embedded objects left to transmit
 */
void TCP_setEmbeddedNumber(struct getRequest_t *request, int value);


/**
 * @brief The function sets the default distribution types and the default parameters
 * @param mainsz a pointer to the random generator for the main size object
 * @param embnr a pointer to the random generator for the number of embedded objects
 * @param embsz a pointer to the random generator for the size of embedded objects
 */
void Request_LoadDefault(struct randomGenerator_t *mainsz,
                          struct randomGenerator_t *embnr,
                          struct randomGenerator_t *embsz);
