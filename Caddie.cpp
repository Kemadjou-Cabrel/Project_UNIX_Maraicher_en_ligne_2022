#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <mysql.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ;

ARTICLE articles[10];
int nbArticles = 0;

int fdWpipe;
int pidClient;

MYSQL* connexion;

int client;

void handlerSIGALRM(int sig);

int main(int argc,char* argv[])
{
//--------------------------------------------------------------------Armement des signaux et masquage -------------------------------
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  
  // TO DO

  struct sigaction A;
  A.sa_handler = handlerSIGALRM;
  sigemptyset(&A.sa_mask);
  A.sa_flags = 0;

  if (sigaction(SIGALRM,&A,NULL) == -1)
  {
    perror("Erreur de sigaction");
    exit(1);
  }
//-----------------------------------------------------------------------------------------------------------------------

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n", getpid());

  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(CADDIE) Erreur de msgget");
    exit(1);
  }

  MESSAGE m;
  MESSAGE reponse;
  int i, k;

  //initialiser vecteur articles avec des valeurs temporaire
  for(i = 0; i < 10; i++)
  {
    articles[i].id = 0; 
  }

  // Récupération descripteur écriture du pipe
  fdWpipe = atoi(argv[1]);

  while(1)
  {
    fprintf(stderr,"(CADDIE %d) attente de requete\n",getpid());

    alarm(60);

    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
    {
      perror("(CADDIE) Erreur de msgrcv");
      exit(1);
    }
    alarm(0);

    switch(m.requete)
    {
      case LOGIN :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);

                      client = m.expediteur; 

                      break;

      case LOGOUT :   // TO DO
                      // envoie signal
                      fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);

                      close(fdWpipe);

                      exit(5);

                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);

                      // Construction et exécution de la requête

                      client = m.expediteur;

                      m.expediteur = getpid();

                      if (write(fdWpipe,&m,sizeof(MESSAGE)) != sizeof(MESSAGE))
                      {
                        perror("(CADDIE) Erreur de write");
                        exit(1);
                      }
                      fprintf(stderr,"(CADDIE %d) attente pour le base de donnes\n",getpid());


                      // attente du message de access bd

                      if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
                      {
                        perror("(CADDIE) Erreur de msgrcv");
                        exit(1);
                      }

                      if(m.data1 != -1)
                      {
                        m.type = client;
                        m.expediteur = getpid();
                        m.requete = CONSULT;

                        if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                        {
                            perror("Erreur de msgsnd");
                            exit(1);
                        }

                        kill(m.type, SIGUSR1);
                      }
                      
                      break;


      case ACHAT :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                      // on transfert la requete à AccesBD
                      client = m.expediteur;

                      m.expediteur = getpid();

                      if (write(fdWpipe,&m,sizeof(MESSAGE)) != sizeof(MESSAGE))
                      {
                        perror("(CADDIE) Erreur de read");
                        exit(1);
                      }
                      

                      fprintf(stderr,"(CADDIE %d) attente pour le base de donnes\n",getpid());

                      if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
                      {
                        perror("(CADDIE) Erreur de msgrcv");
                        exit(1);
                      }
                        
                      // Envoi de la reponse au client

                      if(atoi(m.data3) != 0)
                      {
                        if(nbArticles < 10)
                        {
                          i = 0;

                          while(articles[i].id != 0 && i < 10) i++; //juste pour connaitre endroit pour mettre article 

                          articles[i].id = m.data1;
                          strcpy(articles[i].intitule, m.data2);
                          articles[i].prix = m.data5;
                          articles[i].stock = atoi(m.data3);
                          strcpy(articles[i].image, m.data4);

                          nbArticles++;
                        }
                      }

                      m.type = client;
                      m.expediteur = getpid();
                      m.requete = ACHAT;

                      if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                      {
                          perror("Erreur de msgsnd");
                          exit(1);
                      }

                      kill(m.type, SIGUSR1);

                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d,%d\n",getpid(),m.expediteur);

                      m.type = m.expediteur;
                      m.expediteur = getpid();
                      m.requete=CADDIE;

                      for(i = 0; i < nbArticles; i++)  
                      {
                        m.data1 = articles[i].id;
                        strcpy(m.data2, articles[i].intitule);
                        m.data5 = articles[i].prix;
                        sprintf(m.data3, "%d", articles[i].stock);
                        strcpy(m.data4, articles[i].image);

                        if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                        {
                            perror("Erreur de msgsnd");
                            exit(1);
                        }

                        kill(m.type, SIGUSR1);

                        

                        fprintf(stderr,"(CADDIE %d) Requete CADDIE envoyer au %d\n",getpid(),m.type);

                        

                        printf("Voici le article envoyer: \n");
                        printf("%d ; %s ; %f ; %d\n", articles[i].id, articles[i].intitule, articles[i].prix, articles[i].stock);
                      }



                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);

                      k = m.data1;

                      printf("Voici le article: \n");
                      printf("%d ; %s ; %f ; %d\n", articles[k].id, articles[k].intitule, articles[k].prix, articles[k].stock);

                      reponse.type = m.expediteur;
                      reponse.expediteur = getpid();
                      reponse.data1 = articles[k].id;
                      sprintf(reponse.data2, "%d", articles[k].stock);
                      reponse.requete = CANCEL;

                      // on transmet la requete à AccesBD

                      if (write(fdWpipe,&reponse,sizeof(MESSAGE)) != sizeof(MESSAGE))
                      {
                        perror("(CADDIE) Erreur de read");
                        exit(1);
                      }

                      // Suppression de l'aricle du panier

                      if(articles[k+1].id == 0)
                      {
                        articles[k].id = 0;
                        nbArticles--;
                      }
                      //pousse article
                      else
                      {
                        while(articles[k+1].id != 0 && k < 10)
                        {
                          articles[k].id = articles[k+1].id;
                          strcpy(articles[k].intitule, articles[k+1].intitule);
                          articles[k].prix = articles[k+1].prix;
                          articles[k].stock = articles[k+1].stock;
                          strcpy(articles[k].image, articles[k+1].image);

                          k++;
                        }
                        nbArticles--;
                      }

                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);

                      // On envoie a AccesBD 

                      for(i = 0; i < nbArticles; i++)
                      {
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        reponse.data1 = articles[i].id;
                        sprintf(reponse.data2, "%d", articles[i].stock);
                        reponse.requete = CANCEL;

                        if (write(fdWpipe,&reponse,sizeof(MESSAGE)) != sizeof(MESSAGE))
                        {
                          perror("(CADDIE) Erreur de read");
                          exit(1);
                        }
                      }

                      // On vide le panier

                      for(i = 0; i < nbArticles; i++)
                      {
                        articles[i].id = 0;
                      }

                      nbArticles = 0;

                      break;

      case PAYER :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);

                      // On vide le panier

                      for(i = 0; i < nbArticles; i++)
                      {
                        articles[i].id = 0;
                      }

                      nbArticles = 0;
                      
                      break;
    }
  }
}

void handlerSIGALRM(int sig)
{
  fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());

  // Annulation du caddie et mise à jour de la BD

  MESSAGE reponse;
  int i;

  for(i = 0; i < nbArticles; i++)
  {
    reponse.type = client;
    reponse.expediteur = getpid();
    reponse.data1 = articles[i].id;
    sprintf(reponse.data2, "%d", articles[i].stock);
    reponse.requete = CANCEL;

    if (write(fdWpipe,&reponse,sizeof(MESSAGE)) != sizeof(MESSAGE))
    {
      perror("(CADDIE) Erreur de read");
      exit(1);
    }
  }

  // On vide le panier

  for(i = 0; i < nbArticles; i++)
  {
    articles[i].id = 0;
  }

  nbArticles = 0;

  // Envoi d'un Time Out au client 

  reponse.type = client;
  reponse.expediteur = getpid();
  reponse.requete = TIME_OUT;

  if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
  {
    perror("Erreur de msgsnd");
    exit(1);
  }

  //(s'il existe toujours)
  kill(client, SIGUSR1);
  
  close(fdWpipe);

  exit(0);
}