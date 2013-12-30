/**
 * @file ndesObjectFile.h
 * @brief DÃ©finition de la gestion des files de ndesObject
 *
 */
#ifndef __DEF_LISTE_NDES_OBJECT
#define __DEF_LISTE_NDES_OBJECT

#include <ndesObject.h>
#include <probe.h>

//struct ndesObjectFile_t;

/**
 *  @brief Création d'une file.
 * 
 *  @param type permet de définir le type d'objets de la liste
 *  @return Une strut ndesObjectFile_t * allouée et initialisée
 *
 */
struct ndesObjectFile_t * ndesObjectFile_create(struct ndesObjectType_t * ndesObjectType);

/**
 * @brief Insertion d'un objet dans la file
 * @param file la file dans laquelle on insère l'objet
 * @param object à insérer à la fin de la file
 *
 */
void ndesObjectFile_insert(struct ndesObjectFile_t * file,
                           void * object);
void ndesObjectFile_insertObject(struct ndesObjectFile_t * file,
                                 struct ndesObject_t * object);

/**
 * @brief Extraction d'un objet depuis la file
 * @param file la file depuis laquelle on souhaite extraire
 * @return le premier object de la file ou NULL si la file est vide
 */
struct ndesObject_t * ndesObjectFile_extract(struct ndesObjectFile_t * file);

/**
 * @brief Extraction d'une PDU depuis la file
 * @param file la file depuis laquelle on souhaite extraire la
 * première PDU
 * @return la première PDU ou NULL si la file est vide
 * 
 * Ici la signature est directement compatible avec le modÃ¨le
 * d'entrèe-sortie de NDES.
 */
struct PDU_t * ndesObjectFile_getPDU(void * file);

/*
 * Nombre de PDU dans la file
 */
int ndesObjectFile_length(struct ndesObjectFile_t * file);

/*
 * Un affichage un peu moche de la file. Peut Ãªtre utile dans des
 * phases de dÃ©bogage.
 */
void ndesObjectFile_dump(struct ndesObjectFile_t * file);

/*
 * @brief Outil d'itération sur une liste
 */
struct ndesObjectFileIterator_t;

/*
 * @brief Initialisation d'un itérateur
 */
struct ndesObjectFileIterator_t * ndesObjectFile_createIterator(struct ndesObjectFile_t * of);

/*
 * @brief Obtention du prochain élément
 */
struct ndesObjectFile_t * ndesObjectFile_iteratorGetNext(struct ndesObjectFileIterator_t * ofi);

/*
 * @brief Terminaison de l'itérateur
 */
void ndesObjectFile_deleteIterator(struct ndesObjectFileIterator_t * ofi);

#endif
