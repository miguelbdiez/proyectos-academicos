#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

#define PES 0
#define PG 1
#define PA 2
#define PI 3

#define ARRAYSIZE 100

#define MENOR 1
#define MAYOR 2
#define IGUAL 3

#define TRUE 1
#define FALSE 0

#define LIBRE 1
#define OCUPADO 0

#define PESO_MIM 10000000 // >,=,<
#define PESO_MEDIO 100000

#define TAG_NOTIFICAR_NUMERO 5
#define TAG_PROCESAMIENTO_NUMERO 6
#define TAG_ID 7
#define TAG_INFOPAS 8
#define TAG_FINALIZACION 9
#define TAG_SOLICITUD_CONEXION 10
#define TAG_ESTADISTICAS 11
#define TAG_FINAL 12

#define TOTALNUM 25

// Añade un campo para el contador de llamadas MPI a la estructura Stats
typedef struct {
	int numero;  // Número a adivinar
	int consultas_ocupado;  // Número de consultas ocupado al PA
	int respuestas;  // Número de respuestas >=< al PA
	double tiempo_calculo;  // Tiempo de cálculo
	int numero_pg;
	double tiempo_total;
} Stats;

typedef struct {
	int tipo_proceso;
	double tiempo_total;
	double tiempo_calculo;
	double tiempo_max_qa_pg;
	int mpi_send;
	int mpi_recive;
	int mpi_probe;
	int id_proceso;
}FinalStats;

double Wtime(void) {
	struct timeval tv;
	if (gettimeofday(&tv, 0) < 0) {
    	perror("oops");
	}
	return (double)tv.tv_sec + (0.000001 * (double)tv.tv_usec);
}


MPI_Datatype getMPI_Estadisticas(Stats *stats);
MPI_Datatype getMPI_FinalStats(FinalStats *finalStats);
int mostrarTodasStats(int pg_real,int pa_count,int pg_count, int size, Stats recibidas[], FinalStats recibidasF[],double tiemposTotalesPG[], double tiempo_total);
void cargarNumeros(int vectorNum[]);
void fuerza_espera(unsigned long peso);


void process_pg(int rank, int size) {
	int buscando_conexion = FALSE;
	int id; 				
	int numAAdivinar;		 
	int numMedio = 0;		
	int solicitarPA = 1;	
	int caracter;		
	int conexion_realizada_PA= FALSE;  
	int auxInfoPAs[100];    
	int totalPAs;			
	int tag;
	int i, j;
	int k = 0;			
	MPI_Status status;
	Stats stats_parcial;
	MPI_Datatype structEstadisticas;
	structEstadisticas = getMPI_Estadisticas(&stats_parcial);
	FinalStats stats_final;
	MPI_Datatype structEstadisticasFinal;
	structEstadisticasFinal = getMPI_FinalStats(&stats_final);

	double tiempo_total_pg = 0.0;          // Variable para el tiempo total de PG
    double tiempo_total_calculo_pg = 0.0;  // Variable para el tiempo total de cálculo de PG
	double tiempo_inicio_procesamiento_pg=0.0;
	double tiempo_inicio_procesamiento=0.0;
	double tiempo_tot_ini = 0.0;
	double tiempo_totF_ini = 0.0;

    int contador=0;
    int contador_pa_ocupado = 0; 
    int contador_mpi_s=0;
	int contador_mpi_r=0;
	int contador_mpi_p=0;

    // Inicializa las estadísticas
	stats_parcial.numero=0;
	stats_parcial.consultas_ocupado = 0;
	stats_parcial.respuestas = 0;
	stats_parcial.tiempo_calculo = 0.0;
	stats_parcial.numero_pg =0;
	stats_parcial.tiempo_total=0.0;

	stats_final.tipo_proceso=PG;
	stats_final.id_proceso = rank;
	stats_final.tiempo_calculo = 0.0;
	stats_final.tiempo_total = 0.0;
	stats_final.mpi_probe = 0;
	stats_final.mpi_recive = 0;
	stats_final.mpi_send = 0;
    	

	// Recibir información del tipo y el ID del proceso desde el PES
	MPI_Recv(&id, 1, MPI_INT, PES, TAG_ID, MPI_COMM_WORLD, &status);
	tiempo_tot_ini = Wtime();
	tiempo_totF_ini= Wtime();
	tiempo_inicio_procesamiento = Wtime();
	contador_mpi_r++;
    
	// Verificar que el mensaje proviene del PES
	if (status.MPI_SOURCE != PES) {
		fprintf(stderr, "Error: Mensaje recibido no proviene del PES.\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	// Recibir infomacion de los PAs enviada desde PES
	MPI_Recv(&auxInfoPAs, ARRAYSIZE, MPI_INT, PES, TAG_INFOPAS, MPI_COMM_WORLD, &status);
	contador_mpi_r++;
	totalPAs = auxInfoPAs[0]; 
	int idPAs[totalPAs];

   	for(i =1 ; i<= totalPAs; i++){
        	idPAs[i-1] = auxInfoPAs[i];
    	}		
	
	while(1){
		contador=0;
		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG,MPI_COMM_WORLD, &status);
		contador_mpi_p++;
		tag = status.MPI_TAG;
		//Recibir numero para adivinar
		if(tag == TAG_NOTIFICAR_NUMERO){
		
			MPI_Recv(&numAAdivinar, 1, MPI_INT, PES, TAG_NOTIFICAR_NUMERO, MPI_COMM_WORLD, &status);
			contador_mpi_r++;
			tiempo_inicio_procesamiento_pg=0.0;
			tiempo_inicio_procesamiento_pg = Wtime();
			double tiempo_calculo_numero_pg = 0.0;
			tiempo_calculo_numero_pg = Wtime();
			stats_parcial.numero=numAAdivinar;
			int idPA;
			while(buscando_conexion == FALSE){
				if(k == totalPAs){ 
					k=0;		
				}
				MPI_Send(&solicitarPA, 1, MPI_INT, idPAs[k], TAG_SOLICITUD_CONEXION, MPI_COMM_WORLD);
				contador_mpi_s++;
				MPI_Recv(&buscando_conexion, 1, MPI_INT, idPAs[k], TAG_SOLICITUD_CONEXION, MPI_COMM_WORLD, &status);
				contador_mpi_r++;
				if (buscando_conexion == FALSE) {
					contador_pa_ocupado++;
				}
				if(buscando_conexion == TRUE){
					conexion_realizada_PA = TRUE;
					idPA = k;
					break;
				}
				k = k+1;
			}
			k+=1;
			while(conexion_realizada_PA == TRUE){

				MPI_Recv(&numMedio, 1, MPI_INT, idPAs[idPA], TAG_PROCESAMIENTO_NUMERO, MPI_COMM_WORLD, &status);
				contador_mpi_r++;
				contador++;
				
				if(numAAdivinar>numMedio){       
					caracter = MAYOR; //">"
					fuerza_espera(PESO_MIM);
					MPI_Send(&caracter, 1, MPI_INT, idPAs[idPA], TAG_PROCESAMIENTO_NUMERO, MPI_COMM_WORLD);
					contador_mpi_s++;
				}else if (numAAdivinar<numMedio){   		
					caracter = MENOR; //"<"
					fuerza_espera(PESO_MIM);
					MPI_Send(&caracter, 1, MPI_INT, idPAs[idPA], TAG_PROCESAMIENTO_NUMERO, MPI_COMM_WORLD);
					contador_mpi_r++;
				}else{                             
					caracter = IGUAL; //"="
					fuerza_espera(PESO_MIM);
					MPI_Send(&caracter, 1, MPI_INT, idPAs[idPA], TAG_PROCESAMIENTO_NUMERO, MPI_COMM_WORLD);
					contador_mpi_r++;
					// Medir tiempo de fin de procesamiento
					double tiempo_fin_procesamiento_pg = 0.0;
					tiempo_fin_procesamiento_pg = Wtime();
					stats_parcial.tiempo_calculo = tiempo_fin_procesamiento_pg - tiempo_inicio_procesamiento_pg;

					conexion_realizada_PA = FALSE;
					buscando_conexion = FALSE;

					double tiempo_tot_fin = 0.0;
					tiempo_tot_fin = Wtime();
					stats_parcial.tiempo_total = tiempo_tot_fin - tiempo_tot_ini;

					stats_parcial.respuestas=contador;
					stats_parcial.consultas_ocupado= contador_pa_ocupado;
					stats_parcial.numero_pg = rank;

					MPI_Send(&stats_parcial, 1, structEstadisticas, PES, TAG_ESTADISTICAS, MPI_COMM_WORLD);
					contador_mpi_s++;
					tiempo_inicio_procesamiento_pg= Wtime();
					tiempo_tot_ini=0.0;
					tiempo_tot_ini = Wtime();

				}
   			}
			// Medir el tiempo total de PG
			double tiempo_calculo_fin_pg = Wtime();
            tiempo_total_calculo_pg += (tiempo_calculo_fin_pg-tiempo_calculo_numero_pg);
		}else if(tag == TAG_FINALIZACION){

			MPI_Recv(NULL, 0, MPI_INT, PES, TAG_FINALIZACION , MPI_COMM_WORLD, &status);
			contador_mpi_r++;
			//envia estadisticas fianles
			contador_mpi_s++;
			stats_final.mpi_probe=contador_mpi_p;
			stats_final.mpi_recive=contador_mpi_r;
			stats_final.mpi_send=contador_mpi_s;
			double tiempo_fin_procesamiento = Wtime();
			stats_final.tiempo_total = tiempo_fin_procesamiento - tiempo_totF_ini;
            stats_final.tiempo_calculo = tiempo_total_calculo_pg;

			MPI_Send(&stats_final, 1, structEstadisticasFinal, PES, TAG_FINAL, MPI_COMM_WORLD);

			MPI_Type_free(&structEstadisticas);
			MPI_Type_free(&structEstadisticasFinal);

			return;//FINALIZA PG
		}	
	}

}

void process_pes(int rank, int size, int pg_count, int pa_count) {

	MPI_Status status;
	int infoPaToPg[100];
	int estadosPGs[pg_count][2];
	int j, i;
	int numRemaining = TOTALNUM; 
	int nextNumber = 0;
	int arrayNumeros[TOTALNUM];
	int pg_real;	
	Stats stats_parcial;
	int contadorFinalizados=0;
	int statsRecv = 0;
	Stats recibidasParciales[TOTALNUM];
	FinalStats recibidasTotales[size-1];
	int recibidos = 0;
	double tiempo_inicio, tiempo_final, tiempo_total;

	double tiemposTotalesPG[pg_count+1]; 
	int contador_adivinados_pg[pg_count+1];
	int adivinados_pg[pg_count+1];
	for (int i = 0; i <= pg_count; i++) {
    	tiemposTotalesPG[i] = 0.0;
		adivinados_pg[i]=0;
		contador_adivinados_pg[i] = 0;
	}

	MPI_Datatype structEstadisticas;
	structEstadisticas = getMPI_Estadisticas(&stats_parcial);
	FinalStats stats_final;
	MPI_Datatype structEstadisticasFinal;
	structEstadisticasFinal = getMPI_FinalStats(&stats_final);
	

	// Comprobar si hay suficientes procesos para distribuir según la elección
	if (pg_count + pa_count + 1 > size) {
    	fprintf(stderr, "No hay suficientes procesos para distribuir según tu elección.\n");
    	MPI_Abort(MPI_COMM_WORLD, 1);
    	MPI_Finalize();
    	exit(1);
	}
	tiempo_inicio = Wtime();
	printf("\n=================================================================\n");
    printf("\t\tAsignación final de procesos:\n");
    printf("=================================================================\n");
    printf("Numero de procesos: \t\t\t%d\n",size);
    printf("Numero de procesos gestores: \t\t%d\n",pg_count);
    printf("Numero de procesos adivinadores: \t%d\n",pa_count);
    printf("Numero de procesos TOTAL (+E/S): \t%d\n\n",(pa_count+pg_count+1));

	// Notificar a los procesos PG sobre su tipo
	for (int i = 1; i <= pg_count; i++) {
    	MPI_Send(&i, 1, MPI_INT, i, TAG_ID, MPI_COMM_WORLD);
		printf("\tProceso PG %d\n", i);
	}
	// Notificar a los procesos PA sobre su tipo
	for (int i = pg_count + 1; i <= pg_count + pa_count; i++) {
    	MPI_Send(&i, 1, MPI_INT, i, TAG_ID, MPI_COMM_WORLD);
		printf("\tProceso PA %d\n", i );
	}
	// Notificar a los procesos PI sobre su tipo
	for (int i = pg_count + pa_count + 1; i < size; i++) {
    	MPI_Send(&i, 1, MPI_INT, i, TAG_ID, MPI_COMM_WORLD);
		printf("\tProceso PI %d\n", i);
	}
    printf("=================================================================\n");

	
	for(i=0; i<pg_count;i++){
		estadosPGs[i][0]= LIBRE;
		estadosPGs[i][1]= 1 + i;
	}
	infoPaToPg[0] = pa_count;
	for(int i = 1 ;i <= pa_count; i++){
		infoPaToPg[i] = pg_count + i ;
	} 
	//Envia a los PGs la informacion de los PAs
	for (int i = 1; i <= pg_count; i++) {
    	MPI_Send(&infoPaToPg, ARRAYSIZE, MPI_INT, i, TAG_INFOPAS, MPI_COMM_WORLD);
	}
	cargarNumeros(arrayNumeros);

	if(pg_count > TOTALNUM)
		pg_real = TOTALNUM;
	else
		pg_real = pg_count;

	// Envío inicial de números a los PGs
	for (int i = 0; i < pg_count; i++) {
		if (nextNumber < TOTALNUM) {
			MPI_Send(&arrayNumeros[nextNumber], 1, MPI_INT, estadosPGs[i][1], TAG_NOTIFICAR_NUMERO, MPI_COMM_WORLD);
		    nextNumber++;
		}
	}

	while (numRemaining > 0 || statsRecv < size-1)  {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int source = status.MPI_SOURCE;
        int tag = status.MPI_TAG;

		// Recepción de estadísticas parciales
		if (tag == TAG_ESTADISTICAS) {

		    MPI_Recv(&stats_parcial, 1, structEstadisticas, source, TAG_ESTADISTICAS, MPI_COMM_WORLD, &status);
			recibidasParciales[recibidos] = stats_parcial;
			recibidos++;
		    numRemaining--;
		    // Si quedan números en la lista, envía el siguiente
		    if (nextNumber < TOTALNUM) {
		        MPI_Send(&arrayNumeros[nextNumber], 1, MPI_INT, source, TAG_NOTIFICAR_NUMERO, MPI_COMM_WORLD);
		        nextNumber++;
		    } else {
		        // Si no quedan números y ya se encontraron todos, envía un mensaje de finalización al PG, PI, PA
				contadorFinalizados++;
				if(contadorFinalizados == pg_real){		

					for (int i = 1; i < pg_count; i++) {
						MPI_Send(NULL, 0, MPI_INT, i, TAG_FINALIZACION, MPI_COMM_WORLD);
					}		
					for (int i = pg_count + pa_count + 1; i < size; i++) {
						MPI_Send(NULL, 0, MPI_INT, i, TAG_FINALIZACION, MPI_COMM_WORLD);
					}
					for (int i = pg_count + 1; i <= pg_count + pa_count; i++) {
						MPI_Send(NULL, 0, MPI_INT, i, TAG_FINALIZACION, MPI_COMM_WORLD);
					}
					for (int i = pg_real; i <= pg_count; i++) {
						MPI_Send(NULL, 0, MPI_INT, i, TAG_FINALIZACION, MPI_COMM_WORLD);
					}
				}
		    }
			
		} else if (tag == TAG_FINAL){
			statsRecv++;
			MPI_Recv(&stats_final, 1, structEstadisticasFinal, source, TAG_FINAL, MPI_COMM_WORLD, &status);
			recibidasTotales[source] = stats_final;			
		}	
	}
	tiempo_final = Wtime(); 
    tiempo_total = tiempo_final - tiempo_inicio;
	mostrarTodasStats(pg_real,pa_count ,pg_count, size, recibidasParciales, recibidasTotales,tiemposTotalesPG,tiempo_total);
	MPI_Type_free(&structEstadisticasFinal);
    MPI_Type_free(&structEstadisticas);
	return;

}

void process_pa(int rank, int size){

	int id;                                         	
	int caracter;                             		
	int probe;
	int numMax=999999;          
	int numMin = 0;
	int numMedio = (numMin +((numMax-numMin)/2));   	 
	int conexion = FALSE;              		
	int estatusPA = LIBRE;					
	int tag; 						
	int origenSolicitud;
	int origenPG;						
	MPI_Status status;	
	int w=0;
	FinalStats stats_final;
	MPI_Datatype structEstadisticasFinal;
	structEstadisticasFinal = getMPI_FinalStats(&stats_final);

    int contador_mpi_s=0;
	int contador_mpi_r=0;
	int contador_mpi_p=0;

	stats_final.tipo_proceso=PA;
	stats_final.id_proceso = rank;
	stats_final.tiempo_calculo = 0.0;
	stats_final.tiempo_total = 0.0;
	stats_final.mpi_probe = 0;
	stats_final.mpi_recive = 0;
	stats_final.mpi_send = 0;

	double tiempo_calculos_inicio=0.0;
	double tiempo_calculos_fin;
    double tiempo_calculos ;
	double tiempo_qa_ini= 0.0;
	double tiempo_qa_fin= 0.0;
	double tiempo_qa= 0.0;

	// Recibir información del tipo y el ID del proceso desde el PES
	MPI_Recv(&id, 1, MPI_INT, PES, TAG_ID, MPI_COMM_WORLD, &status);
	double tiempo_inicio_procesamiento = Wtime();
	contador_mpi_r++;

	if (status.MPI_SOURCE != PES) {
		fprintf(stderr, "Error: Mensaje recibido no proviene del PES.\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

    //Recibe la solicitud de conexion de un PG, En caso de estar libre la acepta
	while(1){
		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG,MPI_COMM_WORLD, &status);
		contador_mpi_p++;
		tag = status.MPI_TAG;
		origenSolicitud = status.MPI_SOURCE;
		if(tag == TAG_SOLICITUD_CONEXION){
			MPI_Recv(&conexion, 1, MPI_INT, origenSolicitud, TAG_SOLICITUD_CONEXION, MPI_COMM_WORLD, &status);
			contador_mpi_r++;

			if(estatusPA == LIBRE){
				MPI_Send(&estatusPA, 1, MPI_INT, origenSolicitud, TAG_SOLICITUD_CONEXION, MPI_COMM_WORLD);
				contador_mpi_s++;
				estatusPA = OCUPADO;
				origenPG = origenSolicitud;	
				numMax=999999;
				numMin=0;
				numMedio = (numMin +(numMax-numMin)/2);
				//Empieza el procesamiento del dato
				MPI_Send(&numMedio, 1, MPI_INT, origenPG, TAG_PROCESAMIENTO_NUMERO, MPI_COMM_WORLD);
				tiempo_calculos_inicio=Wtime();

				contador_mpi_s++;
			}else{
				MPI_Send(&estatusPA, 1, MPI_INT, origenSolicitud, TAG_SOLICITUD_CONEXION, MPI_COMM_WORLD);
				contador_mpi_s++;
			}
		}else if(tag == TAG_PROCESAMIENTO_NUMERO){
			MPI_Recv(&caracter, 1, MPI_INT, origenPG, TAG_PROCESAMIENTO_NUMERO, MPI_COMM_WORLD, &status);
			tiempo_qa_fin =Wtime();

			contador_mpi_r++;
			if(caracter == MAYOR){
				numMin = numMedio;
				numMedio = (numMin +(numMax-numMin)/2);
				fuerza_espera(PESO_MEDIO);
				tiempo_qa_ini= Wtime();
				MPI_Send(&numMedio, 1, MPI_INT, origenPG, TAG_PROCESAMIENTO_NUMERO, MPI_COMM_WORLD);
				contador_mpi_s++;
			}else if(caracter == MENOR){
				numMax= numMedio;
				numMedio = (numMin +(numMax-numMin)/2);
				fuerza_espera(PESO_MEDIO);
				tiempo_qa_ini= Wtime();
				MPI_Send(&numMedio, 1, MPI_INT, origenPG, TAG_PROCESAMIENTO_NUMERO, MPI_COMM_WORLD);
				contador_mpi_s++;
			}else if(caracter == IGUAL){
				estatusPA = LIBRE;
				fuerza_espera(PESO_MEDIO);
				tiempo_calculos_fin=Wtime();
				tiempo_calculos+= (tiempo_calculos_fin-tiempo_calculos_inicio);
			}
			if(tiempo_qa < (tiempo_qa_fin-tiempo_qa_ini))
				tiempo_qa = tiempo_qa_fin-tiempo_qa_ini;

		}else if(tag == TAG_FINALIZACION){
			//ACABA PA y envia estadisticas a PES
			MPI_Recv(NULL, 0, MPI_INT, PES, TAG_FINALIZACION , MPI_COMM_WORLD, &status);
			contador_mpi_r++;

			contador_mpi_s++;
			stats_final.mpi_probe=contador_mpi_p;
			stats_final.mpi_recive=contador_mpi_r;
			stats_final.mpi_send=contador_mpi_s;
			double tiempo_fin_procesamiento = Wtime();
			stats_final.tiempo_total = tiempo_fin_procesamiento - tiempo_inicio_procesamiento;
			stats_final.tiempo_calculo=tiempo_calculos;
			stats_final.tiempo_max_qa_pg = tiempo_qa;

			MPI_Send(&stats_final, 1, structEstadisticasFinal, PES, TAG_FINAL, MPI_COMM_WORLD);

			return;//FINALIZA PA
		}
	}
}
void process_pi(int rank, int size) {

	int id;
	MPI_Status status;
	FinalStats stats_final;
	MPI_Datatype structEstadisticasFinal;
	structEstadisticasFinal = getMPI_FinalStats(&stats_final);

    int contador_mpi_s=0;
	int contador_mpi_r=0;
	int contador_mpi_p=0;

	stats_final.tipo_proceso=PI;
	stats_final.id_proceso = rank;
	stats_final.tiempo_calculo = 0.0;
	stats_final.tiempo_total = 0.0;
	stats_final.mpi_probe = 0;
	stats_final.mpi_recive = 0;
	stats_final.mpi_send = 0;

	// Recibir información del tipo y el ID del proceso desde el PES
	MPI_Recv(&id, 1, MPI_INT, PES, TAG_ID, MPI_COMM_WORLD, &status);
	double tiempo_inicio_procesamiento = Wtime();
	contador_mpi_r++;

	if (status.MPI_SOURCE != PES) {
    	fprintf(stderr, "Error: Mensaje recibido no proviene del PES.\n");
    	MPI_Abort(MPI_COMM_WORLD, 1);
	}
	
	MPI_Recv(NULL, 0, MPI_INT, PES, TAG_FINALIZACION, MPI_COMM_WORLD, &status);
	contador_mpi_r++;
	contador_mpi_s++;
	stats_final.mpi_probe=contador_mpi_p;
	stats_final.mpi_recive=contador_mpi_r;
	stats_final.mpi_send=contador_mpi_s;
	double tiempo_fin_procesamiento = Wtime();
	stats_final.tiempo_total = tiempo_fin_procesamiento - tiempo_inicio_procesamiento;

	MPI_Send(&stats_final, 1, structEstadisticasFinal, PES, TAG_FINAL, MPI_COMM_WORLD);
	return;//FINALIZA PI
}

int main(int argc, char *argv[]) {
	int rank, size;
	int pg_count, pa_count;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (argc != 3) {
    	if (rank == PES) {
        	fprintf(stderr, "Uso: %s <procesos gestores> <procesos adivinadores>\n", argv[0]);
    	}
    	MPI_Abort(MPI_COMM_WORLD, 1);
    	MPI_Finalize();
    	exit(1);
	}
	pg_count = atoi(argv[1]);
	pa_count = atoi(argv[2]);

	if (rank == PES) {
    	process_pes(rank, size, pg_count, pa_count);
	} else if (rank <= pg_count) {
    	process_pg(rank, size);
	} else if (rank <= pg_count + pa_count) {
    	process_pa(rank, size);
	} else {
    	process_pi(rank, size);
	}
	MPI_Finalize();

	return 0;
}
MPI_Datatype getMPI_Estadisticas(Stats *stats) {
   MPI_Datatype structEstadisticas;
   int longitudes[6] = {1, 1, 1, 1,1,1};
   MPI_Aint desplazamiento[6];
   MPI_Aint direcciones[7];
   MPI_Datatype tipos[6] = {MPI_INT, MPI_INT, MPI_INT, MPI_DOUBLE, MPI_INT, MPI_DOUBLE};

   /* Calculamos los desplazamientos */
   MPI_Get_address(stats, &direcciones[0]);
   MPI_Get_address(&(stats->numero), &direcciones[1]);
   MPI_Get_address(&(stats->consultas_ocupado), &direcciones[2]);
   MPI_Get_address(&(stats->respuestas), &direcciones[3]);
   MPI_Get_address(&(stats->tiempo_calculo), &direcciones[4]);
   MPI_Get_address(&(stats->numero_pg), &direcciones[5]);
   MPI_Get_address(&(stats->tiempo_total), &direcciones[6]);

   for (int i = 0; i < 6; i++) {
       desplazamiento[i] = direcciones[i + 1] - direcciones[0];  // Ajuste: Multiplicamos por el tamaño del tipo base
   }

   /* Creamos el tipo derivado */
   MPI_Type_create_struct(6, longitudes, desplazamiento, tipos, &structEstadisticas);
   MPI_Type_commit(&structEstadisticas);

   return structEstadisticas;
}

MPI_Datatype getMPI_FinalStats(FinalStats *finalStats) {
    MPI_Datatype structFinalStats;
    int longitudes[8] = {1,1, 1, 1, 1, 1, 1, 1};
    MPI_Aint desplazamiento[8];
    MPI_Aint direcciones[9];
    MPI_Datatype tipos[8] = {MPI_INT, MPI_DOUBLE, MPI_DOUBLE,MPI_DOUBLE, MPI_INT, MPI_INT, MPI_INT,MPI_INT};

    /* Calculamos los desplazamientos */
    MPI_Get_address(finalStats, &direcciones[0]);
    MPI_Get_address(&(finalStats->tipo_proceso), &direcciones[1]);
    MPI_Get_address(&(finalStats->tiempo_total), &direcciones[2]);
    MPI_Get_address(&(finalStats->tiempo_calculo), &direcciones[3]);
	MPI_Get_address(&(finalStats->tiempo_max_qa_pg), &direcciones[4]);
    MPI_Get_address(&(finalStats->mpi_send), &direcciones[5]);
    MPI_Get_address(&(finalStats->mpi_recive), &direcciones[6]);
    MPI_Get_address(&(finalStats->mpi_probe), &direcciones[7]);
	MPI_Get_address(&(finalStats->id_proceso), &direcciones[8]);

    for (int i = 0; i < 8; i++) {
        desplazamiento[i] = direcciones[i + 1] - direcciones[0];  // Ajuste: Multiplicamos por el tamaño del tipo base
    }

    /* Creamos el tipo derivado */
    MPI_Type_create_struct(8, longitudes, desplazamiento, tipos, &structFinalStats);
    MPI_Type_commit(&structFinalStats);

    return structFinalStats;
}

int mostrarTodasStats(int pg_real, int pa_count,int pg_count, int size, Stats recibidas[], FinalStats recibidasF[],double tiemposTotalesPG[], double tiempo_total){
	int l,k;
	printf("=============================================================\n");
	printf("|-----------------------------------------------------------|\n");
	printf("|------------- ESTADISTICAS PARCIALES DE LOS PG ------------|\n");
	printf("|-----------------------------------------------------------|\n");
	printf("=============================================================\n\n\n");

	if(pg_count < TOTALNUM){	
		for(k=1; k<=pg_count;k++){
			printf("\t=============================================================\n");
			printf("\t|------------- ESTADISTICAS PARCIALES DEL PG %d ------------|\n",k);
			printf("\t=============================================================\n");
			for(l=0;l<TOTALNUM;l++){
				Stats temp = recibidas[l];
				if(temp.numero_pg==k){
				printf("\t\tNúmero a adivinar: %d\n", recibidas[l].numero);
				printf("\t\tNúmero de consultas ocupado al PA: %d\n", temp.consultas_ocupado);
				printf("\t\tNúmero de respuestas >=< al PA: %d\n", temp.respuestas);
				printf("\t\tTiempo de cálculo: %.10f segundos\n", temp.tiempo_calculo);
				printf("\t\tTiempo total: %.10f segundos\n",temp.tiempo_total);
				puts("\t-------------------------------------------------------------");
				}
			}
		}
	}else{
		for(k=1; k<=pg_real;k++){
			printf("\t=============================================================\n");
			printf("\t|------------- ESTADISTICAS PARCIALES DEL PG %d ------------|\n",k);
			printf("\t=============================================================\n");
			for(l=0;l<TOTALNUM;l++){
				Stats temp = recibidas[l];
				if(temp.numero_pg==k){
				printf("\t\tNúmero a adivinar: %d\n", recibidas[l].numero);
				printf("\t\tNúmero de consultas ocupado al PA: %d\n", temp.consultas_ocupado);
				printf("\t\tNúmero de respuestas >=< al PA: %d\n", temp.respuestas);
				printf("\t\tTiempo de cálculo: %.10f segundos\n", temp.tiempo_calculo);
				printf("\t\tTiempo total: %.10f segundos\n",temp.tiempo_total);
				puts("\t-------------------------------------------------------------");
				}
			}
		}


		for(k=TOTALNUM+1; k<=pg_count;k++){
			printf("\t=============================================================\n");
			printf("\t|------------- ESTADISTICAS PARCIALES DEL PG %d ------------|\n",k);
			printf("\t=============================================================\n");
			printf("\t    NO TIENE ESTADISTICAS YA QUE NO RESOLVIO NINGUN NUMERO\n");
			puts("\t-------------------------------------------------------------");
		}
	}


	printf("\n\n=============================================================\n");
	printf("|-----------------------------------------------------------|\n");
	printf("|---------------------- ESTADISTICAS------------------------|\n");
	printf("|-------------------------FINALES---------------------------|\n");
	printf("|-----------------------------------------------------------|\n");
	printf("=============================================================\n\n\n");

    int total_send = 0;
    int total_recv = 0;
    int total_probe = 0;
    double suma_tiempo_total = 0.0;
    double suma_tiempo_calculo = 0.0;
	
	for(l=1;l<size;l++){
		FinalStats temp = recibidasF[l];
		printf("\t=============================================================\n");
		printf("\t|------------- ESTADISTICAS FINALES DE %d ------------|\n",temp.id_proceso);

		if(temp.tipo_proceso == PG)
		printf("\t|-------------------TIPO DE PROCESO PG-------------|\n");

		if(temp.tipo_proceso == PA)
		printf("\t|-------------------TIPO DE PROCESO PA-------------|\n");

		if(temp.tipo_proceso == PI)
		printf("\t|-------------------TIPO DE PROCESO PI-------------|\n");

		printf("\t=============================================================\n");
     	printf("\t\tNúmero de SEND: %d\n", temp.mpi_send);
    	printf("\t\tNúmero de RECV: %d\n", temp.mpi_recive);
        printf("\t\tNúmero de PROBE: %d\n", temp.mpi_probe);
        printf("\t\tTiempo total: %.10f\n", temp.tiempo_total);
		printf("\t\tTiempo calculo: %.10f\n", temp.tiempo_calculo);

        double porcentaje_tiempo_ocupado = (temp.tiempo_calculo / temp.tiempo_total) * 100;
        printf("\t\t%% Porcentaje de tiempo ocupado en el cálculo: %.2f%%\n", porcentaje_tiempo_ocupado);

		if(temp.tipo_proceso == PA)
		printf("\t\tTiempo maximo entre preguntas y respuesta PG: %.2f\n",temp.tiempo_max_qa_pg);	

        total_send += temp.mpi_send;
        total_recv += temp.mpi_recive;
        total_probe += temp.mpi_probe;
        suma_tiempo_total += temp.tiempo_total;
        suma_tiempo_calculo += temp.tiempo_calculo;
	}
    double porcentaje_tiempo_ocupado_total = (suma_tiempo_calculo / suma_tiempo_total) * 100;
    printf("\n\n\t==================TOTALES DE SEND, RECV Y PROBE=============================\n");
    printf("\t\tNúmero total de SEND: %d\n", total_send);
    printf("\t\tNúmero total de RECV: %d\n", total_recv);
    printf("\t\tsNúmero total de PROBE: %d\n", total_probe);

	printf("\t==================SUMA DE TIEMPOS DE CALCULO Y TOTAL===========================\n");
    printf("\t\tSuma de tiempos de cálculo: %.10f segundos\n", suma_tiempo_calculo);
    printf("\t\tSuma de tiempos totales: %.10f segundos\n", suma_tiempo_total);
    printf("\t\tPorcentaje de tiempo ocupado en el cálculo (para la suma total): %.2f%%\n\n", porcentaje_tiempo_ocupado_total);

	printf("\t\tTiempo total de ejecución: %.6f segundos\n", tiempo_total);
	return 0;
}

void cargarNumeros(int vectorNum[]){
	int i;
	srand(time(NULL));
	for(i=0; i<TOTALNUM;i++){
		vectorNum[i] = rand()%100000;
	}
}

void fuerza_espera(unsigned long peso){
	for (unsigned long i=1; i<1*peso; i++)
	  	sqrt(i);
}

