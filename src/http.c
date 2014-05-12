/*
 *   The implementation of a simple HTTP model suggested
 *   in the document "Source Traffic Modelling of Wireless Applications". 
 */


#include <stdio.h>
#include <stdlib.h>
#include <http.h>
#include <ndesObject.h>
#include <motsim.h>

/*
 *  General structure of a http Request
 */

struct httpRequest_t {
  declareAsNdesObject;
  
   struct
   {  
     double lambda;        //!< Date Generator parameter (for exponential distribution)
     double debit;         //!< Server's parameter (bits/second)
     struct paramLogn_t    //!< Lognormal distribution's parameters(for size generator)
     {
        double alpha;
        double beta;
     }Lnd;
   }Param;
 
  struct filePDU_t         *filePDU;
  struct dateGenerator_t   *dateGen;
  struct srvGen_t          *server;
  struct randomGenerator_t *sizeGen;
  struct PDUSource_t       *Source;

};


defineObjectFunctions(httpRequest);
struct ndesObjectType_t httpRequestType = {
  ndesObjectTypeDefaultValues(httpRequest)
};


/*
 *  General structure of a http Reply
 */

struct httpReply_t {
  declareAsNdesObject;

   struct
   { 
     double debit;    //!< The server parameter (bits/second)
     struct           //!< Weibull distribution's parameters - for date generator
     {
       double alpha;
       double beta;
     }dW;
     struct           //!< Lognormal distribution's parameters - for webpage main object size
     {
       double alpha;
       double beta;
     }mLn; 
     struct           //!< Lognormal distribution's parameters - for webpage inline object size
     {
       double alpha;
       double beta;
     }iLn;
     struct           //!< Gamma distribution's parameters - for inline object number
     {
       double alpha;
       double beta;
     }iGm;

   }Param;


  struct filePDU_t         *filePDU;
  struct dateGenerator_t   *dateGen;
  struct srvGen_t          *server;
  struct randomGenerator_t *sizeGen;
  struct PDUSource_t       *Source;
};

defineObjectFunctions(httpReply);
struct ndesObjectType_t httpReplyType = {
  ndesObjectTypeDefaultValues(httpReply)
};

/*==========================================================================*/
/*       The functions used for creating default request and reply          */
/*==========================================================================*/



/*
 * Create a Request with default parameters
 */
 struct httpRequest_t* http_CreateRequest()
{
    struct httpRequest_t* Request = (struct httpRequest_t*)sim_malloc(sizeof(struct httpRequest_t));

    Request->Param.lambda = 1;
    Request->Param.debit = 1000.0;
  // parameters for Lognormal distribution - sizeGen
    Request->Param.Lnd.alpha = 5.84;      // the recommended value
    Request->Param.Lnd.beta = 0.29;       // the recommended value
  // We will create the random generators later, for now we will put NULL
    Request->dateGen = NULL;
    Request->sizeGen = NULL;
    Request->server = NULL;
    Request->filePDU = NULL;    
    Request->Source = NULL;    

 return Request;
}

/* 
 * Create a Reply with default parameters
 */
struct httpReply_t* http_CreateReply()
{
    struct httpReply_t* Reply = ( struct httpReply_t*)sim_malloc(sizeof(struct httpReply_t));
    
    Reply->Param.debit = 1000.0;          
    // Default parameters for Weibull distribution 
    // used for arrival date generator
    Reply->Param.dW.alpha = 0.5;
    Reply->Param.dW.beta = 4.44;
    // Default parameters for Lognormal distribution
    // used for main object size generator
    Reply->Param.mLn.alpha = 1.31;
    Reply->Param.mLn.beta = 1.41;
    // Default parameters for Lognormal distribution
    // used for inline object size generator
    Reply->Param.iLn.alpha = -0.75;
    Reply->Param.iLn.beta = 2.36;
    // Default parameters for Gamma distribution
    // used for inline object number generator
    Reply->Param.iGm.alpha = 0.24;
    Reply->Param.iGm.beta = 23.42;
    // We will create the random generators later, for now we will put NULL
    Reply->dateGen = NULL;
    Reply->sizeGen = NULL;
    Reply->server = NULL;
    Reply->filePDU = NULL;    
    Reply->Source = NULL;   

 return Reply;
}


/*==========================================================================*/
/*   The functions used for loading request/reply 's parameters into the    */
/*                             random generators                            */
/*==========================================================================*/


/*
 *   Request load parameter function (created separately for non-default parameter case 
 */

void httpRequest_LoadParameters(struct httpRequest_t *Request, struct PDUSink_t * sink)
{

    Request->server = srvGen_create(sink, (processPDU_t)PDUSink_processPDU);
    srvGen_setServiceTime(Request->server, serviceTimeProp, 1.0/Request->Param.debit); 

    Request->filePDU = filePDU_create(Request->server, (processPDU_t)srvGen_processPDU); 
    Request->dateGen = dateGenerator_createExp(Request->Param.lambda);

    Request->sizeGen = randomGenerator_createDouble();
    randomGenerator_setDistributionLognormal(Request->sizeGen, Request->Param.Lnd.alpha, 
                                                               Request->Param.Lnd.beta);
   
    Request->Source = PDUSource_create(Request->dateGen, Request->filePDU, (processPDU_t)filePDU_processPDU); 
    PDUSource_setPDUSizeGenerator(Request->Source, Request->sizeGen); 

}

/*
 *   Reply load parameter function (created separately for non-default parameter case 
 */


void httpReply_LoadParameters(struct httpReply_t *Reply, struct PDUSink_t *sink)
{

    Reply->server = srvGen_create(sink, (processPDU_t)PDUSink_processPDU);
    srvGen_setServiceTime(Reply->server, serviceTimeProp, 1.0/Reply->Param.debit);
    Reply->filePDU = filePDU_create(Reply->server, (processPDU_t)srvGen_processPDU); 
       
  // The recommended model uses for interarrival time a Weibull distribution
     Reply->dateGen = dateGenerator_createWeibull(Reply->Param.dW.alpha, 
                                                  Reply->Param.dW.beta);

     Reply->sizeGen = randomGenerator_createDouble();
     randomGenerator_setDistributionComposed(Reply->sizeGen, 
                                             Reply->Param.mLn.alpha, Reply->Param.mLn.beta,
                                             Reply->Param.iLn.alpha, Reply->Param.iLn.beta, 
                                             Reply->Param.iGm.alpha, Reply->Param.iGm.beta); 
 
     Reply->Source = PDUSource_create(Reply->dateGen, Reply->filePDU, (processPDU_t)filePDU_processPDU);  
  
  // We associate the size of the request packet to the reqPDU 
     PDUSource_setPDUSizeGenerator(Reply->Source, Reply->sizeGen);
}


/*==========================================================================*/
/*   The functions used for setting request's parameters                    */
/*==========================================================================*/


void httpRequest_setLambda(struct httpRequest_t *Request, double lambda)
{
    Request->Param.lambda = lambda;
    Request->dateGen = dateGenerator_createExp(Request->Param.lambda);
}

void httpRequest_setdebit(struct httpRequest_t *Request, double debit)
{
    Request->Param.debit = debit;
}

void httpRequest_setSizeGenParam(struct httpRequest_t *Request, double alpha, double beta)
{
    Request->Param.Lnd.alpha = alpha;
    Request->Param.Lnd.beta = beta;

}

/*==========================================================================*/
/*   The functions used for setting reply's parameters                      */
/*==========================================================================*/

void httpReply_setdebit(struct httpReply_t *Reply, double debit)
{
  Reply->Param.debit = debit;
}

void httpReply_setDateGenParam(struct httpReply_t *Reply, double alpha, double beta)
{
  Reply->Param.dW.alpha = alpha;
  Reply->Param.dW.beta = beta;
}

void httpReply_setMainSizeGenParam(struct httpReply_t *Reply, double alpha, double beta)
{
  Reply->Param.mLn.alpha = alpha;
  Reply->Param.mLn.beta = beta;
}

void httpReply_setInlineSizeGenParam(struct httpReply_t *Reply, double alpha, double beta)
{
  Reply->Param.iLn.alpha = alpha;
  Reply->Param.iLn.beta = beta;
}

void httpReply_setInlineNbGenParam(struct httpReply_t *Reply, double alpha, double beta)
{
  Reply->Param.iGm.alpha = alpha;
  Reply->Param.iGm.beta = beta;
}

/*==========================================================================*/
/*          The functions used to return request's parameters               */
/*==========================================================================*/

struct dateGenerator_t* httpRequest_GetDateGen(struct httpRequest_t *Request)
{
  return Request->dateGen;
}

struct randomGenerator_t* httpRequest_GetSizeGen(struct httpRequest_t *Request)
{
  return Request->sizeGen;
}

struct filePDU_t* httpRequest_GetFilePDU(struct httpRequest_t *Request)
{
  return Request->filePDU;
}

struct srvGen_t* httpRequest_GetServer(struct httpRequest_t *Request)
{
  return Request->server;
}

struct PDUSource_t* httpRequest_GetPDUSource(struct httpRequest_t *Request)
{
  return Request->Source;
}

/*
 *   Returns the debit of the server's request
 */

double httpRequest_getDebit(struct httpRequest_t *Request)
{
  return Request->Param.debit;
}

/*
 *   Return the parameter for the date generator
 */

double httpRequest_getLambda(struct httpRequest_t *Request)
{
  return Request->Param.lambda;
}

/*
 *   Functions for returning the parameters of the size generator
 */

double httpRequest_getalpha(struct httpRequest_t *Request)
{
  return Request->Param.Lnd.alpha;
} 

double httpRequest_getbeta(struct httpRequest_t *Request)
{
  return Request->Param.Lnd.beta;
}

/*==========================================================================*/
/*          The functions used to return reply's parameters                 */
/*==========================================================================*/

struct dateGenerator_t* httpReply_GetDateGen(struct httpReply_t *Reply)
{
  return Reply->dateGen;
}

struct randomGenerator_t* httpReply_GetSizeGen(struct httpReply_t *Reply)
{
  return Reply->sizeGen;
}

struct filePDU_t* httpReply_GetFilePDU(struct httpReply_t *Reply)
{
  return Reply->filePDU;
}

struct srvGen_t* httpReply_GetServer(struct httpReply_t *Reply)
{
  return Reply->server;
}

struct PDUSource_t* httpReply_GetPDUSource(struct httpReply_t *Reply)
{
  return Reply->Source;
}

/*
 *   Returns the debit of the server's reply
 */

double httpReply_getDebit(struct httpReply_t *Reply)
{
  return Reply->Param.debit;
}
 
/*
 *   Functions for returning the parameters of the date generator
 */

double httpReply_getdWalpha(struct httpReply_t *Reply)
{
  return Reply->Param.dW.alpha;
}

double httpReply_getdWbeta(struct httpReply_t *Reply)
{
  return Reply->Param.dW.beta;
}

/*
 *   Functions for returning the parameters of the main object size generator
 */

double httpReply_getmLnalpha(struct httpReply_t *Reply)
{
  return Reply->Param.mLn.alpha;
}

double httpReply_getmLnbeta(struct httpReply_t *Reply)
{
  return Reply->Param.mLn.beta;
}

/*
 *   Functions for returning the parameters of the inline object size generator
 */

double httpReply_getiLnalpha(struct httpReply_t *Reply)
{
  return Reply->Param.iLn.alpha;
}

double httpReply_getiLnbeta(struct httpReply_t *Reply)
{
  return Reply->Param.iLn.beta;
}

/*
 *   Functions for returning the parameters of the inline object number generator
 */

double httpReply_getiGmalpha(struct httpReply_t *Reply)
{
  return Reply->Param.iGm.alpha;
}

double httpReply_getiGmbeta(struct httpReply_t *Reply)
{
  return Reply->Param.iGm.beta;
}






















