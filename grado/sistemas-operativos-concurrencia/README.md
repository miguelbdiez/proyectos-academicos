# Simulación concurrente de tráfico ferroviario

Cada tren de la simulación es un **hilo independiente** que compite con los demás por los
tramos de vía. El ejercicio consiste en coordinarlos para que circulen sin colisiones ni
interbloqueos, usando primitivas de sincronización de Win32.

Asignatura: *Sistemas Operativos II* — Grado en Ingeniería Informática (USAL).

---

## El problema de concurrencia

Un mapa de vías compartido, hasta 100 trenes circulando simultáneamente y una regla simple:
dos trenes no pueden ocupar el mismo tramo. Como cada tren es un hilo y el mapa es estado
compartido y mutable, todo avance es una sección crítica.

La solución usa dos secciones críticas con propósitos distintos:

| Primitiva | Protege |
|---|---|
| `scInicio` | El arranque escalonado de los hilos, para que ningún tren empiece a moverse antes de que la simulación esté lista |
| `scMapa` | Toda consulta y modificación del estado del mapa: petición de avance, avance efectivo y liberación del tramo anterior |

El acceso al mapa sigue el patrón **petición → confirmación** (`PetAvance` seguido de
`Avance`), de modo que la comprobación de disponibilidad y la ocupación del tramo ocurren
dentro de la misma sección crítica y no puede colarse otro hilo entre ambas.

La sincronización de arranque se resuelve con un evento (`WaitForSingleObject(empezar, INFINITE)`),
que libera a todos los hilos a la vez una vez creados.

## Estructura

```
lomo2.sln                        Solución de Visual Studio
prueba3/
  lomo2.cpp                      Simulación: creación de hilos, sincronización y bucle de cada tren
  lomo2.h                        Declaraciones
  prueba3.vcxproj                Proyecto MSBuild
  prueba3.vcxproj.filters
  mapa.html                      Representación del mapa de vías
docs/                            Memoria y listado comentado
```

## Compilación

Proyecto de Visual Studio para Windows: abre `lomo2.sln` y compila.

> Dos arreglos sobre la entrega original, necesarios para que el proyecto se pueda abrir:
> el fichero de proyecto venía renombrado como `prueba3.vcxproj.xml` (la solución buscaba
> `prueba3.vcxproj` y no lo encontraba), y sus rutas apuntaban a una carpeta
> `AplicacionLomo` de la máquina donde se desarrolló, que no forma parte de la entrega.
> Ahora son relativas al repositorio.

> **Dependencia externa.** El programa carga en tiempo de ejecución `lomo2.dll`, la
> biblioteca de simulación y renderizado del mapa proporcionada por la asignatura
> (`GenerarMapa`, `PetAvance`, `Avance`, `GetColor`…). No se redistribuye aquí. El código
> la espera en `prueba3/../libs/lomo2.dll`.

---

## Autoría

Práctica desarrollada en pareja por **Miguel Barranquero Díez** y **Alejandro González Rengel**.

Documentación y listado comentado en [`docs/`](docs/).
