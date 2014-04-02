#include <stdio.h>
#include <stdlib.h>
#include <httpmodel.h>
#include <ndesObject.h>
#include <motsim.h>


struct httpSim_t {
  declareAsNdesObject;

  struct filePDU_t         *filePDU;
  struct dateGenerator_t   *dateGen;
  struct srvGen_t          *server;
  struct randomGenerator_t *sizeGen;
  struct PDUSource_t       *Source;
};


defineObjectFunctions(httpSim);
struct ndesObjectType_t httpSimType = {
  ndesObjectTypeDefaultValues(httpSim)
};


 struct httpSim_t* httpSim_CreateRequest(struct PDUSink_t* sink)
{
    struct httpSim_t* Request = (struct httpSim_t*)sim_malloc(sizeof(struct httpSim_t));
  
    // Init the parameters

    double lambda = 1;
    double debit = 1000.0;
  // parameters for Lognormal distribution - sizeGen
    double alpha = 5.84;
    double beta = 0.29;    


    Request->server = srvGen_create(sink, (processPDU_t)PDUSink_processPDU);
    srvGen_setServiceTime(Request->server, serviceTimeProp, 1.0/debit); 

    Request->filePDU = filePDU_create(Request->server, (processPDU_t)srvGen_processPDU); 
    Request->dateGen = dateGenerator_createExp(lambda);

    Request->sizeGen = randomGenerator_createDouble();
    randomGenerator_setDistributionLognormal(Request->sizeGen, alpha, beta);
      
    Request->Source = PDUSource_create(Request->dateGen, Request->filePDU, (processPDU_t)filePDU_processPDU); 
    PDUSource_setPDUSizeGenerator(Request->Source, Request->sizeGen); 

 return Request;
}
 
 struct PDUSouce_t* httpSim_GetPDUSource(struct httpSim_t *S)
{
   return S->Source;
}
 
 struct filePDU_t* httpSim_GetFilePDU(struct httpSim_t *S)
{
   return S->filePDU;
} 

 struct srvGen_t* httpSim_GetServer(struct httpSim_t *S)
{
   return S->server;
}

 struct dateGenerator_t* httpSim_GetDateGen(struct httpSim_t *S)
{
   return S->dateGen;
}


 struct httpSim_t* httpSim_CreateReply(struct PDUSink_t* sink)
{
    struct httpSim_t* Reply = ( struct httpSim_t*)sim_malloc(sizeof( struct httpSim_t));

    double debit = 1000.0;          
 
    double walpha = 0.5;
    double wbeta = 4.44;
  
    double a = 1.31;
    double b = 1.41;
  
    double ain = -0.75;
    double bin = 2.36;
 
    double galpha = 0.24;
    double gbeta = 23.42;


    Reply->server = srvGen_create(sink, (processPDU_t)PDUSink_processPDU);
    srvGen_setServiceTime(Reply->server, serviceTimeProp, 1.0/debit);
    Reply->filePDU = filePDU_create(Reply->server, (processPDU_t)srvGen_processPDU); 
       
  // The recommended model uses for interarrival time a Weibull distribution
     Reply->dateGen = dateGenerator_createWeibull(walpha, wbeta);

     Reply->sizeGen = randomGenerator_createDouble();
     randomGenerator_setDistributionComposed(Reply->sizeGen, a, b, ain, bin, galpha, gbeta);
  // To do --- the replied page doesn't load sinchronously
  //   the main object loads first and the inline objects after 
 
     Reply->Source = PDUSource_create(Reply->dateGen, Reply->filePDU, (processPDU_t)filePDU_processPDU);  
  
  // We associate the size of the request packet to the reqPDU 
     PDUSource_setPDUSizeGenerator(Reply->Source, Reply->sizeGen);

 return Reply;
}





