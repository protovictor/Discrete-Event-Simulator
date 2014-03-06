/*----------------------------------------------------------------------*/
/*   Utilisation de gnuplot pour afficher des sondes.                   */
/*----------------------------------------------------------------------*/
#ifndef __DEF_NDES_GPOUT
#define __DEF_NDES_GPOUT

#include <probe.h>
#include <stdarg.h>

/**
 * @brief A gnuplot_t structure is needed to draw some probes
 */
struct gnuplot_t;

/**
 * @brief Gnupot terminal  used
 */
#define gnuplotTerminalTypeWxt 1
#define gnuplotTerminalTypePng 2

/* 
 * Creator
 */
struct gnuplot_t * gnuplot_create();

/**
 * @brief Destructor
 */
void gnuplot_delete(struct gnuplot_t * gp);

/**
 * @brief Select terminal type
 * @param gp The gnuplot terminal 
 * @param terminalType The new terminal type
 */
void gnuplot_setTerminalType(struct gnuplot_t * gp, int terminalType);

/**
 * @brief Change the output file name
 * @param gp The gnuplot terminal 
 * @param name The new file name
 */
void gnuplot_setOutputFileName(struct gnuplot_t * gp, char * outputFileName);

/**
 * @brief Change the title
 * @param gp The gnuplot terminal 
 * @param name The new title
 */
void gnuplot_setTitle(struct gnuplot_t * gp, char * name);

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

#endif
