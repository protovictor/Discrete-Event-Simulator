/**
 *
 */
#include <motsim.h>
#include <hdlc.h>
#include <ll-simplex.h>

int main()
{
   struct hdlc_t * client, * server;
   struct llSimplex_t *c2s, *s2c;

   motSim_create();

   server = hdlc_create();
   client = hdlc_create();

   c2s = llSimplex_create(server,
			  hdlc_processPDU,
			  64000, 0.01);
   s2c = llSimplex_create(client,
			  hdlc_processPDU,
			  64000, 0.01);
   hdlc_setOutLink(server, s2c, llSimplex_processPDU);
   hdlc_setOutLink(client, c2s, llSimplex_processPDU);

   hdlc_init(server);

   hdlc_connect(client, 0);

   //   hdlc_send();
   motSim_runUntilTheEnd();

   return 0;
}
