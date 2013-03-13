/*######################################################################

 Exemple 9 : MPI_Gather

 Description:
   Cet exemple illustre l'utilisation de la fonction MPI_Gather
   pour rassembler les donnes provenant de plusieurs taches vers
   une tache en particulier.
   
          sendbuff
   
          ########
          #      #
        0 #  AA  #
          #      #
          ########
     T    #      #
        1 #  BB  #
     a    #      #
          ########
     c    #      #
        2 #  CC  #                                    AVANT
     h    #      #
          ########
     e    #      #
        3 #  DD  #
     s    #      #
          ########
          #      #
        4 #  EE  #
          #      #
          ########
          
            <---------- recvbuff ---------->

          ####################################
          #      #      #      #      #      #
        0 #  AA  #  BB  #  CC  #  DD  #  EE  #
          #      #      #      #      #      #
          ####################################
     T    #      #      #      #      #      #
        1 #      #      #      #      #      #
     a    #      #      #      #      #      #
          ####################################
     c    #      #      #      #      #      #
        2 #      #      #      #      #      #       APRES
     h    #      #      #      #      #      #
          ####################################
     e    #      #      #      #      #      #
        3 #      #      #      #      #      #
     s    #      #      #      #      #      #
          ####################################
          #      #      #      #      #      #
        4 #      #      #      #      #      #
          #      #      #      #      #      #
          ####################################
   
   Chaque tache prepare (initialise) un vecteur de reels sendbuff
   de dimension buffsize. Ensuite la fonction MPI_Gather rassemble
   a la tache 0 le contenu de ces vecteurs et les place dans le
   tableau recvbuff.
   
   Avant et apres l'etape de communication, la somme de chacun 
   des vecteurs est affichee a l'ecran pour verification.   

 Auteur: Carol Gauthier
         Centre de Calcul scientifique
         Universite de Sherbrooke

 Derniere revision: Septembre 2005

######################################################################*/

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "math.h"
#include "mpi.h"

int main(int argc,char** argv)
{
   /*===============================================================*/
   /* Declaration des variables                                     */
   int          taskid, ntasks;
   MPI_Status   status;
   int          ierr,i,j,itask;
   int	        buffsize;
   double       *sendbuff,**recvbuff,buffsum;
   double       inittime,totaltime;
   
   /*===============================================================*/
   /* Initialisation de MPI. Il est important de placer cet appel   */
   /* au debut du programme, immediatement apres les declarations   */
   MPI_Init(&argc, &argv);

   /*===============================================================*/
   /* Obtenir le nombre de taches MPI et le numero d'identification */
   /* de la tache courante taskid                                   */
   MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
   MPI_Comm_size(MPI_COMM_WORLD,&ntasks);

   /*===============================================================*/
   /* Obtenir buffsize a partir des arguments.                      */
   buffsize=atoi(argv[1]);

   /*===============================================================*/
   /* Affichage de la description de l'exemple.                     */
   if ( taskid == 0 ){
     printf("\n\n\n");
     printf("##########################################################\n\n");
     printf(" Exemple 9 \n\n");
     printf(" Communication collective : MPI_Gather \n\n");
     printf(" La tache 0 rassemble les donnees du vecteur sendbuff de\n");
     printf(" chaque tache et les place dans recvbuff. \n\n");
     printf(" Dimension de chaque vecteur a transferer: %d\n",buffsize);
     printf(" Nombre total de taches: %d\n\n",ntasks);
     printf("##########################################################\n\n");
     printf("                --> AVANT COMMUNICATION <--\n\n");
   }

   /*=============================================================*/
   /* Allocation de la memoire.                                   */ 
   sendbuff=(double *)malloc(sizeof(double)*buffsize);
   if( taskid == 0 ){
     recvbuff=(double **)malloc(sizeof(double *)*ntasks);
     recvbuff[0]=(double *)malloc(sizeof(double)*ntasks*buffsize);
     for(i=1;i<ntasks;i++)recvbuff[i]=recvbuff[i-1]+buffsize;
   }
   else{
     recvbuff=(double **)malloc(sizeof(double *)*1);
     recvbuff[0]=(double *)malloc(sizeof(double)*1);
   }
   
   /*=============================================================*/
   /* Initialisation du/des vecteurs et/ou tableaux               */
   srand((unsigned)time( NULL ) + taskid);
   for(i=0;i<buffsize;i++){
       sendbuff[i]=(double)rand()/RAND_MAX;
   }
   
   /*==============================================================*/
   /* Affichage avant communication.                               */
   
   MPI_Barrier(MPI_COMM_WORLD);
   
   buffsum=0.0;
   for(i=0;i<buffsize;i++){
     buffsum=buffsum+sendbuff[i];
   }
   printf("Tache %d : Somme du vecteur = %e \n",taskid,buffsum);
     
   MPI_Barrier(MPI_COMM_WORLD);     
     
   /*===============================================================*/
   /* Communication.                                                */

   inittime = MPI_Wtime();
   ierr=MPI_Gather(sendbuff,buffsize,MPI_DOUBLE,
                   recvbuff[0],buffsize,MPI_DOUBLE,
                   0,MPI_COMM_WORLD);
   totaltime = MPI_Wtime() - inittime;
     
   /*===============================================================*/
   /* Affichage apres communication.                                */
   if ( taskid == 0 ){
     printf("\n");
     printf("##########################################################\n\n");
     printf("                --> APRES COMMUNICATION <-- \n\n");
     for(itask=0;itask<ntasks;itask++){
       buffsum=0.0;
       for(i=0;i<buffsize;i++){
         buffsum=buffsum+recvbuff[itask][i];
       }
       printf("Tache %d : Somme du vecteur recu par %d -> %e \n",
               taskid,itask,buffsum);
       
     }       
   }       

   if(taskid==0){
     printf("\n");
     printf("##########################################################\n\n");
     printf(" Temps total de communication : %f secondes\n\n",totaltime);  
     printf("##########################################################\n\n");
   }

   /*===============================================================*/
   /* Liberation de la memoire                                      */
   if ( taskid == 0 ){
     free(recvbuff[0]);
     free(recvbuff);
   }
   else{
     free(recvbuff[0]);
     free(recvbuff);
     free(sendbuff);
   }

   /*===============================================================*/
   /* Finalisation de MPI                                           */
   MPI_Finalize();

}


