/**
 * @file log.h
 * @brief Définition des outils de log
 *
 * L'objectif de cet outil est de permettre de "loguer" des
 * informations sur des événements qui ont lieu durant la
 * simulation. La base de données ainsi produite pourra ensuite être
 * sauvée dans un fichier permettant une exploitation a posteriori.
 * Le log est pour le moment une simple séquence de lignes de texte.
 *
 * A faire : une macro USE_LOG sans laquelle tout est defini à rien.
 */
#ifndef __DEF_LOG
#define __DEF_LOG

#include <stdarg.h>

#include "motsim.h"
#include "ndesObject.h"

/**
 * Exporté pour ndesObject qui logue mais ne doit pas logué ce qui a
 * trait aux logs, ...
 */
struct ndesObjectType_t ndesLogEntryType;

#ifdef NDES_USES_LOG

/*
 * @brief Initialisation du log
 */
void ndesLog_init();

/*
 * @brief Activation du log
 */
void ndesLog_enable();

/*
 * @brief Désactivation du log
 */
void ndesLog_disable();

/*
 * Insersion d'une ligne de log
 */
void ndesLog_logLine(struct ndesObject_t * ndesObject, char * line);

/*
 * @brief Insertion d'une ligne de log avec formatage
 */
void ndesLog_logLineF(struct ndesObject_t * ndesObject, char * fmt, ...);

/*
 * Dump du log dans un fichier (l'éventuel contenu précédent est détruit)
 */
int ndesLog_dump(char * fileName);

#else  // Si NDES_USES_LOG n'est pas défini

#define ndesLog_init()
#define ndesLog_enable()
#define ndesLog_disable()
#define ndesLog_logLine(ndesObject, line)
#define ndesLog_logLineF(ndesObject, fmt, ...)
#define ndesLog_dump(fileName)

#endif // ifdef NDES_USES_LOG
#endif
