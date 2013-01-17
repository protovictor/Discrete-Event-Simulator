#include <stdlib.h>     // exit
#include <unistd.h>     // pipe, unlink
#include <string.h>     // strlen, strdup
#include <stdio.h>      // perror
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <motsim.h>
#include <gnuplot.h>

/*
 * Multiple source fils can be associated to a given
 * terminal, so we need to chain the names
 */
struct fileName_t {
   char * fileName;
   struct fileName_t * next;
};

struct gnuplot_t {
   // Ident of gnuplot terminal
   int    id;

   // Input file names daisy chain
   struct fileName_t * srcFileName;

   // Gnuplot specific
   char * title ;
   double xmin, xmax;
   double ymin, ymax;

   // Terminal daisy chaining
   struct gnuplot_t * prev;
   struct gnuplot_t * next; 
};

struct gnuplotProcess_t {
   int stdin;  // stdin du processus
   int stdout; // stdout+stderr
   int pid;

   int nbPlot;
   struct gnuplot_t * lastTerminal;
};

struct gnuplotProcess_t * gnuplotProcess = NULL; 

void GPSendCmd(struct gnuplot_t * gp, char * cmd)
{
   char buffer[512];
   //   int readBytes;
   
   unsigned char cr = 0x0a;

   printf_debug(DEBUG_GNUPLOT, "GPSendCmd IN\n");
/*
   while ((readBytes = read(gnuplotProcess->stdout, buffer, 512)) > 0) {
      write(STDOUT_FILENO, buffer, readBytes);
   }
   printf_debug(DEBUG_GNUPLOT, "GPSendCmd BBB\n");
*/

   if (gp) {
      printf_debug(DEBUG_GNUPLOT, "Selecting terminal (%s)...\n", (gp->title)?gp->title:"Guess my name");
      sprintf(buffer, "%s %d title '%s'",
              GPSetTermWxt,
              gp->id,
	      (gp->title)?gp->title:"Guess my name");
      printf_debug(DEBUG_GNUPLOT, "sending \"%s\"\n", buffer);
      write(gnuplotProcess->stdin, buffer, strlen(buffer));
      write(gnuplotProcess->stdin, &cr, 1);
   }

   printf_debug(DEBUG_GNUPLOT, "sending \"%s\"\n", cmd);
 
   write(gnuplotProcess->stdin, cmd, strlen(cmd));
   write(gnuplotProcess->stdin, &cr, 1);
/*
   while ((readBytes = read(gnuplotProcess->stdout, buffer, 512)) > 0) {
      write(STDOUT_FILENO, buffer, readBytes);
   }
*/
}

/*
 * Terminaison de gnuplot
 */
void on_exitGnuplot(int exitStatus, void * gP)
{
   struct gnuplotProcess_t * gnuplotProcess = (struct gnuplotProcess_t *)gP; 
   struct gnuplot_t * term, *prev;
   struct fileName_t * p;
   int n;

   printf_debug(DEBUG_GNUPLOT, "closing gnuplot terminals ...\n");

   // Terminaison propre du processus
   printf_debug(DEBUG_GNUPLOT, "terminating gnuplot process ...\n");
   GPSendCmd(NULL, "exit");

   // Fermeture des tubes de communication
   printf_debug(DEBUG_GNUPLOT, "closing gnuplot pipes ...\n");
   close(gnuplotProcess->stdin);
   close(gnuplotProcess->stdout);

   // Effacement des fichiers temporaires
   for (n=0,term = gnuplotProcess->lastTerminal; term != NULL; term = term->prev, n++) {
      while (term->srcFileName) {
         printf_debug(DEBUG_GNUPLOT, " unlinking \"%s\"\n", term->srcFileName->fileName);
         if (unlink(term->srcFileName->fileName)) {
            perror("unlink ");
         }
         p = term->srcFileName;
         term->srcFileName = p->next;
         free(p);
      }
   }
   assert(n == gnuplotProcess->nbPlot);

   // Lib�ration de la m�moire
   for (n=0,term = gnuplotProcess->lastTerminal; term != NULL; n++) {
      prev = term->prev;
      free(term);
      term = prev;
   }
   assert(n == gnuplotProcess->nbPlot);
}

int gnuplotProcessCreate()
{
   int  pipeGPin[2];
   int  pipeGPout[2];

   if (gnuplotProcess){
      return 1;
   }

   // Structure creation
   gnuplotProcess = (struct gnuplotProcess_t*) sim_malloc(sizeof(struct gnuplotProcess_t));
   gnuplotProcess->nbPlot = 0;
   gnuplotProcess->lastTerminal = NULL;
 
   // Pipes
   if (pipe(pipeGPin)) {
      perror("pipe ");
      return 0;
   }
   if (pipe(pipeGPout)) {
      perror("pipe ");
      return 0;
   }

   // Let's go !!
   gnuplotProcess->pid = fork();

   if (gnuplotProcess->pid == -1){
      perror("fork ");
      return 0;
   } else if (gnuplotProcess->pid == 0) {
      // Look for gnuplot binary

      // Input from here
      if (dup2(pipeGPin[0], STDIN_FILENO) == -1) {
         perror("dup2 ");
         return 0;
      }
      //close(pipeGPin[0]);
      //close(pipeGPin[1]);
      // Output to here
      if (dup2(pipeGPout[1], STDOUT_FILENO) == -1) {
         perror("dup2 ");
         return 0;
      }
      if (dup2(pipeGPout[1], STDERR_FILENO) == -1) {
         perror("dup2 ");
         return 0;
      }
      //close(pipeGPout[0]);
      //close(pipeGPout[1]);

      // Try to run
      execl(GnuPlotCmd , "gnuplot (forked by NDS)", NULL);
      printf("You should not see this message !\n");
      exit(1);
   }

   // Close useless fds
   close(pipeGPin[0]);
   close(pipeGPout[1]);

   // Don't forget the usefull ones 
   gnuplotProcess->stdin = pipeGPin[1];
   gnuplotProcess->stdout = pipeGPout[0];

   // Prepare the end
   on_exit(on_exitGnuplot, gnuplotProcess);

   return 1;
}

/*
 * Affichage dans une fenetre d'une sonde
 *
 * On passe par un fichier dont le nom est de la forme %d.%d-%s.gp
 * avec, dans l'ordre, le num�ro de processus, le num�ro de terminal
 * et le titre.
 * 
 * Le nom de fichier est sauv� pour effacer le fichier � la fin du
 * programme (par un on_exitGnuplot)
 */
#define BUFFER_LENGTH 4096
int gnuplot_displayProbe(struct gnuplot_t * gp, int with, struct probe_t * probe)
{
   char fileName[BUFFER_LENGTH];   
   struct fileName_t * p;
   int  fd;
   char cmd[BUFFER_LENGTH];
   
   assert(gp != NULL);

   if (gp->title == NULL) {
      printf_debug(DEBUG_GNUPLOT, "gp->title\n");
      gnuplot_setTitle(gp, probe_getName(probe));
   }

   // Cr�ation d'un fichier contenant le dump
   sprintf(fileName, "%d.%d-%s.gp", getpid(), gp->id, gp->title);
   if ((fd = open(fileName, O_CREAT|O_WRONLY, 0644)) == -1) {
      perror("open");
      return 0;
   }
   probe_dumpFd(probe, fd, dumpGnuplotFormat);
   close(fd);

   // Set filename
   p = (struct fileName_t *)sim_malloc(sizeof(struct fileName_t));
   p->next = gp->srcFileName;
   gp->srcFileName = p;
   gp->srcFileName->fileName = strdup(fileName);

   // Set terminal
   sprintf(cmd, "%s %d title '%s'",
           GPSetTermWxt,
           gp->id,
           gp->title);
   GPSendCmd(gp, cmd);

   // Plot the graph
   sprintf(cmd, "plot '%s' using 1:2 with %s title '%s'",
           fileName,
           (with == 1)?"points":(with == 2)?"lines":"boxes",
           probe_getName(probe));
   GPSendCmd(gp, cmd);

   return 1;
}

int gnuplot_displayProbes(struct gnuplot_t * gp, int with, ...)
{
   char fileName[BUFFER_LENGTH];   
   struct fileName_t * p;
   int  fd;
   char cmd[BUFFER_LENGTH];
   char cmdTmp[BUFFER_LENGTH];

   va_list argp;
   struct probe_t * probe;
   int n = 0;

   assert(gp != NULL);

   sprintf(cmd, "plot");

   va_start(argp, with);
   while ((probe = va_arg(argp, struct probe_t *)) != NULL) {
      if (gp->title == NULL) {
         printf_debug(DEBUG_GNUPLOT, "gp->title\n");
         gnuplot_setTitle(gp, probe_getName(probe));
      }

      // Cr�ation d'un fichier contenant le dump
      sprintf(fileName, "%d.%d-%s-%d.gp", getpid(), gp->id, gp->title, n);
      if ((fd = open(fileName, O_CREAT|O_WRONLY, 0644)) == -1) {
         perror("open");
         return 0;
      }
      probe_dumpFd(probe, fd, dumpGnuplotFormat);
      close(fd);

      // Set filename
      p = (struct fileName_t *)sim_malloc(sizeof(struct fileName_t));
      p->next = gp->srcFileName;
      gp->srcFileName = p;
      gp->srcFileName->fileName = strdup(fileName);

      printf_debug(DEBUG_TBD, "Gestion des fichiers temporaires\n");

      sprintf(cmdTmp, "%s '%s' using 1:2 with %s title '%s'",n?",":"",
              fileName,
              (with == 1)?"points":(with == 2)?"lines":"boxes",
              probe_getName(probe));
      n++;
      strcat(cmd, cmdTmp);
   }
   va_end(argp);

   // Plot the graph
   GPSendCmd(gp, cmd);
}

struct gnuplot_t * gnuplot_create()
{
   struct gnuplot_t * result = (struct gnuplot_t *)sim_malloc(sizeof(struct gnuplot_t));


   // Cr�ation du process
   if (!gnuplotProcess) {
      gnuplotProcessCreate();
   }

   result->srcFileName = NULL;
   result->title = NULL;
   result->id = gnuplotProcess->nbPlot++;

   result->xmin = 0.0;
   result->xmax = 0.0;

   result->ymin = 0.0;
   result->ymax = 0.0;

   result->prev = gnuplotProcess->lastTerminal;
   if (result->prev) {
      result->prev->next = result;
   }
   gnuplotProcess->lastTerminal = result;

   return result;
}

void gnuplot_setXRange(struct gnuplot_t * gp, double xmin, double xmax)
{
   char cmd[BUFFER_LENGTH];

   gp->xmin = xmin;
   gp->xmax = xmax;

   // Set ranges
   if (gp->xmin < gp->xmax) {
      sprintf(cmd, "set xrange [%f:%f]", gp->xmin, gp->xmax);
   } else {
      sprintf(cmd, "set xrange [*:*]");
   }
   GPSendCmd(gp, cmd);
}

void gnuplot_setYRange(struct gnuplot_t * gp, double ymin, double ymax)
{
   char cmd[BUFFER_LENGTH];

   gp->ymin = ymin;
   gp->ymax = ymax;

   // Set ranges
   if (gp->ymin < gp->ymax) {
      sprintf(cmd, "set yrange [%f:%f]", gp->ymin, gp->ymax);
   } else {
      sprintf(cmd, "set yrange [*:*]");
   }
   GPSendCmd(gp, cmd);

}

void gnuplot_setTitle(struct gnuplot_t * gp, char * name)
{
  gp->title = strdup(name);
  gp->title = sim_malloc(strlen(name)+1);

  strncpy(gp->title, name, strlen(name) +1);
}