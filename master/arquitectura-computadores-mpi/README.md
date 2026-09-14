# Adivinación distribuida con MPI

Sistema de **búsqueda binaria distribuida** sobre MPI: un conjunto de procesos jugadores
compite por acceder a un conjunto limitado de procesos árbitro para adivinar un número, con
exclusión mutua sobre los árbitros y recogida de estadísticas de rendimiento.

Asignatura: *Arquitectura de Computadores* — Máster Universitario en Ingeniería Informática
(USAL).

---

## Cómo funciona

Los procesos se reparten en tres roles, asignados dinámicamente en tiempo de ejecución por
un proceso coordinador:

| Rol | Función |
|---|---|
| **PES** | Coordinador. Reparte roles e identificadores, distribuye los números a adivinar y recoge las estadísticas finales |
| **PG** | Jugador. Aplica búsqueda binaria sobre el rango, consultando a un árbitro en cada paso |
| **PA** | Árbitro. Custodia el número secreto y responde `MENOR`, `MAYOR` o `IGUAL`. Sólo atiende a un jugador a la vez |

El punto interesante es la **contención sobre los árbitros**: hay más jugadores que árbitros,
así que cada PG debe primero solicitar conexión (`TAG_SOLICITUD_CONEXION`) y recorrer los
árbitros hasta encontrar uno libre. Eso convierte el ejercicio en un problema de exclusión
mutua distribuida resuelto únicamente con paso de mensajes, sin memoria compartida ni
cerrojos.

## Instrumentación

El programa mide y agrega por proceso:

- Número de consultas rechazadas por árbitro ocupado
- Número de respuestas de comparación recibidas
- Tiempo de cálculo frente a tiempo total (para separar cómputo de espera en comunicación)
- Recuento de llamadas `MPI_Send`, `MPI_Recv` y `MPI_Probe`

Las estadísticas viajan al coordinador mediante un **tipo de dato MPI derivado**
(`MPI_Datatype` construido a medida para la estructura `Stats`), en lugar de serializarlas a
mano.

## Compilación y ejecución

```bash
mpicc -o adivinacion adivinacion_distribuida_mpi.c -lm
mpirun -np 8 ./adivinacion
```

Requiere una implementación de MPI (OpenMPI o MPICH).

---

Documentación del seminario en [`docs/SEMINARIO MPI.pdf`](docs/).
