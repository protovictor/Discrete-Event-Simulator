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

/**
 * @brief The function changes the default value of lambda into a given value
 * @param Request The Request on which we make modifications
 * @param lamda The lambda parameter used for generating random arrival 
 * dates (using an exponential distribution in this case) 
 */
void httpRequest_setLambda(struct httpRequest_t *Request, double lambda);
/**
 * @brief The function changes the server's parameter - debit
 * @param Request The Request on which we make modifications
 * @param debit The new value for the server's parameter
 */
void httpRequest_setdebit(struct httpRequest_t *Request, double debit);
/**
 * @brief The function changes the parameters of the random size generator
 * @param Request The Request on which we make modifications
 * @param alpha The alpha parameter of the Lognormal distribution used for the
 * random size generator
 * @param beta The beta parameter of the Lognormal distribution used for the
 * random size generator
 */
void httpRequest_setSizeGenParam(struct httpRequest_t *Request, double alpha, double beta);


/*==========================================================================*/
/*   The functions used for setting reply's parameters                      */
/*==========================================================================*/

/**
 * @brief The function changes the server's parameter - debit
 * @param Reply The Reply on which we make modifications
 * @param debit The new value for the server's parameter
 */
void httpReply_setdebit(struct httpReply_t *Reply, double debit);
/**
 * @brief The function changes the parameters of the random date generator
 * @param Reply The Reply on which we make modifications
 * @param alpha The alpha parameter of the Weibull distribution used for the
 * random date generator
 * @param beta The beta parameter of the Weibull distribution used for the
 * random date generator
 */
void httpReply_setDateGenParam(struct httpReply_t *Reply, double alpha, double beta);
/**
 * @brief The function changes the parameters of the random main object size generator
 * @param Reply The Reply on which we make modifications
 * @param alpha The alpha parameter of the Lognormal distribution used for the
 * random main object size generator
 * @param beta The beta parameter of the Lognormal distribution used for the
 * random main object size generator
 */
void httpReply_setMainSizeGenParam(struct httpReply_t *Reply, double alpha, double beta);
/**
 * @brief The function changes the parameters of the random inline object size generator
 * @param Reply The Reply on which we make modifications
 * @param alpha The alpha parameter of the Lognormal distribution used for the
 * random inline object size generator
 * @param beta The beta parameter of the Lognormal distribution used for the
 * random inline object size generator
 */
void httpReply_setInlineSizeGenParam(struct httpReply_t *Reply, double alpha, double beta);
/**
 * @brief The function changes the parameters of the random inline object number generator
 * @param Reply The Reply on which we make modifications
 * @param alpha The alpha parameter of the Gamma distribution used for the
 * random inline object number generator
 * @param beta The beta parameter of the Gamma distribution used for the
 * random inline object number generator
 */
void httpReply_setInlineNbGenParam(struct httpReply_t *Reply, double alpha, double beta);

/*==========================================================================*/
/*          The functions used to return request's parameters               */
/*==========================================================================*/

/**
 * @brief The function returns the Request's date generator
 * @param Request A pointer of the Request 
 */
struct dateGenerator_t* httpRequest_GetDateGen(struct httpRequest_t *Request);
/**
 * @brief The function returns the Request's size generator
 * @param Request A pointer of the Request 
 */
struct randomGenerator_t* httpRequest_GetSizeGen(struct httpRequest_t *Request);
/**
 * @brief The function returns the Request's filePDU parameter
 * @param Request A pointer of the Request 
 */
struct filePDU_t* httpRequest_GetFilePDU(struct httpRequest_t *Request);
/**
 * @brief The function returns the Request's server parameter
 * @param Request A pointer of the Request 
 */
struct srvGen_t* httpRequest_GetServer(struct httpRequest_t *Request);
/**
 * @brief The function returns the Request's PDUSource
 * @param Request A pointer of the Request 
 */
struct PDUSource_t* httpRequest_GetPDUSource(struct httpRequest_t *Request);

/**
 * @brief The function returns the debit of the server's request
 * @param Request A pointer of the Request 
 */
double httpRequest_getDebit(struct httpRequest_t *Request);
/*
 *  Functions for returning the parameters of the size generator
 */

/**
 * @brief Returns the alpha parameter of the request's size generator
 * (Lognormal distribution) 
 * @param Request A pointer of the Request
 */
double httpRequest_getalpha(struct httpRequest_t *Request);
/**
 * @brief Returns the beta parameter of the request's size generator
 * (Lognormal distribution) 
 * @param Request A pointer of the Request
 */
double httpRequest_getbeta(struct httpRequest_t *Request);
/*
 * @brief The function returns the parameter of the request's date generator
 * @param Request A pointer of the Request
 */
double httpRequest_getLambda(struct httpRequest_t *Request);

/*==========================================================================*/
/*          The functions used to return reply's parameters                 */
/*==========================================================================*/

/**
 * @brief The function returns the Reply's date generator
 * @param Reply A pointer of the Reply 
 */
struct dateGenerator_t* httpReply_GetDateGen(struct httpReply_t *Reply);
/**
 * @brief The function returns the Reply's size generator
 * @param Reply A pointer of the Reply 
 */
struct randomGenerator_t* httpReply_GetSizeGen(struct httpReply_t *Reply);
/**
 * @brief The function returns the Reply's filePDU parameter
 * @param Reply A pointer of the Reply 
 */
struct filePDU_t* httpReply_GetFilePDU(struct httpReply_t *Reply);
/**
 * @brief The function returns the Reply's server parameter
 * @param Reply A pointer of the Reply 
 */
struct srvGen_t* httpReply_GetServer(struct httpReply_t *Reply);
/**
 * @brief The function returns the Reply's PDUSource
 * @param Reply A pointer of the Reply 
 */
struct PDUSource_t* httpReply_GetPDUSource(struct httpReply_t *Reply);
/*
 * @brief The function returns the debit of the server's reply
 * @param Reply A pointer of the Reply 
 */
double httpReply_getDebit(struct httpReply_t *Reply);
/*
 *   Functions for returning the parameters of the date generator
 */

/**
 * @brief Returns the alpha parameter of the reply's date generator
 * (Weibull distribution) 
 * @param Reply A pointer of the Reply
 */
double httpReply_getdWalpha(struct httpReply_t *Reply);
/**
 * @brief Returns the beta parameter of the reply's date generator
 * (Weibull distribution) 
 * @param Reply A pointer of the Reply
 */
double httpReply_getdWbeta(struct httpReply_t *Reply);
/*
 *   Functions for returning the parameters of the main object size generator
 */

/**
 * @brief Returns the alpha parameter of the reply's main object random size generator
 * (Lognormal distribution) 
 * @param Reply A pointer of the Reply
 */
double httpReply_getmLnalpha(struct httpReply_t *Reply);
/**
 * @brief Returns the beta parameter of the reply's main object random size generator
 * (Lognormal distribution) 
 * @param Reply A pointer of the Reply
 */
double httpReply_getmLnbeta(struct httpReply_t *Reply);
/*
 *   Functions for returning the parameters of the inline object size generator
 */
/**
 * @brief Returns the alpha parameter of the reply's inline object random size generator
 * (Lognormal distribution) 
 * @param Reply A pointer of the Reply
 */
double httpReply_getiLnalpha(struct httpReply_t *Reply);
/**
 * @brief Returns the beta parameter of the reply's inline object random size generator
 * (Lognormal distribution) 
 * @param Reply A pointer of the Reply
 */
double httpReply_getiLnbeta(struct httpReply_t *Reply);
/*
 *   Functions for returning the parameters of the inline object number generator
 */

/**
 * @brief Returns the alpha parameter of the reply's inline object random number generator
 * (Gamma distribution) 
 * @param Reply A pointer of the Reply
 */
double httpReply_getiGmalpha(struct httpReply_t *Reply);
/**
 * @brief Returns the beta parameter of the reply's inline object random number generator
 * (Gamma distribution) 
 * @param Reply A pointer of the Reply
 */
double httpReply_getiGmbeta(struct httpReply_t *Reply);















