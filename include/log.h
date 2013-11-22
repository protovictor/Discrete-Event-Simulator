/**
 * @file log.h
 * @brief Définition des outils de log
 *
 * L'objectif de cet outil est de permettre de "loguer" des
 * informations sur des événements qui ont lieu durant la
 * simulation. La base de données ainsi produite pourra ensuite être
 * sauvée dans un fichier permettant une exploitation a posteriori.
 * Le log est pour le moment une simple séquence de lignes de texte.
 */
#ifndef __DEF_LOG
#define __DEF_LOG

#include <motsim.h>


/*
 * Initialisation du log
 */
void ndesLog_init(char * line);

/*
 * Insertion d'une ligne de log
 */
void ndesLog_logLine(char * line);

/*
 * Dump du log dans un fichier (l'éventuel contenu précédent est détruit)
 */
int ndesLog_dump(char * filename);

#endif
