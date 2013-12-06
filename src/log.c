
/** @file log.c
 *  @brief Gestion des log de NDES
 *
 * Pas encore pret à être utilisé
 */
#include <log.h>
#include <ndesObject.h>

void * ndesLog_alloc();

struct ndesTypeHelper_t ndesLog_objectHelper = {
   .name = "log",
   .malloc = ndesLog_alloc
};
/*
 * Initialisation du log
 */
void ndesLog_init()
{
   ndesObject_addType("log", &ndesLog_objectHelper);
};

void * ndesLog_alloc()
{
   
};


/*
 * @brief Insertion d'une ligne de log
 */
void ndesLog_logLine(char * line)
{
   printf("[LOG] %s\n", line);
}

/*
 * @brief Insertion d'une ligne de log avec formatage
 */
void ndesLog_logLineF(char * fmt, ...)           
{
   printf("[LOG] ndesLog_logLineF a faire !\n");
}


/*
 * Dump du log dans un fichier (l'éventuel contenu précédent est détruit)
 */
int ndesLog_dump(char * filename)
{
}
