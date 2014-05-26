#include <stdio.h>
#include <stdlib.h>
#include "ftp.h"
#include "pdu-sink.h"

int main()
{
    struct probe_t             *isProbe, *bsProbe, *sejProbe;
    struct FTP_Session_t       *Transfer;
    struct randomGenerator_t   *fileSize;
    struct dateGenerator_t     *readingTime;

    struct PDUSink_t           *sink;
    struct filePDU_t           *filePDU;
    struct srvGen_t            *base_station;

    double mean = 50;
    double duration = 100000.0;

    motSim_create();
    sink = PDUSink_create();

    base_station = srvGen_create(sink, (processPDU_t)PDUSink_processPDU );
    srvGen_setServiceTime (base_station, serviceTimeExp, mean);

    filePDU = filePDU_create(base_station, (processPDU_t)srvGen_processPDU);

    fileSize = randomGenerator_createDouble();
    randomGenerator_setDistributionLognormal(fileSize, 14.45, 0.35);
    readingTime = dateGenerator_createExp(0.006);

    Transfer = FTP_Session_Create(readingTime, fileSize, filePDU, (processPDU_t) filePDU_processPDU );


  /*==========================================*/
  /*                                          */
  /*       Some sensors for the simulation    */
  /*                                          */
  /*==========================================*/

    isProbe = probe_createExhaustive();
    dateGenerator_addInterArrivalProbe(readingTime, isProbe);

    bsProbe = probe_createExhaustive();
    srvGen_addServiceProbe(base_station, bsProbe);

    sejProbe = probe_createExhaustive();
    filePDU_addSejournProbe(filePDU, sejProbe);


 /*--------------------------------------------------------------------*/

    FTP_Session_Start(Transfer);

    motSim_runUntil(duration);
    motSim_printStatus();

    printf("Number of FTP sessions : %d \n", FTP_GetSessionNr(Transfer));
    printf("Average reading time: %f ms \n", probe_mean(isProbe));
    printf("Base station serving time: %f \n", probe_mean(bsProbe));
    printf("Average journey time: %f \n", probe_mean(sejProbe));



 return 0;
}

