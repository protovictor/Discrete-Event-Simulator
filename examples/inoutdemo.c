/**
 * @file inoutdemo.c
 * @brief Simple examples to explain the basic in/out model behaviour
 */
#include <motsim.h>
#include <pdu-source.h>
#include <ll-simplex.h>
#include <pdu-sink.h>

int main()
{
   double throughput = 1000.00; // 1Kbit/s
   double propagation = .001;    // 1 ms 
   struct dateSize sequence[] = {{0.0, 1000.0}, {0.5, 2000.0}, {1.0, 2000.0}, {0.0, 0.0}};
   struct PDUSource_t * src;
   struct llSimplex_t * ll;
   struct PDUSink_t   * snk;

   motSim_create();
   printf("a\n");
   snk = PDUSink_create();
   printf("a\n");
   ll = llSimplex_create(snk, PDUSink_processPDU, throughput, propagation);
   printf("a\n");
   src = PDUSource_createDeterministic(sequence, ll, llSimplex_processPDU);
   printf("a\n");
   PDUSource_start(src);
   motSim_runUntilTheEnd();

   motSim_printStatus();
   printf("Fini !\n");

   return 1;

}
