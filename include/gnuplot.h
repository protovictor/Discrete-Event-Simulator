/*----------------------------------------------------------------------*/
/*   Utilisation de gnuplot pour afficher des sondes.                   */
/*----------------------------------------------------------------------*/
#ifndef __DEF_NDES_GPOUT
#define __DEF_NDES_GPOUT

#include <probe.h>
#include <stdarg.h>


#ifndef GnuPlotCmd
#   define GnuPlotCmd "/usr/bin/gnuplot"
#endif
#define GPSetTermWxt "set terminal wxt"

struct gnuplot_t;

/* 
 * Creator
 */
struct gnuplot_t * gnuplot_create();

/* 
 * Destructor
 */
void gnuplot_delete(struct gnuplot_t * gp);

/*
 * Affichage dans une fenetre d'une sonde 
 */
int gnuplot_displayProbe(struct gnuplot_t * gp, int with, struct probe_t * probe);

#define WITH_BOXES   0
#define WITH_POINTS  1
#define WITH_LINES   2

int gnuplot_displayProbes(struct gnuplot_t * gp, int with, ...);

/*
 * Setting ranges
 */
void gnuplot_setXRange(struct gnuplot_t * gp, double xmin, double xmax);
void gnuplot_setYRange(struct gnuplot_t * gp, double ymin, double ymax);

void gnuplot_setTitle(struct gnuplot_t * gp, char * name);

#endif
