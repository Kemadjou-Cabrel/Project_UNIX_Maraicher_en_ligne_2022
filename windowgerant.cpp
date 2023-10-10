#include "windowgerant.h"
#include "ui_windowgerant.h"
#include <iostream>
using namespace std;
#include <mysql.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include "protocole.h"

#include "MySemaphores.h"

int idArticleSelectionne = -1;
MYSQL *connexion;
MYSQL_RES  *resultat;
MYSQL_ROW  Tuple;
char requete[200];
int idSem;
int idQ;

int i = 1;
char table[11];

WindowGerant::WindowGerant(QWidget *parent) : QMainWindow(parent),ui(new Ui::WindowGerant)
{
    ui->setupUi(this);

    // Configuration de la table du stock (ne pas modifer)
    ui->tableWidgetStock->setColumnCount(4);
    ui->tableWidgetStock->setRowCount(0);
    QStringList labelsTableStock;
    labelsTableStock << "Id" << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetStock->setHorizontalHeaderLabels(labelsTableStock);
    ui->tableWidgetStock->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetStock->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetStock->horizontalHeader()->setVisible(true);
    ui->tableWidgetStock->horizontalHeader()->setDefaultSectionSize(120);
    ui->tableWidgetStock->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetStock->verticalHeader()->setVisible(false);
    ui->tableWidgetStock->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Recuperation de la file de message car il vas envoie un message new pub au serveur 
    // TO DO
    fprintf(stderr,"(GERANT %d) Recuperation de l'id de la file de messages\n", getpid());

    if ((idQ = msgget(CLE,0)) == -1)
    {
        perror("(GERANT) Erreur de msgget");
        exit(1);
    }

    // TO DO
    if ((idSem = semget(CLE,0,0)) == -1)
    {
        perror("Erreur de semget");
        exit(1);
    }

    printf("idSem = %d\n",idSem);

    // Prise blocante du semaphore
    // TO DO

    sem_wait(0);

    // Connexion à la base de donnée
    connexion = mysql_init(NULL);

    fprintf(stderr,"(GERANT %d) Connexion à la BD\n",getpid());

    if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
    {
      fprintf(stderr,"(GERANT %d) Erreur de connexion à la base de données...\n",getpid());
      exit(1);  
    }

    // Recuperation des articles en BD
    // TO DO

    strcpy(table, "UNIX_FINAL");

    sprintf(requete,"select * from %s where id = %d;", table, i);

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

    while((Tuple = mysql_fetch_row(resultat)) != NULL)// ici on prend les ligne de la table dans la base de donnee
    {   
        string tmp(Tuple[2]); // ici on le contenus du tuple 2(prix )
        size_t x = tmp.find("."); // on regarde le . 
        if (x != string::npos) tmp.replace(x,1,","); // et on remplace par la virgule

        ajouteArticleTablePanier(atoi(Tuple[0]), Tuple[1], atof(tmp.c_str()), atoi(Tuple[3]));

        i++;// ici le i++ est utilise pour preparer la prochain requette sql qui sera utilise 

        sprintf(requete,"select * from %s where id = %d;", table, i);

        if (mysql_query(connexion,requete) != 0)
        {
            fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
            exit(1);
        }

        printf("Requete SELECT réussie.\n");

        // Affichage du Result Set

        if ((resultat = mysql_store_result(connexion)) == NULL)// recupere le nouveau resultat en memoire 
        {
            fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
            exit(1);
        }
    }
}

WindowGerant::~WindowGerant()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du stock (ne pas modifier) //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::ajouteArticleTablePanier(int id,const char* article,float prix,int quantite)
{
    char Id[20],Prix[20],Quantite[20];

    sprintf(Id,"%d",id);
    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetStock->rowCount();
    nbLignes++;
    ui->tableWidgetStock->setRowCount(nbLignes);
    ui->tableWidgetStock->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Id);
    ui->tableWidgetStock->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetStock->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetStock->setItem(nbLignes-1,2,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetStock->setItem(nbLignes-1,3,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::videTableStock()
{
    ui->tableWidgetStock->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowGerant::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetStock->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_tableWidgetStock_cellClicked(int row, int column)
{
    //cerr << "ligne=" << row << " colonne=" << column << endl;
    ui->lineEditIntitule->setText(ui->tableWidgetStock->item(row,1)->text());
    ui->lineEditPrix->setText(ui->tableWidgetStock->item(row,2)->text());
    ui->lineEditStock->setText(ui->tableWidgetStock->item(row,3)->text());
    idArticleSelectionne = atoi(ui->tableWidgetStock->item(row,0)->text().toStdString().c_str());
    //cerr << "id = " << idArticleSelectionne << endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float WindowGerant::getPrix()
{
    return atof(ui->lineEditPrix->text().toStdString().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowGerant::getStock()
{
    return atoi(ui->lineEditStock->text().toStdString().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowGerant::getPublicite()
{
  strcpy(publicite,ui->lineEditPublicite->text().toStdString().c_str());
  return publicite;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::closeEvent(QCloseEvent *event)
{
  fprintf(stderr,"(GERANT %d) Clic sur croix de la fenetre\n",getpid());
  // TO DO
  // Deconnexion BD
  mysql_close(connexion);

  // Liberation du semaphore
  // TO DO
  sem_signal(0);

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_pushButtonPublicite_clicked()
{
    fprintf(stderr,"(GERANT %d) Clic sur bouton Mettre a jour\n",getpid());
    // TO DO (étape 7)
    // Envoi d'une requete NEW_PUB au serveur

    MESSAGE reponse;

    reponse.type = 1;
    reponse.expediteur = getpid();
    strcpy(reponse.data4, getPublicite());
    reponse.requete = NEW_PUB;

    if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
        perror("Erreur de msgsnd");
        exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_pushButtonModifier_clicked()
{
    fprintf(stderr,"(GERANT %d) Clic sur bouton Modifier\n",getpid());
    // TO DO
    cerr << "Prix  : --"  << getPrix() << "--" << endl;
    cerr << "Stock : --"  << getStock() << "--" << endl;

    char Prix[20];
    sprintf(Prix,"%f",getPrix());
    string tmp(Prix);// ici on va tranforme prix en chaine de caractere 
    size_t x = tmp.find(",");
    if (x != string::npos) tmp.replace(x,1,".");

    //fprintf(stderr,"(GERANT %d) stock = %d et prix= %s and %s\n",getpid(), getStock(),Prix, tmp.c_str());

    fprintf(stderr,"(GERANT %d) Modification en base de données pour id=%d\n",getpid(),idArticleSelectionne);

    // Mise a jour table BD
    // TO DO

    sprintf(requete,"UPDATE %s SET prix = %s, stock = %d WHERE id = %d;", table, tmp.c_str(), getStock(), idArticleSelectionne);
    
    if (mysql_query(connexion,requete) != 0)
    {
        fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
        exit(1);
    }

    printf("Requete UPDATE réussie.\n");

    videTableStock();// efface le contenue de la table d'affiche a ecran 

    i = 1;

    sprintf(requete,"select * from %s where id = %d;", table, i);

    if (mysql_query(connexion,requete) != 0)
    {
        fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
        exit(1);
    }

    //printf("Requete SELECT réussie.\n");

    // Affichage du Result Set

    if ((resultat = mysql_store_result(connexion)) == NULL)
    {
        fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
        exit(1);
    }
    // recupere en nouveaux l'ensemble des article dans la bd 

    while((Tuple = mysql_fetch_row(resultat)) != NULL)
    {
        string tmp(Tuple[2]);
        size_t x = tmp.find(".");
        if (x != string::npos) tmp.replace(x,1,",");

        ajouteArticleTablePanier(atoi(Tuple[0]), Tuple[1], atof(tmp.c_str()), atoi(Tuple[3]));

        i++;

        sprintf(requete,"select * from %s where id = %d;", table, i);

        if (mysql_query(connexion,requete) != 0)
        {
            fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
            exit(1);
        }

        //printf("Requete SELECT réussie.\n");

        // Affichage du Result Set

        if ((resultat = mysql_store_result(connexion)) == NULL)
        {
            fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
            exit(1);
        }
    }
}
