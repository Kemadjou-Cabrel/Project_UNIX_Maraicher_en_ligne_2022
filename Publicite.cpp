#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ, idShm;
char *pShm;
void handlerSIGUSR1(int sig);
int fd;

int main()
{
  //------------------------------------------Armement des signaux-------------------------------------------
  // TO DO
  struct sigaction A;
  A.sa_handler = handlerSIGUSR1;
  A.sa_flags = 0;
  sigemptyset(&A.sa_mask);
  sigaction(SIGUSR1,&A,NULL);

  // Masquage des tout les signaux sauf Sigusr1
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask,SIGUSR1);

  sigprocmask(SIG_SETMASK,&mask,NULL);
//-----------------------------------------------------------------------------------------------------

//----------------------------------Etape 2-------------------------------------------
  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(PUBLICITE) Erreur de msgget");
    exit(1);
  }

  // Recuperation de l'identifiant de la mémoire partagée
  if ((idShm = shmget(CLE,0,0)) == -1)
  {
    perror("Erreur de shmget");
    exit(1);
  }

  // Attachement à la mémoire partagée
  //pShm = (char*)malloc(52); // a supprimer et remplacer par ce qu'il faut
  if ((pShm = (char*)shmat(idShm,NULL,0)) == (char*)-1)//emplacement libre dans l’espace adressable du processu
  {
    perror("Erreur de shmat");
    exit(1);
  }

  // Mise en place de la publicité en mémoire partagée
  char pub[51];
  strcpy(pub,"Bienvenue sur le site du Maraicher en ligne !");

  for (int i=0 ; i<=50 ; i++) pShm[i] = ' '; 
  pShm[51] = '\0';

  int indDebut = (51 - strlen(pub))/2;

  for (int i=0 ; i<strlen(pub) ; i++) 
  {
    pShm[indDebut + i] = pub[i];
  }

  MESSAGE m;
  char tmp;
  int i;

  while(1)
  {
    m.type = 1;
    m.expediteur = getpid();
    m.requete = UPDATE_PUB;

    if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
      perror("Erreur de msgsnd");
      exit(1);
    }

    sleep(1); 

    // Decallage vers la gauche

    tmp = pShm[0]; // on garde le premier caractere

    for(i = 0; i < 50; i++)
    {
      pShm[i] = pShm[i+1];
    }

    pShm[50] = tmp;// et a la fin on mets ce caractere

  }
}
//----------------------------------Etape 7-------------------------------------------
void handlerSIGUSR1(int sig)
{
  fprintf(stderr,"(PUBLICITE %d) Nouvelle publicite !\n",getpid());

  // Lecture message NEW_PUB
  MESSAGE m;

  if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
  {
    perror("(PUBLICITE) Erreur de msgrcv");
    exit(1);
  }

  char pub[51];
  strcpy(pub,m.data4);

  for (int i=0 ; i<=50 ; i++)
  {
    pShm[i] = ' ';
  }
  pShm[51] = '\0';
  int indDebut = 25 - strlen(pub)/2;
  for (int i=0 ; i<strlen(pub) ; i++)
  {
    pShm[indDebut + i] = pub[i];
  }
    
}

