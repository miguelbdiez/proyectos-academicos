
 /*
** Fichero: cliente.c
** Autores:
** Miguel Barranquero Diez
** Alvaro Barcala Martín
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/errno.h>


#define _GNU_SOURCE
#define _XOPEN_SOURCE
/*servidor udp*/
extern int errno;

#define ADDRNOTFOUND	0xffffffff	/* value returned for unknown host */
#define RETRIES	5		/* number of times to retry before givin up */
#define BUFFERSIZE	1024	/* maximum size of packets to be received */
#define PUERTO 17278
#define TIMEOUT 6
#define MAXHOST 512

//////
#define PUERTO 17278


void TCP(FILE*, int, char**);
void UDP(FILE*, int, char**);

/*
 *			H A N D L E R
 *
 *	This routine is the signal handler for the alarm signal.
 */
void handler()
{
 printf("Alarma recibida \n");
}
/*
 *			M A I N
 *
 *	This routine is the client which request service from the remote.
 *	It creates a connection, sends a number of
 *	requests, shuts down the connection in one direction to signal the
 *	server about the end of data, and then receives all of the responses.
 *	Status will be written to stdout.
 *
 *	The name of the system to which the requests will be sent is given
 *	as a parameter to the command.
 */
int main(int argc, char *argv[]){
 FILE *fich;
    //COMPROBACION DE ARGUMENTOS
	if(argc!=4 || strcmp(argv[2],"TCP") && (strcmp(argv[2],"UDP"))){
		fprintf(stderr, "Numero de argumentos erroneos, USO:\n");
		fprintf(stderr, "%s [Nombre servidor] [TCP | UDP] [Fichero de ordenes.txt]\n", argv[0]);
		exit(1);
	}

	fich = fopen(argv[3],"r");
	if(fich== NULL){
		printf("ERROR en la apertura  del fichero de ordenes");
		exit(1);
	}

	//COMPROBACION LANZAR UDP O TCP
	if(strcmp(argv[2], "TCP") == 0){
		TCP(fich,argc,argv);
	}else{
		UDP(fich,argc,argv);
	}
	fclose(fich);
	return(0);

}

/*
 *			C L I E N T C P
 *
 *	This is an example program that demonstrates the use of
 *	stream sockets as an IPC mechanism.  This contains the client,
 *	and is intended to operate in conjunction with the server
 *	program.  Together, these two programs
 *	demonstrate many of the features of sockets, as well as good
 *	conventions for using these features.
 *
 *
 */
void TCP(FILE *f, int argc, char *argv[]){

	char pEfimero[100];//nombre del fichero donde se guarda el proceso
	FILE *g; 


    int s;				/* connected socket descriptor */
   	struct addrinfo hints, *res;
    long timevar;			/* contains time returned by time() */
    struct sockaddr_in myaddr_in;	/* for local socket address */
    struct sockaddr_in servaddr_in;	/* for server socket address */
	int addrlen, i, j, errcode,len ;
	

	char respuesta[BUFFERSIZE]; 
	char linea[256]; // Suponiendo que cada línea tiene como máximo 255 caracteres
	char lineaAux[256]; 
	char fech_hora_inicio[256];
	char fech_hora_final[256];
    
	//char respuesta[BUFFERSIZE];
/* Create the socket. */
	s = socket (AF_INET, SOCK_STREAM, 0);
	if (s == -1) {

		//fichero que necesita un semaforo pues todos los clientes pueden acceder a el y procucirse acoplamientos
		 if (NULL == (g = (fopen("depuracion.txt", "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero socket");
        fprintf(g, "%s: unable to create socket\n", argv[0]);
        fclose(g);
        exit(1);
	}
	
	/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

	/* Set up the peer address to which we will connect. */
	servaddr_in.sin_family = AF_INET;
	
	/* Get the host information for the hostname that the
	 * user passed in. */
      memset (&hints, 0, sizeof (hints));
      hints.ai_family = AF_INET;
 	 /* esta función es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta*/
    errcode = getaddrinfo (argv[1], NULL, &hints, &res); 
    if (errcode != 0){
			/* Name was not found.  Return a
			 * special value signifying the error. */
		  if (NULL == (g = (fopen("depuracion.txt", "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");
        fprintf(g, "%s: Couldn't resolve IP for %s\n", argv[0], argv[1]);
        fclose(g);
        exit(1);
        }
    else {
		/* Copy address of host */
		servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
	    }
    freeaddrinfo(res);

    /* puerto del servidor en orden de red*/
	servaddr_in.sin_port = htons(PUERTO);

		/* Try to connect to the remote server at the address
		 * which was just built into peeraddr.
		 */
	if (connect(s, (const struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);

		  if (NULL == (g = (fopen("depuracion.txt", "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

        fprintf(g, "%s: Couldn't resolve IP for %s\n", argv[0], argv[1]);
        fclose(g);

        exit(1);
	}
		/* Since the connect call assigns a free address
		 * to the local end of this connection, let's use
		 * getsockname to see what it assigned.  Note that
		 * addrlen needs to be passed in as a pointer,
		 * because getsockname returns the actual length
		 * of the address.
		 */
	addrlen = sizeof(struct sockaddr_in);
	if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
		perror(argv[0]);
		
		if (NULL == (g = (fopen("depuracion.txt", "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");
			
        fprintf(g, "%s: Couldn't resolve IP for %s\n", argv[0], argv[1]);
        fclose(g);

		 exit(1);
	 }

	/* Print out a startup message for the user. */
	time(&timevar);
	/* The port number must be converted first to host byte
	 * order before printing.  On most hosts, this is not
	 * necessary, but the ntohs() call is included here so
	 * that this program could easily be ported to a host
	 * that does require it.
	 */

	strcpy(pEfimero, "");
    sprintf(pEfimero, "%u", ntohs(myaddr_in.sin_port));
    strcat(pEfimero, "[TCP].txt");

	sprintf(fech_hora_inicio,"Connected to %s on port %u at %s",argv[1], ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));

	if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

    fputs(fech_hora_inicio,g );
    fclose(g);



	 if (-1 == (recv(s, respuesta, BUFFERSIZE, 0))){

        if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

        fprintf(g, "%s: error reading result\n", argv[0]);
        fclose(g);
        exit(1);
    }
	strcat(respuesta,"\n");
	if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

    fputs(respuesta,g );
    fclose(g);
        
   // printf("[S]: %s\n", respuesta);
    memset(respuesta, 0, BUFFERSIZE);

	while (fgets(linea, sizeof(linea), f) != NULL) {
            // Elimina el salto de línea al final de la línea leída, si existe

			if(strcmp(linea,"ADIOS") == 0){	
				strcpy(lineaAux,"");
				strcpy(lineaAux,"[C]: ");
				strcat(lineaAux,linea);
				strcat(lineaAux,"\n");

			}else{
				strcpy(lineaAux,"");
				strcpy(lineaAux,"[C]: ");
				strcat(lineaAux,linea);
			}
		

		if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

    	fputs(lineaAux,g);
    	fclose(g);

		printf("%s", lineaAux);
           // linea[strcspn(linea, "\n")] = '\0';
            
        // Concatena "\r\n" al final de la línea
        strcat(linea, "\r\n");
            


//ENVIAMOS LA ORDEN DEL FICHERO

		len = send(s,linea,strlen(linea),0);
		if(len != strlen(linea)){
			if (NULL == (g = (fopen(pEfimero, "a"))))
				fprintf(stderr, "No se ha podido abrir el fichero");

			fprintf(g, "%s: Connection aborted on error\nMessage was: %s\nLength returned by send() was: %d\n", argv[0], linea, len);
			fclose(g);
		
			exit(1);
		}

		strcpy(linea, "");
		strcpy(lineaAux,"");

		

		//RECEPCION DE LA RESPUESTA DEL SERVIDOR

		 memset(respuesta, 0, BUFFERSIZE);

            if (-1 == (recv(s, respuesta, BUFFERSIZE, 0)))
            {
                if (NULL == (g = (fopen(pEfimero, "a"))))
                    fprintf(stderr, "No se ha podido abrir el fichero");

                fprintf(g, "%s: error reading result\n", argv[0]);
                fclose(g);

                exit(1);
            }
			strcpy(lineaAux, "[S]: ");
			strcat(lineaAux,respuesta);

			if (NULL == (g = (fopen(pEfimero, "a"))))
            	fprintf(stderr, "No se ha podido abrir el fichero");

    		fputs(lineaAux,g );
    		fclose(g);


         printf("%s", lineaAux);
            // Guardamos el mensaje de progreso en el fichero de puerto efimero
           /* if (NULL == (g = (fopen(puertoEfimero, "a"))))
                fprintf(stderr, "No se ha podido abrir el fichero");
            fputs(respuesta, g);
            fputs("\n", g);
            fclose(g);*/

            //Una vez operado ya todo lo necesario con respuesta

        }

        
      //  fclose(f); // Cierra el archivo después de leerlo


		/* Now, shutdown the connection for further sends.
		 * This will cause the server to receive an end-of-file
		 * condition after it has received all the requests that
		 * have just been sent, indicating that we will not be
		 * sending any further requests.
		 */
	if (shutdown(s, 1) == -1) {
		perror(argv[0]);

		 if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

        fprintf(g, "%s: unable to shutdown socket\n", argv[0]);
        fclose(g);

        exit(1);
	}

		/* Now, start receiving all of the replys from the server.
		 * This loop will terminate when the recv returns zero,
		 * which is an end-of-file condition.  This will happen
		 * after the server has sent all of its replies, and closed
		 * its end of the connection.
	

     Print message indicating completion of task. */
	time(&timevar);
	sprintf(fech_hora_final,"All done at %s", (char *)ctime(&timevar));

	if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

    fputs(fech_hora_final,g );
    fclose(g);

	return;
	
}

	

void UDP(FILE *f, int argc, char *argv[]){

	char pEfimero[100];
	FILE *g; 

	char respuesta[BUFFERSIZE]; 
	char linea[256];

	char lineaAux[256]; 
	char fech_hora_inicio[256];
	char fech_hora_final[256];
	
	char buf[BUFFERSIZE];
	int len;

	int i, errcode;
	int retry = RETRIES;		/* holds the retry count */
    int s;				/* socket descriptor */
    long timevar;     
	                  /* contains time returned by time() */
    struct sockaddr_in myaddr_in;	/* for local socket address */
    struct sockaddr_in servaddr_in;	/* for server socket address */
    struct in_addr reqaddr;		/* for returned internet address */
   	struct addrinfo hints, *res;

    int	addrlen, n_retry;
    struct sigaction vec;
   	char hostname[MAXHOST];
   


	
		/* Create the socket. */
	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {

		 if (NULL == (g = (fopen("depuracion.txt", "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

        fprintf(g, "%s: unable to create socket\n", argv[0]);
        fclose(g);
        exit(1);
	}
	
    /* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));
	
			/* Bind socket to some local address so that the
		 * server can send the reply back.  A port number
		 * of zero will be used so that the system will
		 * assign any available port number.  An address
		 * of INADDR_ANY will be used so we do not have to
		 * look up the internet address of the local host.
		 */
	myaddr_in.sin_family = AF_INET;
	myaddr_in.sin_port = 0;
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {

		if (NULL == (g = (fopen("depuracion.txt", "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

        fprintf(g, "%s: unable to bind socket\n", argv[0]);
    	fclose(g);

        exit(1);
	   }
    addrlen = sizeof(struct sockaddr_in);
    if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
			
        if (NULL == (g = (fopen("depuracion.txt", "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");
			
        fprintf(g, "%s: Couldn't resolve IP for %s\n", argv[0], argv[1]);
        fclose(g);

		 exit(1);
    }

            /* Print out a startup message for the user. */
   
	/* Set up the server address. */
	servaddr_in.sin_family = AF_INET;
		/* Get the host information for the server's hostname that the
		 * user passed in.
		 */
      memset (&hints, 0, sizeof (hints));
      hints.ai_family = AF_INET;
 	 /* esta función es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta*/
    errcode = getaddrinfo (argv[1], NULL, &hints, &res); 
    if (errcode != 0){

			if (NULL == (g = (fopen("depuracion.txt", "a"))))
	            fprintf(stderr, "No se ha podido abrir el fichero");

		fprintf(g ," No es posible resolver la IP");
		fclose(g);

		exit(1);
      }
    else {
			/* Copy address of host */
		servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
	 }
     freeaddrinfo(res);
     /* puerto del servidor en orden de red*/
	 servaddr_in.sin_port = htons(PUERTO);

   /* Registrar SIGALRM para no quedar bloqueados en los recvfrom */
    vec.sa_handler = (void *) handler;
    vec.sa_flags = 0;
    if ( sigaction(SIGALRM, &vec, (struct sigaction *) 0) == -1) {
            perror(" sigaction(SIGALRM)");

			if (NULL == (g = (fopen("depuracion.txt", "a"))))
	            fprintf(stderr, "No se ha podido abrir el fichero");

            fprintf(g,"%s: unable to register the SIGALRM signal\n", argv[0]);
			fclose(g);

            exit(1);
        }

	time(&timevar);
        

	strcpy(pEfimero, "");
    sprintf(pEfimero, "%u", ntohs(myaddr_in.sin_port));
    strcat(pEfimero, "[UDP].txt");
    /* The port number must be converted first to host byte
             * order before printing.  On most hosts, this is not
             * necessary, but the ntohs() call is included here so
             * that this program could easily be ported to a host
             * that does require it.
             */
sprintf(fech_hora_inicio,"Connected to %s on port %u at %s",argv[1], ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));

	if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

    fputs(fech_hora_inicio,g );
    fclose(g);
	

//Mensaje de inicio SERVICIOS PREPARADOS
//enviamos un mensaje especial en buf = "READY";

	strcpy(buf,"READY\r\n");
	if (sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) != strlen(buf)){
      	printf("ERROR envio");
		// error(1);
             /*if (NULL == (g = (fopen(puertoEfimero, "a"))))
                        fprintf(stderr, "No se ha podido abrir el fichero");
                    fprintf(g, "%s: unable to send request\n", argv[0]);
                    fclose(g);*/
    }
	alarm(TIMEOUT);
	
	memset(respuesta, 0, BUFFERSIZE);
	if ((len = recvfrom(s, respuesta, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in, &addrlen)) == -1){
   		if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

        fprintf(g,"Unable to get response from.\n");
    	fclose(g);

        exit(1);
        
    }else{

        alarm(0); //Cancelamos la alarma

	

       // exit(1);
    }
	//strcat(respuesta,"\n");
	if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(g, "No se ha podido abrir el fichero");

    fputs(respuesta,g );
    fclose(g);
                              // Mostramos la respuesta
    printf("[S]: %s", respuesta);
          
    

	while (fgets(linea, sizeof(linea), f) != NULL){
		memset(respuesta,0,BUFFERSIZE);
		n_retry=RETRIES;
		if(strcmp(linea,"ADIOS") == 0){	
				strcpy(lineaAux,"");
				strcpy(lineaAux,"[C]: ");
				strcat(lineaAux,linea);
				strcat(lineaAux,"\n");

			}else{
				strcpy(lineaAux,"");
				strcpy(lineaAux,"[C]: ");
				strcat(lineaAux,linea);
			}
		if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

    	fputs(lineaAux,g);
    	fclose(g);


		printf("[C]: %s", linea);



		// Concatena "\r\n" al final de la línea
		strcat(linea, "\r\n");
		// while (n_retry > 0){

		

		if (sendto(s, linea, strlen(linea), 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) != strlen(linea)){
            if (NULL == (g = (fopen(pEfimero, "a"))))
				fprintf(stderr, "No se ha podido abrir el fichero");

			fprintf(g, "%s: Connection aborted on error\nMessage was: %s\nLength returned by send() was: %d\n", argv[0], linea, len);
			fclose(g);
		
			exit(1);
		}
		alarm(TIMEOUT);
		//	memset(linea, 0, BUFFERSIZE);


		if((len = recvfrom(s, respuesta, BUFFERSIZE, 0, (struct sockaddr *)&servaddr_in, &addrlen)) == -1){
            if (errno == EINTR){
                // Si se produce el SIGALRM restamos un RETRY

				if (NULL == (g = (fopen(pEfimero, "a"))))
                    fprintf(stderr, "No se ha podido abrir el fichero");

                fprintf(g, "Attempt %d (Retries %d).\n", n_retry, RETRIES);
                fclose(g);

                
                n_retry--;
            }else{

				if (NULL == (g = (fopen(pEfimero, "a"))))
                    fprintf(stderr, "No se ha podido abrir el fichero");

				fprintf(g,"Unable to get response from.\n");
                fclose(g);
                
                exit(1);
            }
        }else{
            alarm(0); //Cancelamos la alarma

			strcpy(lineaAux, "[S]: ");
			strcat(lineaAux,respuesta);

			if (NULL == (g = (fopen(pEfimero, "a"))))
            	fprintf(stderr, "No se ha podido abrir el fichero");

    		fputs(lineaAux,g );
    		fclose(g);
            // Mostramos la respuesta
			printf("%s", lineaAux);
			//sleep(4);
                    //Mete mensaje progeso
                    /*if (NULL == (g = (fopen(puertoEfimero, "a"))))
                        fprintf(stderr, "No se ha podido abrir el fichero");
                    fputs(respuesta, g);
                    fputs("\n", g);
                    fclose(g);*/
                   // break;
        }
	
		/* linea es char[256]: usar BUFFERSIZE (1024) desbordaba la pila. */
		memset(linea, 0, sizeof(linea));
		if(n_retry == 0) {

			if (NULL == (g = (fopen(pEfimero, "a"))))
                    fprintf(stderr, "No se ha podido abrir el fichero");
			fprintf(g,"Unable to get response from");
			fprintf(g," %s after %d attempts.\n", argv[1], RETRIES);
			
			fclose(g);

		}
	}
	time(&timevar);
 	sprintf(fech_hora_final,"All done at %s", (char *)ctime(&timevar));

	if (NULL == (g = (fopen(pEfimero, "a"))))
            fprintf(stderr, "No se ha podido abrir el fichero");

    fputs(fech_hora_final,g );
    fclose(g);

	return;



}


