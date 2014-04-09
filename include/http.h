/**
 * @file http.h
 * @brief A simple http traffic source model
 *
 * The model contains: reply, request
 * Each of them is represented by a structure
 * 
 * - struct httpRequest_t (for the request)
 * - struct httpReply_t   (for the reply)
 * The structure is described below  
 */ 

#include <pdu-source.h>
#include <file_pdu.h>
#include <date-generator.h>
#include <random-generator.h>
#include <srv-gen.h>
#include <pdu-sink.h>

/*
 *     The declaration of the request and reply structures
 */

struct httpRequest_t;
struct httpReply_t;


/*==========================================================================*/
/*       The functions used for creating default request and reply          */
/*==========================================================================*/

struct httpRequest_t * http_CreateRequest();
struct httpReply_t* http_CreateReply();

/*==========================================================================*/
/*   The functions used for loading request/reply 's parameters into the    */
/*                             random generators                            */
/*==========================================================================*/

/**
 * @brief Load the parameters into the request's structure
 * The parameters can be seted manually, by a user, or we can use 
 * the default parameters - recommended in the suggested model
 * @param Request The request on which we will load the parameters for the 
 * initialisation
 * @param sink The sink for the server and PDUSource creation
 */
void httpRequest_LoadParameters(struct httpRequest_t *Request, struct PDUSink_t * sink);
/**
 * @brief Load the parameters into the reply's structure
 * The parameters can be seted manually, by a user, or we can use 
 * the default parameters - recommended in the suggested model
 * @param Reply The reply on which we will load the parameters for the 
 * initialisation
 * @param sink The sink for the server and PDUSource creation
 */
void httpReply_LoadParameters(struct httpReply_t *Reply, struct PDUSink_t *sink);


/*==========================================================================*/
/*   The functions used for setting request's parameters                    */
/*==========================================================================*/

void httpRequest_setLambda(struct httpRequest_t *Request, double lambda);
void httpRequest_setdebit(struct httpRequest_t *Request, double debit);
void httpRequest_setSizeGenParam(struct httpRequest_t *Request, double alpha, double beta);


/*==========================================================================*/
/*   The functions used for setting reply's parameters                      */
/*==========================================================================*/

void httpReply_setdebit(struct httpReply_t *Reply, double debit);
void httpReply_setDateGenParam(struct httpReply_t *Reply, double alpha, double beta);
void httpReply_setMainSizeGenParam(struct httpReply_t *Reply, double alpha, double beta);
void httpReply_setInlineSizeGenParam(struct httpReply_t *Reply, double alpha, double beta);
void httpReply_setInlineNbGenParam(struct httpReply_t *Reply, double alpha, double beta);

/*==========================================================================*/
/*          The functions used to return request's parameters               */
/*==========================================================================*/

struct dateGenerator_t* httpRequest_GetDateGen(struct httpRequest_t *Request);
struct randomGenerator_t* httpRequest_GetSizeGen(struct httpRequest_t *Request);
struct filePDU_t* httpRequest_GetFilePDU(struct httpRequest_t *Request);
struct srvGen_t* httpRequest_GetServer(struct httpRequest_t *Request);
struct PDUSource_t* httpRequest_GetPDUSource(struct httpRequest_t *Request);

/*
 *   Returns the debit of the server's request
 */
double httpRequest_getDebit(struct httpRequest_t *Request);
/*
 *   Functions for returning the parameters of the size generator
 */
double httpRequest_getalpha(struct httpRequest_t *Request);
double httpRequest_getbeta(struct httpRequest_t *Request);
/*
 *   Return the parameter for the date generator
 */
double httpRequest_getLambda(struct httpRequest_t *Request);

/*==========================================================================*/
/*          The functions used to return reply's parameters                 */
/*==========================================================================*/

struct dateGenerator_t* httpReply_GetDateGen(struct httpReply_t *Reply);
struct randomGenerator_t* httpReply_GetSizeGen(struct httpReply_t *Reply);
struct filePDU_t* httpReply_GetFilePDU(struct httpReply_t *Reply);
struct srvGen_t* httpReply_GetServer(struct httpReply_t *Reply);
struct PDUSource_t* httpReply_GetPDUSource(struct httpReply_t *Reply);
/*
 *   Returns the debit of the server's reply
 */
double httpReply_getDebit(struct httpReply_t *Reply);
/*
 *   Functions for returning the parameters of the date generator
 */
double httpReply_getdWalpha(struct httpReply_t *Reply);
double httpReply_getdWbeta(struct httpReply_t *Reply);
/*
 *   Functions for returning the parameters of the main object size generator
 */
double httpReply_getmLnalpha(struct httpReply_t *Reply);
double httpReply_getmLnbeta(struct httpReply_t *Reply);
/*
 *   Functions for returning the parameters of the inline object size generator
 */
double httpReply_getiLnalpha(struct httpReply_t *Reply);
double httpReply_getiLnbeta(struct httpReply_t *Reply);
/*
 *   Functions for returning the parameters of the inline object number generator
 */
double httpReply_getiGmalpha(struct httpReply_t *Reply);
double httpReply_getiGmbeta(struct httpReply_t *Reply);















