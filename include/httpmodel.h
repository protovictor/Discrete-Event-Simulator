#include <pdu-source.h>
#include <file_pdu.h>
#include <date-generator.h>
#include <random-generator.h>
#include <srv-gen.h>
#include <pdu-sink.h>


struct httpSim_t;

struct httpSim_t* httpSim_CreateRequest(struct PDUSink_t* sink);
struct httpSim_t* httpSim_CreateReply(struct PDUSink_t* sink);

struct PDUSouce_t* httpSim_GetPDUSource(struct httpSim_t *S);
struct filePDU_t* httpSim_GetFilePDU(struct httpSim_t *S);
struct srvGen_t* httpSim_GetServer(struct httpSim_t *S);
struct dateGenerator_t* httpSim_GetDateGen(struct httpSim_t *S);

