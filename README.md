# Proyectos académicos

Recopilación de prácticas y trabajos desarrollados durante el **Grado en Ingeniería
Informática** y el **Máster Universitario en Ingeniería Informática** de la Universidad de
Salamanca.

Cada carpeta es un proyecto autónomo con su propio README, su código y la documentación
entregada. Los tres proyectos de mayor envergadura tienen repositorio propio y están
enlazados al final.

---

## Máster Universitario en Ingeniería Informática

| Proyecto | Asignatura | Tecnologías |
|---|---|---|
| [Sistemas inteligentes](master/sistemas-inteligentes/) | Sistemas Inteligentes | Python · scikit-learn · C · Lógica difusa · Gensym G2 |
| [Calidad y auditoría de software](master/calidad-y-auditoria/) | Calidad y Auditoría | Python · pytest · Análisis estático · Java · C |
| [Infraestructura AWS con Terraform](master/infraestructura-aws-terraform/) | DAIITI | Terraform · AWS EC2 · IaC |
| [Adivinación distribuida con MPI](master/arquitectura-computadores-mpi/) | Arquitectura de Computadores | C · MPI |

## Grado en Ingeniería Informática

| Proyecto | Asignatura | Tecnologías |
|---|---|---|
| [Servicio de preguntas TCP/UDP](grado/redes-cliente-servidor/) | Redes | C · Sockets BSD · Daemons |
| [Servicio REST distribuido](grado/sistemas-distribuidos-rest/) | Sistemas Distribuidos | Java · JAX-RS · Jersey · Tomcat |
| [Simulación concurrente de trenes](grado/sistemas-operativos-concurrencia/) | Sistemas Operativos | C++ · Win32 · Hilos y semáforos |

---

## Proyectos con repositorio propio

| Proyecto | Descripción |
|---|---|
| **Xana** | Sistema de gestión hospitalaria. Trabajo de Fin de Grado. Node.js, Express, MySQL, Socket.IO, integración con la API CIE-11 de la OMS |
| **F1-Service** | Agregador de siete APIs públicas sobre Fórmula 1. TypeScript, Express, MySQL |
| **ModelGen** | Generación de esquemas de datos a partir de lenguaje natural mediante LLM. Node.js, Groq, PlantUML |
| **waterwave-hpc** | Simulación de aguas someras paralelizada de cinco formas distintas: Pthreads, OpenMP, MPI y dos híbridos. C |

---

## Nota

El código se publica tal y como se entregó, salvo la retirada de credenciales, datos
personales y artefactos de compilación. No se ha reescrito para corregir a posteriori
decisiones de diseño de la época: refleja el nivel y el contexto de cada asignatura.

Sí se ha comprobado que **todo compila**: los proyectos en C con `gcc` y OpenMPI, y los de
Java con `javac` y Maven. En el proceso salieron un par de cosas que la entrega original
arrastraba y que sí se han corregido, porque impedían compilar: un `Makefile` que apuntaba a
un fuente inexistente, un `memset` que desbordaba un buffer de la pila y unos fuentes Java
guardados en Latin-1.

## Autor

**Miguel Barranquero Díez** — Universidad de Salamanca.
Algunos proyectos se desarrollaron en pareja; la autoría compartida se indica en el README
de cada uno.

## Licencia

MIT. Ver [LICENSE](LICENSE).
