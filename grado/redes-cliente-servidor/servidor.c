/*
 *          		S E R V I D O R
 *
 *	This is an example program that demonstrates the use of
 *	sockets TCP and UDP as an IPC mechanism.  
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/sem.h>  /* Semáforos.  */



#define SEMAFORO 0
#define PUERTO 17278
#define ADDRNOTFOUND	0xffffffff	/* return address for unfound host */
#define BUFFERSIZE	1024	/* maximum size of packets to be received */
#define TAM_BUFFER 10
#define MAXHOST 128
#define TIMEOUT 6 
extern int errno;

int semID;

/*
 *			M A I N
 *
 *	This routine starts the server.  It forks, leaving the child
 *	to do all the work, so it does not have to be run in the
 *	background.  It sets up the sockets.  It
 *	will loop forever, until killed by a signal.
 *
 */

// Unión necesario para la ejecución del programa en encina

union{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
}semf;


//===========TODO BIEN================
typedef struct
{
    char pregunta[100];
    int respuesta;
}Pregunta;

Pregunta preguntas[100];
int numPreguntas;

//======================================
void mostrar(Pregunta *preguntas, int numPreguntas);
int cargarDiccionario(Pregunta *preguntas);
//======================================

void serverTCP(int s, struct sockaddr_in peeraddr_in);
void serverUDP(int s, char * buffer, struct sockaddr_in clientaddr_in);
void errout(char *);		/* declare error out routine */
void splitString(char *input, char **tokens, const char *delimiter);
int comprobarSintaxTCP(char* cadena);
int comprobarSintaxUDP(char* cadena);
char crearLineaLog(char hostname, char hostnameIP, char protocolo, char pEfimero, char orden);


//----------Semaforos
void semwait(int);
void semsignal(int);
//--------------------

void handler();
int FIN = 0;             /* Para el cierre ordenado */
void finalizar(){ FIN = 1; }

int main(argc, argv)
int argc;
char *argv[];
{
  
    int s_TCP, s_UDP;		/* connected socket descriptor */
    int ls_TCP;				/* listen socket descriptor */
    
    int cc;				    /* contains the number of bytes read */
     
    struct sigaction sa = {.sa_handler = SIG_IGN}; /* used to ignore SIGCHLD */
    
    struct sockaddr_in myaddr_in;	/* for local socket address */
    struct sockaddr_in clientaddr_in;	/* for peer socket address */
	int addrlen;
	
    fd_set readmask;
    int numfds,s_mayor;
    
    char buffer[BUFFERSIZE];	/* buffer for packets to be read into */
    
    struct sigaction vec;

		/* Create the listen socket. */
	ls_TCP = socket (AF_INET, SOCK_STREAM, 0);
	if (ls_TCP == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket TCP\n", argv[0]);
		exit(1);
	}
	/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
   	memset ((char *)&clientaddr_in, 0, sizeof(struct sockaddr_in));

    //=======CARGAR DICCIONARIO Y MOSTRAR==========
    numPreguntas = cargarDiccionario(preguntas);
    //mostrar(preguntas,numPreguntas);
    //=========================


	//creacion del semaforo 
	semID = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);	

	if(semID == -1)
		{
			printf("Error al crear el lote de semaforos");
			exit(1);
		}

	//inicioSemaforo(memPunt->semID ,SEM_BUZON, 1)==-1)	
    
	semf.val = 1;

	if (semctl(semID, SEMAFORO, SETVAL, semf)==-1)
	{
		printf("No se ha podido establecer el valor del semáforo");
		exit(1);
	}	
    
    addrlen = sizeof(struct sockaddr_in);

		/* Set up address structure for the listen socket. */
	myaddr_in.sin_family = AF_INET;
		/* The server should listen on the wildcard address,
		 * rather than its own internet address.  This is
		 * generally good practice for servers, because on
		 * systems which are connected to more than one
		 * network at once will be able to have one server
		 * listening on all networks at once.  Even when the
		 * host is connected to only one network, this is good
		 * practice, because it makes the server program more
		 * portable.
		 */
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	myaddr_in.sin_port = htons(PUERTO);

	/* Bind the listen address to the socket. */
	if (bind(ls_TCP, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to bind address TCP\n", argv[0]);
		exit(1);
	}
		/* Initiate the listen on the socket so remote users
		 * can connect.  The listen backlog is set to 5, which
		 * is the largest currently supported.
		 */
	if (listen(ls_TCP, 5) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to listen on socket\n", argv[0]);
		exit(1);
	}
	
	
	/* Create the socket UDP. */
	s_UDP = socket (AF_INET, SOCK_DGRAM, 0);
	if (s_UDP == -1) {
		perror(argv[0]);
		printf("%s: unable to create socket UDP\n", argv[0]);
		exit(1);
	   }
	/* Bind the server's address to the socket. */
	if (bind(s_UDP, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		printf("%s: unable to bind address UDP\n", argv[0]);
		exit(1);
	    }

		/* Now, all the initialization of the server is
		 * complete, and any user errors will have already
		 * been detected.  Now we can fork the daemon and
		 * return to the user.  We need to do a setpgrp
		 * so that the daemon will no longer be associated
		 * with the user's control terminal.  This is done
		 * before the fork, so that the child will not be
		 * a process group leader.  Otherwise, if the child
		 * were to open a terminal, it would become associated
		 * with that terminal as its control terminal.  It is
		 * always best for the parent to do the setpgrp.
		 */
	setpgrp();

	switch (fork()) {
	case -1:		/* Unable to fork, for some reason. */
		perror(argv[0]);
		fprintf(stderr, "%s: unable to fork daemon\n", argv[0]);
		exit(1);

	case 0:     /* The child process (daemon) comes here. */

			/* Close stdin and stderr so that they will not
			 * be kept open.  Stdout is assumed to have been
			 * redirected to some logging file, or /dev/null.
			 * From now on, the daemon will not report any
			 * error messages.  This daemon will loop forever,
			 * waiting for connections and forking a child
			 * server to handle each one.
			 */
		fclose(stdin);
		fclose(stderr);

			/* Set SIGCLD to SIG_IGN, in order to prevent
			 * the accumulation of zombies as each child
			 * terminates.  This means the daemon does not
			 * have to make wait calls to clean them up.
			 */
		if ( sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror(" sigaction(SIGCHLD)");
            fprintf(stderr,"%s: unable to register the SIGCHLD signal\n", argv[0]);
            exit(1);
            }
            
		    /* Registrar SIGTERM para la finalizacion ordenada del programa servidor */
        vec.sa_handler = (void *) finalizar;
        vec.sa_flags = 0;
        if ( sigaction(SIGTERM, &vec, (struct sigaction *) 0) == -1) {
            perror(" sigaction(SIGTERM)");
            fprintf(stderr,"%s: unable to register the SIGTERM signal\n", argv[0]);
            exit(1);
            }
        
		while (!FIN) {
            /* Meter en el conjunto de sockets los sockets UDP y TCP */
            FD_ZERO(&readmask);
            FD_SET(ls_TCP, &readmask);
            FD_SET(s_UDP, &readmask);
            /* 
            Seleccionar el descriptor del socket que ha cambiado. Deja una marca en 
            el conjunto de sockets (readmask)
            */ 
    	    if (ls_TCP > s_UDP) s_mayor=ls_TCP;
    		else s_mayor=s_UDP;

            if ( (numfds = select(s_mayor+1, &readmask, (fd_set *)0, (fd_set *)0, NULL)) < 0) {
                if (errno == EINTR) {
                    FIN=1;
		            close (ls_TCP);
		            close (s_UDP);
                    perror("\nFinalizando el servidor. Se�al recibida en elect\n "); 
                }
            }
           else { 

                /* Comprobamos si el socket seleccionado es el socket TCP */
                if (FD_ISSET(ls_TCP, &readmask)) {
                    /* Note that addrlen is passed as a pointer
                     * so that the accept call can return the
                     * size of the returned address.
                     */
    				/* This call will block until a new
    				 * connection arrives.  Then, it will
    				 * return the address of the connecting
    				 * peer, and a new socket descriptor, s,
    				 * for that connection.
    				 */
    			s_TCP = accept(ls_TCP, (struct sockaddr *) &clientaddr_in, &addrlen);
    			if (s_TCP == -1) exit(1);
    			switch (fork()) {
        			case -1:	/* Can't fork, just exit. */
        				exit(1);
        			case 0:		/* Child process comes here. */
                    			close(ls_TCP); /* Close the listen socket inherited from the daemon. */
        				serverTCP(s_TCP, clientaddr_in);
        				exit(0);
        			default:	/* Daemon process comes here. */
        					/* The daemon needs to remember
        					 * to close the new accept socket
        					 * after forking the child.  This
        					 * prevents the daemon from running
        					 * out of file descriptor space.  It
        					 * also means that when the server
        					 * closes the socket, that it will
        					 * allow the socket to be destroyed
        					 * since it will be the last close.
        					 */
        				close(s_TCP);
        			}
             } /* De TCP*/
          /* Comprobamos si el socket seleccionado es el socket UDP */
          if (FD_ISSET(s_UDP, &readmask)) {
                /* This call will block until a new
                * request arrives.  Then, it will
                * return the address of the client,
                * and a buffer containing its request.
                * BUFFERSIZE - 1 bytes are read so that
                * room is left at the end of the buffer
                * for a null character.
                */
                cc = recvfrom(s_UDP, buffer, BUFFERSIZE - 1, 0,
                   (struct sockaddr *)&clientaddr_in, &addrlen);
                if ( cc == -1) {
                    perror(argv[0]);
                    printf("%s: recvfrom error\n", argv[0]);
                    exit (1);
                    }
                /* Make sure the message received is
                * null terminated.
                */
                buffer[cc]='\0';
                serverUDP (s_UDP, buffer, clientaddr_in);
                }
          }
		}   /* Fin del bucle infinito de atenci�n a clientes */
        /* Cerramos los sockets UDP y TCP */
        close(ls_TCP);
        close(s_UDP);
    
        printf("\nFin de programa servidor!\n");

		printf("Eliminando IPCS");
		
		if(semID!=-1){
			if(semctl(semID,0, IPC_RMID)==-1)
			{
				printf("Error: No se ha podido borrar el conjunto de semáforos.\n");
			}
		}

        
        
	default:		/* Parent process comes here. */
		exit(0);
	}

}



/*
 *				S E R V E R T C P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */
void serverTCP(int s, struct sockaddr_in clientaddr_in){

	FILE *l;
	 
	char codERROR_SINTAX[5]= "500";
	char codHOLA[5]= "250";
	char codRESP[5]= "354";
	char codACIERTO[5]= "350";
	char codCERRANDO_SERVICIO[5]= "221";
	char codSERVICIO_PREP[5]= "220";
	char codFALLO[5]= "375";
	char intentString[5];


	int reqcnt = 0;		/* keeps count of number of requests */
	char buf[BUFFERSIZE];	
	char rPruebas[TAM_BUFFER];	/* This example uses TAM_BUFFER byte messages. */
	char hostname[MAXHOST];	
	char hostnameIP[MAXHOST];	
	char puertoEfimero[50];	
	char cadenaLog[2064];/* remote host's name string */

	int len, len1, status;
    struct hostent *hp;		/* pointer to host info for remote host */
    long timevar;			/* contains time returned by time() */
    
    struct linger linger;	/* allow a lingering, graceful close; */
	char *newline ;
    char carRetorno[] = "\r\n";
	char palabra1[BUFFERSIZE];
	char palabra2[BUFFERSIZE];
	char palabraLog[BUFFERSIZE];
	char mensaje[BUFFERSIZE];
	int respuestaPregunta, preguntaElegida;
	int numeroRespuesta;
	int intentos = 0;
	int flagConexion = 0;
	int flagPregunta=0;
	int sintax=0;
	char instante[64];
	
	char *tokens[10];

	char fichLog[15]="peticiones.log";
    				            /* used when setting SO_LINGER */
	
	char conexionRed[BUFFERSIZE]="";	
    				
	/* Look up the host information for the remote host
	 * that we have connected with.  Its internet address
	 * was returned by the accept call, in the main
	 * daemon loop above.
	 */
	 
     status = getnameinfo((struct sockaddr *)&clientaddr_in,sizeof(clientaddr_in),
                           hostname,MAXHOST,NULL,0,0);
     if(status){
           	/* The information is unavailable for the remote
			 * host.  Just format its internet address to be
			 * printed out in the logging information.  The
			 * address will be shown in "internet dot format".
			 */
			 /* inet_ntop para interoperatividad con IPv6 */
            if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
            	perror(" inet_ntop \n");
             }
	 if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostnameIP, MAXHOST) == NULL)
            	perror(" inet_ntop \n");
    /* Log a startup message. */
    time (&timevar);
	//puertoEfimero= ntohs(clientaddr_in.sin_port);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("Startup from %s port %u at %s",
		hostname, ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));

		/* Set the socket for a lingering, graceful close.
		 * This will cause a final close of this socket to wait until all of the
		 * data sent on it has been received by the remote host.
		 */
	strcpy(conexionRed, "");
	strcpy(conexionRed,"[S]: ");
	strcat(conexionRed, codSERVICIO_PREP);
	strcat(conexionRed, " Servicio preparado");
//	memset(conexionRed,0,BUFFERSIZE);
	send(s, conexionRed, BUFFERSIZE, 0);

	strcpy(instante,(char *) ctime(&timevar));	
	strtok(instante,"\n");

	sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), conexionRed);
	
	semwait(SEMAFORO);

	if (NULL == (l = (fopen(fichLog, "a")))){
		fprintf(stderr, "No se ha podido abrir el fichero");
		semsignal(SEMAFORO);

	}
	fputs(cadenaLog, l);
	fclose(l);

	semsignal(SEMAFORO);

	memset(cadenaLog,0,2064);
	memset(instante,0 ,64);

	memset(conexionRed,0,BUFFERSIZE);

	linger.l_onoff  =1;
	linger.l_linger =1;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &linger,
					sizeof(linger)) == -1) {
		errout(hostname);

	}

		/* Go into a loop, receiving requests from the remote
		 * client.  After the client has sent the last request,
		 * it will do a shutdown for sending, which will cause
		 * an end-of-file condition to appear on this end of the
		 * connection.  After all of the client's requests have
		 * been received, the next recv call will return zero
		 * bytes, signalling an end-of-file condition.  This is
		 * how the server will know that no more requests will
		 * follow, and the loop will be exited.
		 */

	//	 printf("antes de while"); 
	while ((len = recv(s, buf, BUFFERSIZE, 0))) {

		memset(palabraLog,0,BUFFERSIZE);
		strcpy(palabraLog,buf);
		strtok(palabraLog, carRetorno);

		strcpy(instante,(char *) ctime(&timevar));	
		strtok(instante,"\n");

		sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Orden Recibida: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);
		
		semwait(SEMAFORO);

		if (NULL == (l = (fopen(fichLog, "a")))){
			fprintf(stderr, "No se ha podido abrir el fichero");
			semsignal(SEMAFORO);
		}
		fputs(cadenaLog, l);
		fclose(l);

		semsignal(SEMAFORO);

		memset(cadenaLog,0,2064);
		memset(instante,0 ,64);


		memset(palabra1,0, BUFFERSIZE);
		memset(palabra2,0, BUFFERSIZE);
		sintax=0;
		//printf(" %s",buf);
		if (len == -1) errout(hostname); 

		splitString(buf, tokens, " ");
		if(intentos==0){
			flagPregunta=0;
			respuestaPregunta=0;		
		}
		//comprobarSintaxPalabra1(token[0]);

		 if (tokens[0] != NULL && tokens[1] == NULL) {

			strtok(tokens[0], carRetorno);
			strcpy(palabra1,tokens[0]);
			

			sintax = comprobarSintaxTCP(palabra1);
			if(sintax==1){
				strcpy(mensaje,"");
				strcpy(mensaje,codERROR_SINTAX);
				strcat(mensaje, " Error de sintaxix");
				strcat(mensaje,carRetorno);
				send(s,mensaje,BUFFERSIZE,0);

				memset(palabraLog,0,BUFFERSIZE);
				strcpy(palabraLog,mensaje);
				strtok(palabraLog, carRetorno);

				strcpy(instante,(char *) ctime(&timevar));	
				strtok(instante,"\n");

				sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, REspuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);
		
				semwait(SEMAFORO);

				if (NULL == (l = (fopen(fichLog, "a")))){
					fprintf(stderr, "No se ha podido abrir el fichero");
					semsignal(SEMAFORO);
				}
				fputs(cadenaLog, l);
				fclose(l);

				semsignal(SEMAFORO);

				memset(cadenaLog,0,2064);
				memset(instante,0 ,64);


				memset(mensaje, 0, BUFFERSIZE);

			}else{

			
				if(flagConexion==0 && (strcmp(palabra1,"+")==0 || strcmp(palabra1,"RESPUESTA")==0)){
				
					strcpy(mensaje,"");
					strcpy(mensaje,codERROR_SINTAX);
					strcat(mensaje, " Error de sintaxix");
					strcat(mensaje,carRetorno);
					send(s,mensaje,BUFFERSIZE,0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, REspuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);

					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);	

					memset(mensaje, 0, BUFFERSIZE);
				//	break;
				}


				if(strcmp(palabra1,"HOLA")==0 && flagConexion == 1){

					strcpy(mensaje,codERROR_SINTAX);
					strcat(mensaje, "Error de sintaxix");
					strcat(mensaje,carRetorno);
					send(s,mensaje,BUFFERSIZE,0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, REspuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);
		
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);	

					memset(mensaje, 0, BUFFERSIZE);
					//break;

				}else if(strcmp(palabra1,"HOLA")==0 && flagConexion == 0){
					flagConexion = 1 ;
					flagPregunta=1;
					intentos = 5;
				
					srand(time(NULL));
					preguntaElegida = rand() % (numPreguntas + 1);				
					strcpy(mensaje, codHOLA);
					strcat(mensaje,"");
					strcat(mensaje,preguntas[preguntaElegida].pregunta);
					respuestaPregunta = preguntas[preguntaElegida].respuesta;
					
					sprintf(intentString, "%d", intentos);
					strcat(mensaje,"#");
					strcat(mensaje,intentString);

					strcat(mensaje,carRetorno);
					send(s, mensaje, BUFFERSIZE, 0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, REspuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);
		
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);	

					memset(mensaje, 0, BUFFERSIZE);
					//printf("Enviamos pregunta");
				}else if(strcmp(palabra1,"+") == 0 && intentos == 0 && flagConexion == 1){
					//Se puede enviar otra pregunta porque la pregunta anterior la acerto o se le acabaron los intentos
						flagPregunta=1;
						intentos = 5;

					preguntaElegida = rand() % (numPreguntas + 1);
					srand(time(NULL));
					strcpy(mensaje, codHOLA);
					strcat(mensaje,"");
					strcat(mensaje,preguntas[preguntaElegida].pregunta);
					respuestaPregunta = preguntas[preguntaElegida].respuesta;

					sprintf(intentString, "%d", intentos);
					strcat(mensaje,"#");
					strcat(mensaje,intentString);

					strcat(mensaje,carRetorno);
					send(s, mensaje, BUFFERSIZE, 0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, REspuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);
		
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);	

					memset(mensaje, 0, BUFFERSIZE);
					//printf("Enviamos pregunta");

				}else if(strcmp(palabra1,"+") == 0 && flagPregunta == 1 && intentos != 0 ){
					//NO Se puede enviar otra pregunta porque la pregunta anterior la no se acerto todavia o aun quedan intentos intentos
				
				
					strcpy(mensaje,"");
					strcpy(mensaje,codERROR_SINTAX);
					strcat(mensaje, " Error de sintaxix");
					strcat(mensaje,carRetorno);
					send(s,mensaje,BUFFERSIZE,0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, REspuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);
		
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);	

					memset(mensaje, 0, BUFFERSIZE);

				}else if(strcmp(palabra1,"ADIOS")==0){

					flagConexion = 0;
					strcpy(mensaje,"");
					strcat(mensaje,codCERRANDO_SERVICIO);
					strcat(mensaje, " Cerrando el servicio");
					strcat(mensaje,carRetorno);
					send(s,mensaje,BUFFERSIZE,0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, REspuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);
		
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);					
					memset(mensaje, 0, BUFFERSIZE);
				}
			}
			
		} else if (tokens[0] != NULL && tokens[1] != NULL  && tokens[2] == NULL ){

			strcpy(palabra1,tokens[0]);
		
			if(strcmp(palabra1,"RESPUESTA")==0 && flagPregunta == 1 && intentos != 0){

				intentos--;
				strtok(tokens[1], carRetorno);
				strcpy(palabra2,tokens[1]);


				numeroRespuesta = atoi(palabra2);
				//printf("transofma numero a entero %d",numeroRespuesta);
				if(numeroRespuesta==0){
					//printf("ENTRA ==0");
					strcpy(mensaje,"");
					strcpy(mensaje,codERROR_SINTAX);
					strcat(mensaje, " Error de sintaxix");
					strcat(mensaje,carRetorno);
					send(s,mensaje,BUFFERSIZE,0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, REspuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);
		
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);				
					memset(mensaje, 0, BUFFERSIZE);
				}else{

					if(numeroRespuesta<respuestaPregunta){
					//printf("ENTRA MAS");

					strcpy(mensaje,"");
					strcpy(mensaje,codRESP);
					strcat(mensaje,"");
					strcat(mensaje," MAYOR");
					sprintf(intentString, "%d", intentos);
					strcat(mensaje,"#");
					strcat(mensaje,intentString);
					strcat(mensaje,carRetorno);
					send(s,mensaje,BUFFERSIZE,0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, REspuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);
		
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);					
					memset(mensaje, 0, BUFFERSIZE);

					}else if(numeroRespuesta>respuestaPregunta){
						//("ENTRA MENOS");
						strcpy(mensaje,"");
						strcpy(mensaje,codRESP);
						strcat(mensaje,"");
						strcat(mensaje," MENOR");
						sprintf(intentString, "%d", intentos);
						strcat(mensaje,"#");
						strcat(mensaje,intentString);
						strcat(mensaje,carRetorno);
						send(s,mensaje,BUFFERSIZE,0);

						memset(palabraLog,0,BUFFERSIZE);
						strcpy(palabraLog,mensaje);
						strtok(palabraLog, carRetorno);

						strcpy(instante,(char *) ctime(&timevar));	
						strtok(instante,"\n");

						sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port),palabraLog);

			
						semwait(SEMAFORO);

						if (NULL == (l = (fopen(fichLog, "a")))){
							fprintf(stderr, "No se ha podido abrir el fichero");
							semsignal(SEMAFORO);
						}
						fputs(cadenaLog, l);
						fclose(l);

						semsignal(SEMAFORO);

						memset(cadenaLog,0,2064);
						memset(instante,0 ,64);

						memset(mensaje, 0, BUFFERSIZE);


					}else{

						//En caso de que la respuesta sea la correcta enviara menaje de acierto y cambiara el flagPregunta para que
						//no se puedan recibir mas respuestas 
						//printf("ENTRA ACIERTO");
						flagPregunta = 0;  
						strcpy(mensaje,"");
						strcpy(mensaje,codACIERTO);
						strcat(mensaje,"");
						strcat(mensaje," ACIERTO");
						strcat(mensaje,carRetorno);
						send(s,mensaje,BUFFERSIZE,0);

						memset(palabraLog,0,BUFFERSIZE);
						strcpy(palabraLog,mensaje);
						strtok(palabraLog, carRetorno);

						strcpy(instante,(char *) ctime(&timevar));	
						strtok(instante,"\n");

						sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port),palabraLog);

			
						semwait(SEMAFORO);

						if (NULL == (l = (fopen(fichLog, "a")))){
							fprintf(stderr, "No se ha podido abrir el fichero");
							semsignal(SEMAFORO);
						}
						fputs(cadenaLog, l);
						fclose(l);

						semsignal(SEMAFORO);

						memset(cadenaLog,0,2064);
						memset(instante,0 ,64);					
						memset(mensaje, 0, BUFFERSIZE);


					}

				}


			}else{
				// O respuesta esta mal escrito o
				//Cuando se recibe una RESPUESTA y ya no se tienen mas intentos o ya se ha acertado la pregunta
				//La unca manera sera enviando un "+" y reestableciodo el flagPregunta a 1 para poder seguir respondiendo

				//printf("RESPONDIO sin tener intentos o despues de acertar");
				
				
				if(intentos == 0){

					strcpy(mensaje,"");
					strcpy(mensaje,codFALLO);
					strcat(mensaje, " FALLO");
					strcat(mensaje,carRetorno);
					send(s,mensaje,BUFFERSIZE,0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port),palabraLog);
			
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);			

					memset(mensaje, 0, BUFFERSIZE);

					flagPregunta=0;
				}else{
				
					strcpy(mensaje,"");
					strcpy(mensaje,codERROR_SINTAX);
					strcat(mensaje, " Error de sintaxix");
					strcat(mensaje,carRetorno);
					send(s,mensaje,BUFFERSIZE,0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port),palabraLog);
			
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);			

					memset(mensaje, 0, BUFFERSIZE);
				}

			}

			
			//printf("Hay dos palabras.\n");
       		 // tokens[0] y tokens[1] contienen las dos palabras
   		}else {


					strcpy(mensaje,"");
					strcpy(mensaje,codERROR_SINTAX);
					strcat(mensaje, " Error de sintaxix");
					strcat(mensaje,carRetorno);
					send(s,mensaje,BUFFERSIZE,0);

					memset(palabraLog,0,BUFFERSIZE);
					strcpy(palabraLog,mensaje);
					strtok(palabraLog, carRetorno);

					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");

					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, REspuesta enviada: %s \n", instante, hostname, hostnameIP, "TCP", ntohs(clientaddr_in.sin_port), palabraLog);
			
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);
					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);					
					memset(mensaje, 0, BUFFERSIZE);

				
				
  		}
		
				memset(buf, 0, BUFFERSIZE);



	
	}		
			
	close(s);

		/* Log a finishing message. */
	time (&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("Completed %s port %u, %d requests, at %s\n",
		hostname, ntohs(clientaddr_in.sin_port), reqcnt, (char *) ctime(&timevar));
}






/*
 *				S E R V E R U D P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */

void serverUDP(int s, char * buffer, struct sockaddr_in clientaddr_in){	//printf("ENtra en UDP");

	char codERROR_SINTAX[5]= "500";
	char codHOLA[5]= "250";
	char codRESP[5]= "354";
	char codACIERTO[5]= "350";
	char codCERRANDO_SERVICIO[5]= "221";
	char codSERVICIO_PREP[5]= "220";
	char intentString[5];

	//FICHERO LOG
	char fichLog[15]="peticiones.log";
	char instante[64];
	char cadenaLog[2064];
	FILE *l;
	char hostnameIP[MAXHOST];


     struct in_addr reqaddr;	/* for requested host's address */
     struct hostent *hp;		/* pointer to host info for requested host */
     struct linger linger; 
     struct addrinfo hints, *res;
	 struct sigaction alarma;

	 int nc, errcode;
	 int addrlen;
	 long timevar;
    
	int len,status;
	char hostname[MAXHOST];		
   	 

	char carRetorno[] = "\r\n";
	char palabra1[BUFFERSIZE];
	char palabra2[BUFFERSIZE];
	char mensaje[BUFFERSIZE];
	char buf[BUFFERSIZE];
	char conexionRed[BUFFERSIZE];


	int respuestaPregunta, preguntaElegida;
	int numeroRespuesta;
	int intentos = 0;
	int flagConexion = 0;
	int flagPregunta=0;
	int sintax=0;
	
	char *tokens[10];
	
	
	 
	addrlen = sizeof(struct sockaddr_in);

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET;

	time(&timevar);
	//printf("Esto es lo que hay en buffer %s\n",buffer);
	//printf("Startup port %u at %s", ntohs(clientaddr_in.sin_port), (char *)ctime(&timevar));


	linger.l_onoff = 1;
	linger.l_linger = 1;


	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) == -1){
		errout(hostname);
	}
	/* Registrar SIGALRM para no quedar bloqueados en los recvfrom */
	alarma.sa_handler = (void *)handler;
	alarma.sa_flags = 0;
	if (sigaction(SIGALRM, &alarma, (struct sigaction *)0) == -1)
	{
		perror(" sigaction(SIGALRM)");
		fprintf(stderr, "Unable to register the SIGALRM signal\n");
		exit(1);
	}

	status = getnameinfo((struct sockaddr *)&clientaddr_in, sizeof(clientaddr_in), hostname, MAXHOST, NULL, 0, 0);
	if (status)
	{
		/* The information is unavailable for the remote
			 * host.  Just format its internet address to be
			 * printed out in the logging information.  The
			 * address will be shown in "internet dot format".
			 */
		/* inet_ntop para interoperatividad con IPv6 */
		if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
			perror(" inet_ntop \n");

			
	}

	memset(hostnameIP,0,MAXHOST);
	if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostnameIP, MAXHOST) == NULL)
       	 perror(" inet_ntop \n");
	



	do{

		memset(palabra1,0, BUFFERSIZE);
		memset(palabra2,0, BUFFERSIZE);
		//printf(" %s",buf);
		if (len == -1) errout(hostname); 

		splitString(buffer, tokens, " ");
		if(intentos==0){
			flagPregunta=0;
			respuestaPregunta=0;		
		}
		//comprobarSintaxPalabra1(token[0]);

		if (tokens[0] != NULL && tokens[1] == NULL){

			

			strtok(tokens[0], carRetorno);
			strcpy(palabra1,tokens[0]);

			sintax = comprobarSintaxUDP(palabra1);

			if(sintax==1){
				strcpy(mensaje,"");
				strcpy(mensaje,codERROR_SINTAX);
				strcat(mensaje, " Error de sintaxix");
				strcat(mensaje,carRetorno);
				if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
						printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);	
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================
				memset(mensaje, 0, BUFFERSIZE);
			}else{

					if(strcmp(palabra1, "READY") == 0){
					strcpy(conexionRed, "");
					strcpy(conexionRed,"[S]: ");
					strcat(conexionRed, codSERVICIO_PREP);
					strcat(conexionRed, " Servicio preparado");
					strcat(conexionRed,carRetorno);
					if (sendto(s, conexionRed, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE){
						//error(1);
						// if (NULL == (g = (fopen(pEfimero, "a"))))
						// 	fprintf(stderr, "No se ha podido abrir el fichero");
						// fprintf(g, "%s: unable to send request\n", argv[0]);
						// fclose(g);
					}
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(conexionRed,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), conexionRed);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================
					
					memset(buf,0,BUFFERSIZE);
					memset(conexionRed,0,BUFFERSIZE);
				}
				
				if(flagConexion==0 && (strcmp(palabra1,"+")==0 || strcmp(palabra1,"RESPUESTA")==0)){
				
					strcpy(mensaje,"");
					strcpy(mensaje,codERROR_SINTAX);
					strcat(mensaje, " Error de sintaxix");
					strcat(mensaje,carRetorno);

					if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
						printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);	
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

					memset(mensaje, 0, BUFFERSIZE);
				//	break;
				}else if(strcmp(palabra1,"HOLA")==0 && flagConexion == 1){

					strcpy(mensaje,codERROR_SINTAX);
					strcat(mensaje, "Error de sintaxix");
					strcat(mensaje,carRetorno);

					if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
						printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);		
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

					memset(mensaje, 0, BUFFERSIZE);
					//break;

				}else if(strcmp(palabra1,"HOLA")==0 && flagConexion == 0){
					flagConexion = 1 ;
					flagPregunta=1;
					intentos = 5;
				
					preguntaElegida = rand() % numPreguntas + 1;
					strcpy(mensaje, codHOLA);
					strcat(mensaje,"");
					strcat(mensaje,preguntas[preguntaElegida].pregunta);
					respuestaPregunta = preguntas[preguntaElegida].respuesta;

					sprintf(intentString, "%d", intentos);
					strcat(mensaje,"#");
					strcat(mensaje,intentString);

					strcat(mensaje,carRetorno);
					if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
						printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);		
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

					memset(mensaje, 0, BUFFERSIZE);
					//printf("Enviamos pregunta");
				}else if(strcmp(palabra1,"+") == 0 && flagPregunta == 0 && flagConexion == 1){
					//Se puede enviar otra pregunta porque la pregunta anterior la acerto o se le acabaron los intentos
						flagPregunta=1;
						intentos = 5;
				
					preguntaElegida = rand() % numPreguntas + 1;
					strcpy(mensaje, codHOLA);
					strcat(mensaje,"");
					strcat(mensaje,preguntas[preguntaElegida].pregunta);
					respuestaPregunta = preguntas[preguntaElegida].respuesta;

					sprintf(intentString, "%d", intentos);
					strcat(mensaje,"#");
					strcat(mensaje,intentString);

					strcat(mensaje,carRetorno);

					if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
						printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);	
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================
					
					memset(mensaje, 0, BUFFERSIZE);
					//printf("Enviamos pregunta");

				}else if(strcmp(palabra1,"+") == 0 && flagPregunta == 1 ){
					//NO Se puede enviar otra pregunta porque la pregunta anterior la no se acerto todavia o aun quedan intentos intentos
				
				
					strcpy(mensaje,"");
					strcpy(mensaje,codERROR_SINTAX);
					strcat(mensaje, " Error de sintaxix");
					strcat(mensaje,carRetorno);

					if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
						printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);		
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

					memset(mensaje, 0, BUFFERSIZE);

				}else if(strcmp(palabra1,"ADIOS")==0){

					flagConexion = 0;
					flagPregunta = 0;
					strcpy(mensaje,"");
					strcpy(mensaje,codCERRANDO_SERVICIO);
					strcat(mensaje, " Cerrando el servicio");
					strcat(mensaje,carRetorno);

					if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
						printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);	
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

					memset(mensaje, 0, BUFFERSIZE);
				//	break;
				}
			


			}

				
		}else if (tokens[0] != NULL && tokens[1] != NULL  && tokens[2] == NULL ){

			strcpy(palabra1,tokens[0]);
		
			if(strcmp(palabra1,"RESPUESTA")==0 && flagPregunta == 1){

				intentos--;
				strtok(tokens[1], carRetorno);
				strcpy(palabra2,tokens[1]);


				numeroRespuesta = atoi(palabra2);
				//printf("transofma numero a entero %d",numeroRespuesta);
				if(numeroRespuesta==0){
				//	printf("ENTRA ==0");
					strcpy(mensaje,"");
					strcpy(mensaje,codERROR_SINTAX);
					strcat(mensaje, " Error de sintaxix");
					strcat(mensaje,carRetorno);

					if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
      	 			printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);	
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

					memset(mensaje, 0, BUFFERSIZE);
				}else{

					if(numeroRespuesta<respuestaPregunta){
					//printf("ENTRA MAS");

					strcpy(mensaje,"");
					strcpy(mensaje,codRESP);
					strcat(mensaje,"");
					strcat(mensaje," MAYOR");

					sprintf(intentString, "%d", intentos);
					strcat(mensaje,"#");
					strcat(mensaje,intentString);
				
					strcat(mensaje,carRetorno);

					if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
      	 			printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);		
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);	
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);									
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

					memset(mensaje, 0, BUFFERSIZE);

					}else if(numeroRespuesta>respuestaPregunta){
						//printf("ENTRA MENOS");
						strcpy(mensaje,"");
						strcpy(mensaje,codRESP);
						strcat(mensaje,"");
						strcat(mensaje," MENOR");

						sprintf(intentString, "%d", intentos);
						strcat(mensaje,"#");
						strcat(mensaje,intentString);
						
						strcat(mensaje,carRetorno);

						if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
      	 			printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);	
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

						memset(mensaje, 0, BUFFERSIZE);


					}else{

						//En caso de que la respuesta sea la correcta enviara menaje de acierto y cambiara el flagPregunta para que
						//no se puedan recibir mas respuestas 
						//printf("ENTRA ACIERTO");
						flagPregunta = 0;  
						strcpy(mensaje,"");
						strcpy(mensaje,codACIERTO);
						strcat(mensaje,"");
						strcat(mensaje," ACIERTO");
						strcat(mensaje,carRetorno);

						if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
      	 			printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);		
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

						memset(mensaje, 0, BUFFERSIZE);


					}

				}


			}else{

				//Cuando se recibe una RESPUESTA y ya no se tienen mas intentos o ya se ha acertado la pregunta
				//La unca manera sera enviando un "+" y reestableciodo el flagPregunta a 1 para poder seguir respondiendo

			//	printf("RESPONDIO sin tener intentos o despues de acertar");
				strcpy(mensaje,"");
				strcpy(mensaje,codERROR_SINTAX);
				strcat(mensaje, " Error de sintaxix");
				strcat(mensaje,carRetorno);

				if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
      	 			printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);		
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

				memset(mensaje, 0, BUFFERSIZE);

			}

			
		//	printf("Hay dos palabras.\n");
       		 // tokens[0] y tokens[1] contienen las dos palabras
   		} else {
				strcpy(mensaje,"");
				strcpy(mensaje,codERROR_SINTAX);
				strcat(mensaje, " Error de sintaxix");
				strcat(mensaje,carRetorno);
				
				if (sendto(s, mensaje, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) != BUFFERSIZE)
      	 			printf("ERROR EL ENVIAR EL MENSAJE");
						//error(1);	
				//=======================================================================
					strcpy(instante,(char *) ctime(&timevar));	
					strtok(instante,"\n");
					strtok(mensaje,carRetorno);
					sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Respuesta enviada: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), mensaje);
					
					semwait(SEMAFORO);

					if (NULL == (l = (fopen(fichLog, "a")))){
						fprintf(stderr, "No se ha podido abrir el fichero");
						semsignal(SEMAFORO);

					}
					fputs(cadenaLog, l);
					fclose(l);

					semsignal(SEMAFORO);

					memset(cadenaLog,0,2064);
					memset(instante,0 ,64);

					memset(conexionRed,0,BUFFERSIZE);
			    //=======================================================================

				memset(mensaje, 0, BUFFERSIZE);
  		}	


	//	if (flagSocket == 0)
				//{
					memset(buffer,0,BUFFERSIZE);
					alarm(TIMEOUT);
					//printf("EWNTRA A RECV");
					//Recibe y vuelve a enviar al empezar el wh
					if (recvfrom(s, buffer, BUFFERSIZE, 0, (struct sockaddr *)&clientaddr_in, &addrlen) == -1)
					{
						if (errno == EINTR)
							break;
						else
						{
							printf("Unable to get response from");
							exit(1);
						}
					}

				//=======================================================================
						strtok(buffer,carRetorno);
						strcpy(instante,(char *) ctime(&timevar));	
						strtok(instante,"\n");

						sprintf(cadenaLog, "Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %u, Orden Recibida: %s \n", instante, hostname, hostnameIP, "UDP", ntohs(clientaddr_in.sin_port), buffer);
						
						semwait(SEMAFORO);

						if (NULL == (l = (fopen(fichLog, "a")))){
							fprintf(stderr, "No se ha podido abrir el fichero");
							semsignal(SEMAFORO);
						}
						fputs(cadenaLog, l);
						fclose(l);

						semsignal(SEMAFORO);

						memset(cadenaLog,0,2064);
						memset(instante,0 ,64);
			    //=======================================================================
					
					//strcpy(mensaje, buffer);
					
				//}
				//else
				//cle	break;
			

	}while(1);
}


// Funcion que necesita el SIGACTION

void handler()
{
	//printf("No more messages to receive\n");
}


/*
 *	This routine aborts the child process attending the client.
 */
void errout(char *hostname)
{
	printf("Connection with %s aborted on error\n", hostname);
	exit(1);     
}

//============FUNCION DE CARGAR PREGUNTAS=========================
int cargarDiccionario(Pregunta *preguntas){
    FILE *archivo;
    archivo = fopen("preguntas.txt", "r"); // Abre el archivo en modo lectura
    char linea[100];
    if (archivo == NULL) {
        perror("Error al abrir el archivo de preguntas\n");
        exit(EXIT_FAILURE);
    }
    int numPreguntas = 0; // PAra luego hacer el random

     while(fgets(linea, sizeof(linea), archivo) != NULL){
        char *token = strtok(linea, "#"); // Divide la línea en pregunta y respuesta usando '#'
        
        // Copia la pregunta a la estructura
        strncpy(preguntas[numPreguntas].pregunta, token, sizeof(preguntas[numPreguntas].pregunta));
        
        // Lee la respuesta como entero y la almacena en la estructura
        preguntas[numPreguntas].respuesta = atoi(strtok(NULL, "#"));

         numPreguntas++; // Incrementa el contador de preguntas
    }
    fclose(archivo);
    return numPreguntas;
        
}//END FUNCION
//============FIN FUNCION DE CARGAR PREGUNTAS=========================


//===================FUNCION TEMPORAL PARA COMPROBAR QUE FUNCIONA===========================
void mostrar(Pregunta *preguntas, int numPreguntas){
    int i;
    for (i = 0; i < numPreguntas; i++) {
        printf("Pregunta[%d]: %s\n",i, preguntas[i].pregunta);
        printf("Respuesta[%d]: %d\n",i, preguntas[i].respuesta);
    }
    
}
//==================================================================



void splitString(char *input, char **tokens, const char *delimiter) {
    // Utiliza strtok para dividir la cadena en tokens
    char *token = strtok(input, delimiter);
    int i = 0;

    // Almacena los tokens en el array
    while (token != NULL && i < 10) {
        tokens[i++] = token;
        token = strtok(NULL, delimiter);
    }

    // Asegúrar de que el último elemento del array sea NULL
    tokens[i] = NULL;
}
int comprobarSintaxTCP(char* cadena){
	//printf("ESTA ES LA CADENA QUE LE LLEGA A COMPROBAR SINTAX %s",cadena);

	if(strcmp(cadena,"HOLA") == 0 || strcmp(cadena,"ADIOS")== 0 || strcmp(cadena,"+")== 0){
		return 0;
		
	}else{
		return 1;
	}


}
int comprobarSintaxUDP(char* cadena){
	//printf("ESTA ES LA CADENA QUE LE LLEGA A COMPROBAR SINTAX %s",cadena);

	if(strcmp(cadena,"HOLA") == 0 || strcmp(cadena,"ADIOS")== 0 || strcmp(cadena,"+")== 0 || strcmp(cadena,"READY")== 0){
		return 0;
		
	}else{
		return 1;
	}


}

void semsignal(int numsem){
 		struct sembuf s;

 		s.sem_num = numsem;
 		s.sem_op = 1;
 		s.sem_flg = SEM_UNDO;

 		semop(semID,&s, 1);
 		return;
}

void semwait(int numsem){
 		struct sembuf s;

 		s.sem_num = numsem;
 		s.sem_op = -1;
 		s.sem_flg = SEM_UNDO;

 		semop(semID,&s, 1);
 		return;
		
}


//Orden recibida: nombre del host, dirección IP, protocolo de transporte, el puerto del cliente y la orden.
//fevhaHora (&timevar)|| NOMBre del host (hostname)|| IP del host hostnameIP, PROTOCOLO DE TRANSPORTE protocolo, numero de pEfimero
// char crearLineaLog(char time ,char hostname, char hostnameIP, char protocolo, char pEfimero, char orden){
// 	char cadena[256];
//     sprintf(cadena, " Time: %s, Hostname: %s, Hostname IP: %s, Protocolo: %s, Puerto Efímero: %s, Orden: %s",time, hostname, hostnameIP, protocolo, pEfimero, orden);
// 	return cadena;

// }