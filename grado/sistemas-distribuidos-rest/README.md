# Multidifusión con orden total sobre servicios REST

Implementación de un algoritmo de **multidifusión con orden total** (*total-order multicast*)
entre varios nodos que se comunican exclusivamente mediante servicios web REST. Todos los
nodos entregan los mensajes en el mismo orden, sin reloj global ni coordinador central.

Asignatura: *Sistemas Distribuidos* — Grado en Ingeniería Informática (USAL).

---

## El algoritmo

Es un acuerdo en dos fases, del estilo del algoritmo de ISIS:

**Fase 1 — Propuesta.** El emisor difunde el mensaje a todos los nodos con `tipoMsj = 0` y
orden provisional. Cada receptor lo almacena como *no entregable* (`flagApertura = 0`) y
devuelve su propia propuesta de número de secuencia.

**Fase 2 — Acuerdo.** El emisor toma el **máximo** de las propuestas recibidas y lo difunde
como orden definitivo (`tipoMsj = 1`, `flagApertura = 1`). Cada receptor actualiza el mensaje
en su cola, lo marca como entregable y reordena la cola por número de secuencia.

Como todos los nodos aplican el mismo criterio de desempate sobre el mismo conjunto de
números acordados, la secuencia de entrega es idéntica en todos ellos aunque los mensajes
lleguen a la red en órdenes distintos.

Un tercer tipo (`tipoMsj = 2`) señaliza la finalización de un nodo.

Para que el problema no sea trivial, cada nodo introduce **esperas aleatorias** entre envíos
(`esperaAleatoriaEntreMsj`, `esperaAletoriaMultidifusion`), forzando entrelazados distintos
en cada ejecución. La traza resultante se vuelca a fichero para poder comprobar que el orden
final coincide entre nodos.

## API REST

Servicio JAX-RS bajo `/servidor`:

| Método | Ruta | Función |
|---|---|---|
| `GET` | `/servidor/hola_Mundo` | Prueba de vida del nodo |
| `GET` | `/servidor/start` | Arranca el proceso de multidifusión |
| `GET` | `/servidor/reinicio` | Reinicia el estado del nodo |
| `GET` | `/servidor/buzon` | Consulta la cola de mensajes y su orden actual |
| `GET` | `/servidor/close` | Detiene el nodo |

## Estructura

```
src/main/java/servicios/
  Servidor.java    Recurso JAX-RS (@Singleton) y estado compartido del nodo
  Proceso.java     Hilo del nodo: emisión, recogida de propuestas y ordenación
  Mensaje.java     Mensaje del protocolo (tipo, id, contenido, orden, flag, origen)
src/main/webapp/
  WEB-INF/web.xml  Configuración del despliegue
  index.html
```

## Despliegue

Aplicación web Java para **Apache Tomcat 8.5**, con **Jersey 2.x** como implementación de
JAX-RS.

```bash
mvn package        # genera target/practicaObligatoria.war
```

> La entrega original no traía descriptor de construcción: las dependencias de Jersey se
> copiaban a mano en `WEB-INF/lib`. Se ha añadido un [`pom.xml`](pom.xml) que las declara, de
> modo que el WAR se construye sin arrastrar los JAR al repositorio.

Después:

1. Despliega el WAR en Tomcat.
2. Levanta **tres instancias** en puertos distintos y anota sus direcciones: cada nodo
   necesita conocer las de sus pares.
3. Invoca `GET /servidor/start` en cada una y consulta `GET /servidor/buzon` para comparar el
   orden de entrega resultante.
