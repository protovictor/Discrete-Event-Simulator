#include <pdu-source.h>
#include <file_pdu.h>
#include <date-generator.h>
#include <random-generator.h>
#include <srv-gen.h>
#include <pdu-sink.h>


 /* We declare the structure for a file transfer */
struct FTP_t;

/**
 * @brief The function is used for creating a file
 * transfer with the default parameters
 *
 */
struct FTP_t * FTP_CreateFileTransfer();

/**
 * @brief The function loads the parameters into
 * the file transfer structure
 * @param fileTransfer a pointer to the file transfer structure 
 * @param sink a pointer to the sink
 */
void FTP_LoadParameters(struct FTP_t *fileTransfer, struct PDUSink_t * sink);


/*===================================================*/
/*        These functions are used to set the        */
/*        parameters for the ftp traffic model       */
/*===================================================*/

/**
 * @brief The function sets new parameters to the fileSize
 * generator
 * @param fileTransfer a pointer to the file transfer structure 
 * @param alpha the new value for the alpha parameter of Lognormal distribution
 * @param beta the new value for the beta parameter of Lognormal distribution
 */
void FTP_setFileSizeParams(struct FTP_t *fileTransfer, double alpha, double beta);

/**
 * @brief The function sets the reading time parameter
 * @param fileTransfer a pointer to the file transfer structure 
 * @param lambda the new value for the lambda paramater of Exponential distribution
 */
void FTP_setReadingTimeParam(struct FTP_t *fileTransfer, double lambda);

/*===================================================*/
/*        These functions are used to get the        */
/*        parameters for the ftp traffic model       */
/*===================================================*/

/**
 * @brief The function returns the file size generator
 * @param fileTransfer a pointer to the file transfer structure
 */
struct randomGenerator_t*  FTP_getFileSizeGen(struct FTP_t *fileTransfer);

/**
 * @brief The function returns the value of the alpha parameter
 * @param fileTransfer a pointer to the file transfer structure
 */
double FTP_getFileSizeGen_alpha(struct FTP_t *fileTransfer);

/**
 * @brief The function returns the value of the beta parameter
 * @param fileTransfer a pointer to the file transfer structure
 */
double FTP_getFileSizeGen_beta(struct FTP_t *fileTransfer);

/**
 * @brief The function returns the reading time generator
 * @param fileTransfer a pointer to the file transfer structure
 */
struct dateGeneratot_t*    FTP_getReadingTimeGen(struct FTP_t *fileTransfer);

/**
 * @brief The function returns the value of the lambda parameter
 * @param fileTransfer a pointer to the file transfer structure
 */
double FTP_getReadingTimeGen_lambda(struct FTP_t *fileTransfer);

/**
 * @brief The function returns the file pdu which holds all the
 * PDUs that will be transmitted through a file transfer
 * @param fileTransfer a pointer to the file transfer structure
 */
struct filePDU_t*   FTP_getFilePDU(struct FTP_t *fileTransfer);

/**
 * @brief The function returns the server which will receive the
 * PDUs in a file transfer
 * @param fileTransfer a pointer to the file transfer structure
 */
struct srvGen_t* FTP_getServer(struct FTP_t *fileTransfer);

/**
 * @brief The function returns the source of PDUs which will be
 * transmitted
 * @param fileTransfer a pointer to the file transfer structure 
 */
struct PDUSource_t* FTP_getPDUSource(struct FTP_t *fileTransfer);



