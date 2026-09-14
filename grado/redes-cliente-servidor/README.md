# Servicio de preguntas sobre sockets TCP y UDP

Un juego de preguntas y respuestas implementado como servicio de red en C, con un protocolo
de aplicación propio de estilo textual y **el mismo servicio ofrecido simultáneamente sobre
TCP y sobre UDP**.

Asignatura: *Redes* — Grado en Ingeniería Informática (USAL).

---

## El protocolo

Protocolo textual con códigos numéricos de respuesta, al estilo de SMTP o FTP:

| Código | Significado |
|---|---|
| `220` | Servicio preparado |
| `250` | Orden aceptada |
| `500` | Error de sintaxis |

Órdenes del cliente: `HOLA` para iniciar sesión, `+` para solicitar una pregunta nueva,
`RESPUESTA <texto>` para contestar y `ADIOS` para cerrar. El servidor controla el número de
intentos por pregunta y sólo pasa a la siguiente cuando se acierta o se agotan.

Toda orden fuera de secuencia —responder sin haber saludado, por ejemplo— se rechaza con
`500`, de modo que el servidor mantiene una **máquina de estados por conexión**.

## TCP y UDP en el mismo servidor

El interés de la práctica está aquí. El servidor ([`servidor.c`](servidor.c), ~2.000 líneas)
atiende ambos transportes:

- **TCP**, orientado a conexión: cada cliente tiene su propio descriptor y el estado de la
  partida vive asociado a la conexión.
- **UDP**, sin conexión: no hay sesión que sostenga el estado, así que el servidor debe
  identificar al cliente por su dirección de origen y mantener la tabla de partidas él mismo.

El servidor se ejecuta como **daemon** y registra la actividad de cada sesión.

## Ficheros

```
servidor.c        Servidor TCP + UDP, daemonización y máquina de estados
cliente.c         Cliente para ambos transportes
preguntas.txt     Banco de preguntas, formato "enunciado#respuesta"
ordenes*.txt      Guiones de órdenes para pruebas automatizadas
lanzaServidor.sh  Arranca el servidor y seis clientes concurrentes (3 TCP, 3 UDP)
Makefile
```

## Compilación y ejecución

```bash
make
./servidor
./cliente localhost TCP ordenes.txt
./cliente localhost UDP ordenes.txt
```

> Dos correcciones sobre la entrega original, ambas necesarias para que compile y no
> corrompa la pila: el `Makefile` invocaba `clear` como parte de la construcción (falla sin
> terminal interactiva) y en `cliente.c` había un `memset` de 1024 bytes sobre un buffer de
> 256, que desbordaba la pila. Compilado con `-Wall -Wextra` sobre GCC 13.

O la prueba de carga completa, con seis clientes concurrentes:

```bash
./lanzaServidor.sh
```
