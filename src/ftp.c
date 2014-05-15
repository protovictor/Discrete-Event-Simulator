#include <stdio.h>
#include <stdlib.h>
#include <probe.h>
#include <ftp.h>

/*=========================================*/
/*    This structure contains the          */
/*  parameters of the FTP traffic model    */
/*=========================================*/


struct FTP_t{
   declareAsNdesObject;

   struct {
      double alpha;
      double beta;
   }tLn;                                       //!< truncated Longormal distribution parameters - for fileSize
  
   double lambda;                              //!< lambda parameter for exponential distribution

   struct randomGenerator_t    *fileSize;      //!< Truncated Lognormal distribution
   struct dateGenerator_t      *readingTime;   //!< Exponential distribution

   struct filePDU_t            *filePDU;
   struct srvGen_t             *server;
   struct PDUSource_t          *Source;   
};


defineObjectFunctions(FTP);
struct ndesObjectType_t FTPType = {
  ndesObjectTypeDefaultValues(FTP)
};

struct FTP_t * FTP_CreateFileTransfer()
{
   struct FTP_t * newFileTransfer = (struct FTP_t *)sim_malloc(sizeof(struct FTP_t));

   newFileTransfer->tLn.alpha = 14.45;
   newFileTransfer->tLn.beta = 0.35;
   newFileTransfer->lambda = 0.006;

   newFileTransfer->fileSize = NULL;
   newFileTransfer->readingTime = NULL;

   newFileTransfer->filePDU = NULL;
   newFileTransfer->server = NULL;
   newFileTransfer->Source = NULL;
   

 return newFileTransfer;
}


void FTP_LoadParameters(struct FTP_t *fileTransfer, struct PDUSink_t * sink)
{
   fileTransfer->server = srvGen_create(sink, (processPDU_t)PDUSink_processPDU);
   srvGen_setServiceTime(fileTransfer->server, serviceTimeExp, 1.0/fileTransfer->lambda); 
   
   fileTransfer->fileSize = randomGenerator_createDouble();
   randomGenerator_setDistributionLognormal(fileTransfer->fileSize, fileTransfer->tLn.alpha, 
                                                                    fileTransfer->tLn.beta);
   fileTransfer->readingTime = dateGenerator_createExp(fileTransfer->lambda);


   fileTransfer->filePDU = filePDU_create(fileTransfer->server, (processPDU_t)srvGen_processPDU); 
   
   fileTransfer->Source = PDUSource_create(fileTransfer->readingTime, fileTransfer->filePDU, (processPDU_t)filePDU_processPDU); 
   PDUSource_setPDUSizeGenerator(fileTransfer->Source, fileTransfer->fileSize); 

}

/*===================================================*/
/*        These functions are used to set the        */
/*        parameters for the ftp traffic model       */
/*===================================================*/

void FTP_setFileSizeParams(struct FTP_t *fileTransfer, double alpha, double beta)
{
     fileTransfer->tLn.alpha = alpha;
     fileTransfer->tLn.beta = beta;
} 

void FTP_setReadingTimeParam(struct FTP_t *fileTransfer, double lambda)
{
     fileTransfer->lambda = lambda;
}


/*===================================================*/
/*        These functions are used to get the        */
/*        parameters for the ftp traffic model       */
/*===================================================*/
 

struct randomGenerator_t * FTP_getFileSizeGen(struct FTP_t *fileTransfer)
{
     return fileTransfer->fileSize;
}

double FTP_getFileSizeGen_alpha(struct FTP_t *fileTransfer)
{
     return fileTransfer->tLn.alpha;
}

double FTP_getFileSizeGen_beta(struct FTP_t *fileTransfer)
{
     return fileTransfer->tLn.beta;
}

struct dateGeneratot_t * FTP_getReadingTimeGen(struct FTP_t *fileTransfer)
{
     return fileTransfer->readingTime;
}

double FTP_getReadingTimeGen_lambda(struct FTP_t *fileTransfer)
{
     return fileTransfer->lambda;
}

struct filePDU_t * FTP_getFilePDU(struct FTP_t *fileTransfer)
{
     return fileTransfer->filePDU;
}

struct srvGen_t *FTP_getServer(struct FTP_t *fileTransfer)
{
     return fileTransfer->server;
}

struct PDUSource_t * FTP_getPDUSource(struct FTP_t *fileTransfer)
{
     return fileTransfer->Source;
}



