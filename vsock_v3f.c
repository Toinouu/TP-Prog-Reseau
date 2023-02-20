
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
   données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>




void construire_message (char *message, char motif, int lg,int compt) {
  int i;
  for (i=5 ; i<lg; i++) message[i] = motif; }

void afficher_donnees(int lg , int nbmsg , int source){
  if (source == 1){
    printf("SOURCE : lg_message=%d, nb message=%d, ",lg,nbmsg);
  }
  else{
    if(nbmsg==-1){
      printf("PUITS : lg_message=%d, nb message=infini ",lg);
    }else {
      printf("PUITS : lg_message=%d, nb message=%d ",lg,nbmsg);
    }
  }
 
}

void afficher_message (char *message, int lg,int source, int nbmsg,int compt) {
  int i;
  if(source==1){
    printf("SOURCE : Envoi n°%5d (%5d) ",compt,lg);
  }
  else{
    printf("PUITS: Reception n°%5d (%5d)",compt,lg);
  }
  printf("[");
  printf("%5d",compt);
  for (i=5 ; i<lg ; i++) printf("%c", message[i]) ;
  printf("]");
  printf("\n");
}
  
    

  





int main(int argc, char **argv)
{
  int c;
  extern char *optarg;
  extern int optind;
  int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
  int source = -1 ; /* 0=puits, 1=source */
  int lg_message = 30; //Sert pour le -l
  int udp = 0; 
  
  while ((c = getopt(argc, argv, "pn:sul:")) != -1) {
    switch (c) {
    case 'p':
      if (source == 1) {
	printf("usage: cmd [-p|-s][-n ##]\n");
	exit(1);
      }
      source = 0;
      break;

    case 's':
      if (source == 0) {
	printf("usage: cmd [-p|-s][-n ##]\n");
	exit(1) ;
      }
      source = 1;
      break;

    case 'n':
      nb_message = atoi(optarg);
      break;

    case 'u':
      udp=1;
      break;

    case'l':
      lg_message=atoi(optarg);
      break;
      
    default:
      printf("usage: cmd [-p|-s][-n ##]\n");
      break;
    }
  }

  
  /*=============================================================================*/



  if (nb_message != -1) { //Si -n spécifiée
    
    if (source == 1) // Si on envoie
      afficher_donnees(lg_message , nb_message , source);
      
    
    else // Si on reçoit
      afficher_donnees(lg_message , nb_message , source);
    
  }
  else { //Si -n  nn spécifiée
    
    if (source == 1) { // Si on envoie

     
      nb_message = 10 ;
      afficher_donnees(lg_message , nb_message , source);
      
    }
    else // Si on reçoit
      afficher_donnees(lg_message , nb_message , source);
     
  }


  if (source == -1) {
    printf("usage: cmd [-p|-s][-n ##]\n");
    exit(1) ;
  }

  /*=============================================================================*/
  
  if (source == 1){                        /*Source*/   

    if (udp==1){                          //Si on est en mode UDP
      
      struct sockaddr_in adr_distant;
      struct hostent * hp;
      int sock;
      char * msg;
      char motif = 'a';
      unsigned int adr_lg=sizeof(struct sockaddr_in);
      int i ;

      /* Création socket */
      sock=socket(AF_INET,SOCK_DGRAM,0);
      adr_distant.sin_port=htons(atoi(argv[argc-1]));
      hp = gethostbyname(argv[argc-2]);
      memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);
      adr_distant.sin_family=AF_INET;
      
      // Création + emission msg
      msg=malloc(nb_message*sizeof(lg_message));
      printf("Port=%s, proto=UDP, dest -> %s \n",argv[argc-1],argv[argc-2]);
      for (i=0;i<nb_message;i++){
	if (motif=='{'){
	  motif='a';
	}
	construire_message(msg,motif,lg_message,i);
	
	afficher_message(msg,lg_message,source,nb_message,i+1);
	motif += 1;
	sendto(sock,msg,lg_message,0,(struct sockaddr *)&adr_distant,adr_lg);
      }
     		      
    }
    else{                                //Si on est en mode TCP

      struct sockaddr_in adr_distant;
      struct hostent * hp;
      int sock;
      char * msg;
      char motif = 'a';
      unsigned int adr_lg=sizeof(struct sockaddr_in);
      int i ;
      int succes ;

      /* Création socket */
      sock=socket(AF_INET,SOCK_STREAM,0);
      adr_distant.sin_port=htons(atoi(argv[argc-1]));
      hp = gethostbyname(argv[argc-2]);
      memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);
      adr_distant.sin_family=AF_INET;

      //Requete connexion
      succes = connect(sock,(struct sockaddr *)&adr_distant,adr_lg) ;
      printf("Port=%s, proto=TCP, dest -> %s \n",argv[argc-1],argv[argc-2]);
      if (succes == 0){
	printf("SOURCE : Connexion etablie \n");
      }
      else {
	printf("SOURCE : Connexion refusée \n");
	exit(1) ;
      }
      
      // Emission message
      
      msg=malloc(nb_message*sizeof(lg_message));
      for (i=0;i<nb_message;i++){
	if (motif=='{'){
	  motif='a';
	}
	construire_message(msg,motif,lg_message,i);
	afficher_message(msg,lg_message,source,nb_message,i+1);
	motif= motif+1;
	send(sock,msg,lg_message,0);
      }

      //Fermeture connexion
      shutdown(sock,1) ;
      
      
    }
  }
  else{                                   /* PUIT */
    
    if (udp==1){  // Pour le mode UDP
      
      struct sockaddr_in adr_local;
      int sock;
      struct sockaddr_in adr_distant;
      char * adr_msg;
      adr_msg=malloc(sizeof(lg_message));
      int lg_eff;
      unsigned int adr_lg=sizeof(struct sockaddr_in);
      int i=0;
      
      // Création socket distant
      sock=socket(AF_INET,SOCK_DGRAM,0);
      adr_local.sin_port=htons(atoi(argv[argc-1]));
      adr_local.sin_addr.s_addr = INADDR_ANY ;
      adr_local.sin_family=AF_INET;
      
      // Association @->socket
      bind(sock,(struct sockaddr *)&adr_local,sizeof(adr_local));
      
      // Reception + affichage msg
      printf("Port=%s, proto=UDP, \n",argv[argc-1]);
      while(i!=nb_message){
	lg_eff=recvfrom(sock,adr_msg,lg_message,0,(struct sockaddr *)&adr_distant,&adr_lg);
	afficher_message(adr_msg,lg_eff,source,nb_message,i+1);
      
	i++;
      }
    
    }
    else{                                    //Si on est en mode TCP
        
      struct sockaddr_in adr_local;
      int sock;
      int sockbis ;
      struct sockaddr_in adr_distant;
      char * adr_msg;
      adr_msg=malloc(sizeof(lg_message));
      int lg_eff;
      unsigned int adr_lg=sizeof(struct sockaddr_in);
      int i=0;
      
      // Création socket distant
      sock=socket(AF_INET,SOCK_STREAM,0);
      adr_local.sin_port=htons(atoi(argv[argc-1]));
      adr_local.sin_addr.s_addr = INADDR_ANY ;
      adr_local.sin_family=AF_INET;

      // Association @->socket
      bind(sock,(struct sockaddr *)&adr_local,sizeof(adr_local));
      printf("Port=%s, proto=UDP, \n",argv[argc-1]);
      //Bufferisation des requêtes
      listen(sock, 5);
      
      // Mise en état d'acceptation
      sockbis=accept(sock,(struct sockaddr *)&adr_distant,&adr_lg); 
      
      if (sockbis==-1){
	printf("PUITS : Echec de la connexion\n");
      }
      else{
	printf("PUITS : Connexion accepté\n");
      }
      
      
      // Reception + affichage msg
     
      while(i!=nb_message){
	lg_eff=recv(sockbis,adr_msg,lg_message,0);
	if (lg_eff !=0){
	  afficher_message(adr_msg,lg_eff,source,nb_message,i+1);
	}
	else{  //On ne reçoit un message que s'il n'est pas vide
	  exit(1);
	}
	i++;
      }


  
    }
  } 
  
}
 
  
