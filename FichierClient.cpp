#include "FichierClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

int estPresent(const char* nom)
{
  int fd, pos = 1, rc;
  CLIENT buffer;

  if((fd = open(FICHIER_CLIENTS, O_RDONLY, 0664)) == -1)
  {
    //perror("Erreur de open() dans estpresent");
    return -1;
  }

  if ((rc = lseek(fd,0,SEEK_END)) == -1) perror("Erreur de lseek"); // pour connaitre la taille du fichier

  if(rc == 0)
  {
    if (close(fd)) {perror("Erreur de close()");}
    return 0;
  }

  if ((rc = lseek(fd,0,SEEK_SET)) == -1) perror("Erreur de lseek");// on se reposition

  if ((rc = read(fd,&buffer,sizeof(CLIENT))) == -1) perror("Erreur de read");

  while(rc != 0 && strcmp(buffer.nom, nom) != 0)
  {
    pos++;
    if ((rc = read(fd,&buffer,sizeof(CLIENT))) == -1) perror("Erreur de read");
  }

  if(rc == 0) 
  {
    return 0;
  }

  else return pos;

  if (close(fd)) 
  {
    perror("Erreur de close()");
  }


}

////////////////////////////////////////////////////////////////////////////////////
int hash(const char* motDePasse)
{
  int s = 0, j = 1, i = 0;

  while(motDePasse[i]!='\0')
  {
    s += (j * motDePasse[i]);
    i++;
    j++;
  }

  s = s%97;

  return s;
}

////////////////////////////////////////////////////////////////////////////////////
void ajouteClient(const char* nom, const char* motDePasse)
{
  int fd, rc;
  CLIENT buffer;

  fd = open(FICHIER_CLIENTS, O_WRONLY|O_CREAT|O_APPEND, 0664);

  if(fd == -1)
  {
    
    return;
  }

  strcpy(buffer.nom, nom);
  buffer.hash = hash(motDePasse);

  if(write(fd, &buffer, sizeof(CLIENT)) != sizeof(CLIENT)) perror("Erreur de write");

  if (close(fd)) 
  {
    perror("Erreur de close()");
  }
}

////////////////////////////////////////////////////////////////////////////////////
int verifieMotDePasse(int pos, const char* motDePasse)
{
  int fd, rc, mdp, veri = 0;
  CLIENT buffer;

  if((fd = open(FICHIER_CLIENTS, O_RDONLY, 0664)) == -1)
  {
    
    return -1;
  }

  pos--;

  if ((rc = lseek(fd,pos*sizeof(CLIENT),SEEK_SET)) == -1) perror("Erreur de lseek");

  mdp = hash(motDePasse);

  if ((rc = read(fd,&buffer,sizeof(CLIENT))) == -1) perror("Erreur de read");

  if(mdp == buffer.hash) veri = 1;
  else veri = 0;

  if (close(fd)) {perror("Erreur de close()");}

  return veri;
}

////////////////////////////////////////////////////////////////////////////////////
int listeClients(CLIENT *vecteur) // le vecteur doit etre suffisamment grand
{
  int nbr = 0, fd, rc;
  CLIENT buffer;

  if((fd = open(FICHIER_CLIENTS, O_RDONLY, 0664)) == -1)
  {
    //perror("Erreur de open() dans verimotdepasse");
    return -1;
  }

  if ((rc = read(fd,&buffer,sizeof(CLIENT))) == -1) perror("Erreur de read");

  while(rc != 0 && nbr < 20)
  {
    nbr++;

    strcpy(vecteur->nom, buffer.nom);
    vecteur->hash = buffer.hash;

    vecteur++;
    
    if ((rc = read(fd,&buffer,sizeof(CLIENT))) == -1) perror("Erreur de read");
  }

  return nbr;
}
