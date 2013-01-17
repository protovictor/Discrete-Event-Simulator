#ifndef __DEF_MAFILE
#define __DEF_MAFILE

extern struct t_file;

struct t_file * creerFileVide();

void insererFile(struct t_file * file, void * data);

void * extraireFile(struct t_file * file);

int tailleFile(struct t_file * file);

#endif
