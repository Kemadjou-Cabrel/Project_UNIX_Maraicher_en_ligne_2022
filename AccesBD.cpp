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

MYSQL* connexion;

int main(int argc,char* argv[])
{
  //--------------------------------------------Masquage de SIGINT--------------------------------------------
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  //----------------------------------------------------------------------------------------------------------


  //Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(ACCESBD %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(ACCESBD) Erreur de msgget");
    exit(1);
  }

  // Récupération descripteur lecture du pipe
  int fdRpipe = atoi(argv[1]);

  fprintf(stderr,"(ACCESBD %d) pipe lecture recu : %d\n", getpid(), fdRpipe);

  // Connexion à la base de donnée
  // TO DO
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  char requete[200];
  char newUser[20];
  MYSQL_RES  *resultat;
  MYSQL_ROW  Tuple;

  MESSAGE m, reponse;
  int ret, RESULTAT;
  char table[11];

  strcpy(table, "UNIX_FINAL");

  while(1)
  {
    // Lecture d'une requete sur le pipe
    // TO DO

    fprintf(stderr,"(ACCESBD %d) Attente d'une requete...\n", getpid());

    if ((ret = read(fdRpipe,&m,sizeof(MESSAGE))) < 0)
    {
      perror("(ACCESBD) Erreur de read");
      exit(1);
    }

    if (ret == 0)
    {
      //printf("(ACCESBD %d) ----- Fin de lecture rien dans le fichier  ---\n",getpid());
      mysql_close(connexion);
      close(fdRpipe);
      exit(1);
    }

    switch(m.requete)
    {
      case CONSULT :  // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      sprintf(requete,"select * from %s where id = %d;", table, m.data1);

                      if (mysql_query(connexion,requete) != 0)
                      {
                        fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
                        exit(1);
                      }

                      printf("Requete SELECT réussie.\n");

                      // Affichage du Result Set

                      if ((resultat = mysql_store_result(connexion)) == NULL)
                      {
                        fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
                        exit(1);
                      }

                      // Preparation de la reponse

                      if((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        reponse.requete = CONSULT;
                        reponse.data1 = atoi(Tuple[0]);
                        strcpy(reponse.data2, Tuple[1]);
                        strcpy(reponse.data3, Tuple[3]);
                        strcpy(reponse.data4, Tuple[4]);
                        reponse.data5 = atof(Tuple[2]);
                      }
                      else
                      {
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        reponse.requete = CONSULT;
                        reponse.data1 = -1;
                      }

                      // Envoi de la reponse au bon caddie

                      if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                      {
                        perror("Erreur de msgsnd");
                        exit(1);
                      }

                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      sprintf(requete,"select * from %s where id = %d;", table, m.data1);

                      if (mysql_query(connexion,requete) != 0)
                      {
                        fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
                        exit(1);
                      }

                      printf("Requete SELECT réussie.\n");

                      // Affichage du Result Set

                      if ((resultat = mysql_store_result(connexion)) == NULL)
                      {
                        fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
                        exit(1);
                      }

                      // Preparation de la reponse

                      if((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                        int quant = atoi(Tuple[3]);
                        int quantClient = atoi(m.data2);

                        if(quantClient <= quant)
                        {
                          reponse.type = m.expediteur;
                          reponse.expediteur = getpid();
                          reponse.requete = ACHAT;
                          reponse.data1 = atoi(Tuple[0]);
                          strcpy(reponse.data2, Tuple[1]);
                          strcpy(reponse.data3, m.data2);
                          strcpy(reponse.data4, Tuple[4]);
                          reponse.data5 = atof(Tuple[2]);

                          // mettre ajour la bd 
                          RESULTAT = quant - quantClient;

                          sprintf(requete,"update %s set stock = %d where id = %d;", table, RESULTAT, m.data1);

                          if (mysql_query(connexion,requete) != 0)
                          {
                            fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
                            exit(1);
                          }

                          printf("Requete UPDATE réussie.\n");
                        }
                        else
                        {
                          reponse.type = m.expediteur;
                          reponse.expediteur = getpid();
                          reponse.requete = ACHAT;
                          reponse.data1 = atoi(Tuple[0]);
                          strcpy(reponse.data2, Tuple[1]);
                          strcpy(reponse.data3, "0");// pour dire quil ya rien 
                          strcpy(reponse.data4, Tuple[4]);
                          reponse.data5 = atof(Tuple[2]);
                        }
                      }

                      //envoi de la reponse

                      if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                      {
                        perror("Erreur de msgsnd");
                        exit(1);
                      }

                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      sprintf(requete,"select * from %s where id = %d;", table, m.data1);

                      if (mysql_query(connexion,requete) != 0)
                      {
                        fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
                        exit(1);
                      }

                      printf("Requete SELECT réussie.\n");

                      // Affichage du Result Set

                      if ((resultat = mysql_store_result(connexion)) == NULL)
                      {
                        fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
                        exit(1);
                      }

                      // Mise à jour du stock en BD

                      if((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                        int quant = atoi(Tuple[3]);
                        int quantClient = atoi(m.data2);

                        RESULTAT = quant + quantClient;

                        sprintf(requete,"update %s set stock = %d where id = %d;", table, RESULTAT, m.data1);

                        if (mysql_query(connexion,requete) != 0)
                        {
                          fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
                          exit(1);
                        }

                        printf("Requete UPDATE réussie.\n");
                      }

                      break;

    }
  }
}
