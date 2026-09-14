//SEGUNDA PRÁCTICA EVALUABLE · SSOOII · MAS LOMO · Miguel Barranquero Diez · Alejandro González Rengel

#include <iostream>
#include <time.h>
#include "windows.h"
#include "lomo2.h"

#define LOGIN1 "i0970320"
#define LOGIN2 "i0917650"

#define NUM_TREN_MAX 100
#define NUM_TREN_MIN   0

#define ROWS		  17
#define COLUMNS		  75


#pragma region DLL

HINSTANCE libreria = LoadLibrary(TEXT("../libs/lomo2.dll"));

int(*GenerarMapa)(char const*, char const*);
int(*Iniciar)(int, int, char const*, char const*);

int(*PetAvance)(int, int*, int*);
int(*Avance)(int, int*, int*);

char* (*GetColor)(int);
VOID(*Espera)(int, int);

void(*Error)(const char*);
int(*TrenNuevo)(VOID);
int(*Fin)(VOID);

#pragma endregion


CRITICAL_SECTION scInicio;
CRITICAL_SECTION scMapa;
HANDLE empezar;
int salir_bucle = 1;
int bloqueado = 0;

#pragma region ENCABEZADOS FUNCIONES

int cargarDLL();

DWORD WINAPI funcionTrenes(LPVOID a);
BOOL WINAPI CtrlHandler(DWORD senial);

#pragma endregion


//LOMO_inicio

int main(int argc, const char* argv[]) 
{
	if (libreria == NULL)
	{
		printf("No encuentro la DLL lomo2\r\n");
		fflush(stdout);
		return -1;
	}

	if (cargarDLL() == -1)
	{
		Error("ERROR al cargar lomo2.dll");
		FreeLibrary(libreria);
		return 0;
	}

	if (argc == 1 || argc > 4) 
	{
		printf("Error, numero de argumentos incorrectos\n\r");
		fflush(stdout);
		FreeLibrary(libreria);
		return -1;
	}

	if (strcmp(argv[1], "--mapa") == 0)
	{
		if (GenerarMapa(LOGIN1, LOGIN2) == -1) 
		{
			Error("Error al generar MAPA LOMO\r\n");
		}
		FreeLibrary(libreria);

		return 0;
	}

	if (argc == 4)
	{
		int mapa[1275];

		HANDLE trenes[NUM_TREN_MAX];
		DWORD trenesID[NUM_TREN_MAX];

		int retardo = atoi(argv[1]);
		int tamanoTren = atoi(argv[2]);
		int nTrenes = atoi(argv[3]);

		if (retardo < 0)
		{
			puts("ERROR: Valor de retardo incorrecto\r\n");
			fflush(stdout);
			FreeLibrary(libreria);

			exit(1);
		}
		else if ((nTrenes < NUM_TREN_MIN) || (nTrenes > NUM_TREN_MAX))
		{
			puts("ERROR: Numero de trenes incorrecto\r\n");
			fflush(stdout);
			FreeLibrary(libreria);

			exit(1);
		}
		else if ((tamanoTren < 3) || (tamanoTren > 19))
		{
			puts("ERROR: tamano de los trenes incorrecto\r\n");
			fflush(stdout);
			FreeLibrary(libreria);

			exit(1);
		}

		InitializeCriticalSection(&scInicio);
		InitializeCriticalSection(&scMapa);

		empezar = CreateEvent(NULL, TRUE, FALSE, "trenesEvent");
		if (empezar == NULL) 
		{
			Error("Error Evento");
			DeleteCriticalSection(&scMapa);
			DeleteCriticalSection(&scInicio);

			FreeLibrary(libreria);
			exit(-1);
		}

		HANDLE memmoriaCompartida;
		LPINT  puntMem;
		if (INVALID_HANDLE_VALUE == (memmoriaCompartida = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(mapa), "memP"))) 
		{
			Error("Error al crear segmento de memoria compartida");

			DeleteCriticalSection(&scMapa);
			DeleteCriticalSection(&scInicio);
			CloseHandle(empezar);

			FreeLibrary(libreria);
			exit(2);
		}

		if (NULL == (puntMem = (LPINT)MapViewOfFile(memmoriaCompartida, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(mapa)))) 
		{
			Error("Error al mapear memoria");

			CloseHandle(memmoriaCompartida);
			DeleteCriticalSection(&scMapa);
			DeleteCriticalSection(&scInicio);
			CloseHandle(empezar);
			FreeLibrary(libreria);

			exit(-1);
		}

		for (int i = 0; i < ROWS; i++) 
		{
			for (int j = 0; j < COLUMNS; j++)
			{
				mapa[i * COLUMNS + j] = 0; //inicializar cada posicion a cero;
			}
		}
		CopyMemory(puntMem, &mapa, sizeof(mapa));


		if (-1 == Iniciar(retardo, tamanoTren, LOGIN1, LOGIN2)) 
		{
			Error("ERROR AL INICIAR LOMO\n");
			CloseHandle(memmoriaCompartida);
			UnmapViewOfFile(puntMem);
			DeleteCriticalSection(&scMapa);
			DeleteCriticalSection(&scInicio);
			CloseHandle(empezar);

			FreeLibrary(libreria);
			exit(1);
		}


		for (int i = 0; i < nTrenes; i++) 
		{
			trenes[i] = CreateThread(NULL, 0, funcionTrenes, LPVOID(i), 0, &trenesID[i]);

			if (trenes[i] == NULL) 
			{
				Error("ERROR, al crear tren %d\r\n");

				CloseHandle(memmoriaCompartida);
				UnmapViewOfFile(puntMem);
				DeleteCriticalSection(&scMapa);
				DeleteCriticalSection(&scInicio);
				CloseHandle(empezar);
				FreeLibrary(libreria);
			}
		}

		SetEvent(empezar);
		SetConsoleCtrlHandler(CtrlHandler, TRUE);

		clock_t startTime = clock();
		clock_t endTime = startTime + 60 * CLOCKS_PER_SEC;

		while (clock() < endTime) 
		{
			if (salir_bucle == 0) 
			{
				break;
			}
		}

		for (int i = 0; i < nTrenes; i++) 
		{
			PostThreadMessage(trenesID[i], 110, WPARAM(0), LPARAM(0));
		}

		WaitForMultipleObjects(nTrenes, trenes, true, INFINITE);

		if (Fin() == 0) 
		{
			Error("EJECUCION CORRECTA");
			system("cls");
		}

		for (int j = 0; j < nTrenes; j++) 
		{
			CloseHandle(trenes[j]);
		}

		CloseHandle(memmoriaCompartida);
		CloseHandle(empezar);
		UnmapViewOfFile(puntMem);

		DeleteCriticalSection(&scMapa);
		DeleteCriticalSection(&scInicio);

		FreeLibrary(libreria);
	}
	return 0;
}

DWORD WINAPI funcionTrenes(LPVOID a)
{
	MSG msj;
	HANDLE memoria = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "memP");

	if (memoria == INVALID_HANDLE_VALUE) 
	{
		Error("ERROR al abrir memoria compartida");
		fflush(stdout);
		return -1;
	}

	LPINT  m = (LPINT)MapViewOfFile(memoria, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * 1275);

	if (m == NULL) 
	{
		CloseHandle(memoria);
		ExitThread(-1);
	}

	DWORD tiempoInicio;
	DWORD tiempoFinal;
	DWORD tiempoEspera;
	HANDLE con = GetStdHandle(STD_OUTPUT_HANDLE);

	int idTren;

	int xcab = 0;
	int xcola = 0;
	int ycab = 0;
	int ycola = 0;

	int libre = 0;
	int creado = 0;

	int ycabezaAntes = 0;

	WaitForSingleObject(empezar, INFINITE);
	EnterCriticalSection(&scInicio);

	idTren = TrenNuevo();

	if (idTren == -1) 
	{
		Error("Error al asignar identificador al tren");
		CloseHandle(memoria);
		UnmapViewOfFile(m);
		CloseHandle(con);
		ExitThread(-1);
	}

	const char* color = GetColor(idTren);

	while (true) 
	{
		tiempoInicio = GetTickCount();

		libre = 0;
		ycabezaAntes = ycab;

		PetAvance(idTren, &xcab, &ycab);

		do 
		{
			if (PeekMessage(&msj, NULL, 110, 110, PM_NOREMOVE)) 
			{
				CloseHandle(memoria);
				UnmapViewOfFile(m);
				ExitThread(1);
			}

			EnterCriticalSection(&scMapa);

			tiempoFinal = GetTickCount();				// Tiempo de finalización de la ejecución
			tiempoEspera = tiempoFinal - tiempoInicio;  // Tiempo de espera para acceder a la sección crítica

			if (tiempoEspera > 900 && bloqueado == 0) 
			{
				bloqueado = 1;

				printf("\n\n  TREN BLOQUEADO, ID: %d  ---- ", idTren);

				if (strcmp(color, "verde") == 0) 
				{
					SetConsoleTextAttribute(con, FOREGROUND_GREEN);
				}
				else if (strcmp(color, "blanco") == 0) 
				{
					SetConsoleTextAttribute(con, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				}
				else if (strcmp(color, "morado") == 0) 
				{
					SetConsoleTextAttribute(con, FOREGROUND_RED | FOREGROUND_BLUE);
				}
				else if (strcmp(color, "cielo") == 0) 
				{
					SetConsoleTextAttribute(con, FOREGROUND_BLUE);
				}
				else if (strcmp(color, "amarillo") == 0) 
				{
					SetConsoleTextAttribute(con, FOREGROUND_RED | FOREGROUND_GREEN);
				}
				else if (strcmp(color, "rojo") == 0) 
				{
					SetConsoleTextAttribute(con, FOREGROUND_RED);
				}
				printf("COLOR: %s", color);
				SetConsoleTextAttribute(con, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				salir_bucle = 0;
			}

			m = m + (ycab * COLUMNS + xcab);

			if (*m == 0)
			{
			   *m = 1;
				m = m - (ycab * COLUMNS + xcab);
				LeaveCriticalSection(&scMapa);
				libre = 1;
			}
			else 
			{
				m = m - (ycab * COLUMNS + xcab);
				LeaveCriticalSection(&scMapa);
			}
		} while (libre == 0);

		Avance(idTren, &xcola, &ycola);

		if (xcola != -1 && creado == 0) 
		{
			LeaveCriticalSection(&scInicio);
			creado = 1;
		}
		else if (xcola != -1) 
		{
			EnterCriticalSection(&scMapa);
			m = m + (ycola * COLUMNS + xcola);
			*m = 0;
			m = m - (ycola * COLUMNS + xcola);
			LeaveCriticalSection(&scMapa);
		}

		Espera(ycabezaAntes, ycab);
	}
}


BOOL WINAPI CtrlHandler(DWORD senial) 
{
	if (senial == CTRL_C_EVENT) 
	{
		salir_bucle = 0;
		return true;
	}
	return false;
}

int cargarDLL() 
{
	GenerarMapa = (int(*)(char const*, char const*))GetProcAddress(libreria, "LOMO_generar_mapa");
	if (GenerarMapa == NULL) 
	{
		printf("Error al cargar GenerarMapa() de lomo2.dll\r\n");
		fflush(stdout);
		return -1;
	}

	Iniciar = (int(*)(int, int, char const*, char const*))GetProcAddress(libreria, "LOMO_inicio");
	if (Iniciar == NULL)
	{
		printf("Error Bindeo Iniciar\r\n");
		fflush(stdout);
		return -1;
	}

	TrenNuevo = (int(*)(VOID))GetProcAddress(libreria, "LOMO_trenNuevo");
	if (Iniciar == NULL)
	{
		printf("Error Bindeo Iniciar\r\n");
		fflush(stdout);
		return -1;
	}

	PetAvance = (int(*)(int, int*, int*))GetProcAddress(libreria, "LOMO_peticiOnAvance");
	if (PetAvance == NULL) 
	{
		printf("Error al cargar LOMO_peticiOnAvance() de lomo2.dll\r\n");
		fflush(stdout);
		return -1;
	}

	Avance = (int(*)(int, int*, int*))GetProcAddress(libreria, "LOMO_avance");
	if (Avance == NULL) 
	{
		printf("Error al cargar LOMO_avance() de lomo2.dll\r\n");
		fflush(stdout);
		return -1;
	}

	GetColor = (char* (*)(int))GetProcAddress(libreria, "LOMO_getColor");
	if (GetColor == NULL) 
	{
		printf("Error al cargar LOMO_getColor() de lomo2.dll\r\n");
		fflush(stdout);
		return -2;
	}

	Espera = (VOID(*)(int, int))GetProcAddress(libreria, "LOMO_espera");
	if (Espera == NULL) 
	{
		printf("Error al cargar LOMO_espera() de lomo2.dll\r\n");
		fflush(stdout);
		return -2;
	}

	Error = (VOID(*)(const char*))GetProcAddress(libreria, "pon_error");
	if (Error == NULL) 
	{
		printf("Error al cargar LOMO_espera() de lomo2.dll\r\n");
		fflush(stdout);
		return -2;
	}

	Fin = (int(*)(VOID))GetProcAddress(libreria, "LOMO_fin");
	if (Fin == NULL) 
	{
		printf("Error al cargar LOMO_fin() de lomo2.dll\r\n");
		fflush(stdout);
		return -2;
	}
return 0;
}
