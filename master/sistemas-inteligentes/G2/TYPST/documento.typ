
= Introducción

Este informe documenta el desarrollo de un sistema experto médico implementado en G2, diseñado para actuar como un médico virtual capaz de analizar síntomas básicos y ofrecer orientación preliminar. El sistema clasifica los síntomas del paciente en dos categorías principales (generales o localizados) y, mediante un proceso de razonamiento basado en reglas, proporciona diagnósticos preliminares y recomendaciones apropiadas.

El objetivo de esta práctica es aplicar los conceptos de sistemas expertos y razonamiento basado en conocimiento para crear una herramienta que pueda asistir en la evaluación inicial de síntomas médicos comunes.

#pagebreak()
= Arquitectura del Sistema

== Estructura de Clases

El sistema se fundamenta en la clase `PACIENTE`, que modela la información necesaria para realizar el diagnóstico médico. A continuación se presenta la definición de la clase:

```gsi
define class PACIENTE
  with
    sintoma_principal initially is desconocido;
    tipo_sintoma initially is desconocido;
    tiene_fiebre initially is desconocido;
    acompanamiento_fiebre initially is desconocido;
    tiene_cansancio initially is desconocido;
    tipo_cansancio initially is desconocido;
    ubicacion_dolor initially is desconocido;
    tipo_dolor initially is desconocido;
    diagnostico initially is desconocido;
    recomendacion initially is desconocido;
end define
```

=== Descripción de Atributos

- *sintoma_principal*: Identifica el síntoma que reporta el paciente (fiebre, cansancio, dolor_abdominal, dolor_pecho)
- *tipo_sintoma*: Clasifica el síntoma como general o localizado
- *acompanamiento_fiebre*: Síntomas adicionales cuando hay fiebre (escalofríos, sudoración_nocturna, ninguno)
- *tipo_cansancio*: Características del cansancio reportado (estres_falta_sueno, persistente_sin_causa, con_fiebre_perdida_peso)
- *ubicacion_dolor*: Localización del dolor abdominal (lado_derecho, lado_izquierdo, generalizado)
- *tipo_dolor*: Naturaleza del dolor de pecho (punzante, presion)
- *diagnostico*: Identificador del diagnóstico alcanzado
- *recomendacion*: Texto con la recomendación médica

#pagebreak()
== Objeto Principal

En el workspace `SISTEMA_DIAGNOSTICO_MEDICO` se define el objeto principal del sistema:

```gsi
define object PACIENTE_ACTUAL instance of PACIENTE
```

Este objeto mantiene el estado del paciente durante toda la sesión de diagnóstico y contiene un subworkspace llamado `DIAGNOSTICO_FINAL` donde se presenta el resultado final del análisis.

#figure(
  image("/capturas/workspace_sistema.png", width: 80%),
  caption: [Workspace SISTEMA_DIAGNOSTICO_MEDICO con el objeto PACIENTE_ACTUAL]
)

#pagebreak()
== Variables de Interfaz

El sistema utiliza variables de tipo símbolo para capturar las respuestas del usuario. Estas variables actúan como puente entre la interfaz gráfica y el motor de inferencia:

#figure(
  image("/capturas/workspace_variables.png", width: 80%),
  caption: [Workspace VARIABLES con las variables de captura]
)

Las variables definidas son:
- `SINTOMA_VAR`: Captura el síntoma principal
- `FIEBRE_VAR`: Captura el tipo de fiebre
- `CANSANCIO_VAR`: Captura el tipo de cansancio
- `ACOMP_FIEBRE_VAR`: Captura el acompañamiento de la fiebre
- `UBICACION_DOLOR_VAR`: Captura la ubicación del dolor abdominal
- `TIPO_DOLOR_VAR`: Captura el tipo de dolor de pecho

#pagebreak()
= Interfaz de Usuario

El sistema presenta al usuario una serie de preguntas mediante workspaces interactivos que contienen radio buttons para seleccionar opciones. Cada workspace está diseñado para recopilar información específica según el flujo de diagnóstico.

== Pregunta Inicial: Síntoma Principal

#figure(
  image("/capturas/workspace_principal_ask.png", width: 70%),
  caption: [Interfaz para seleccionar el síntoma principal]
)

Esta es la primera pregunta que se presenta al usuario, permitiendo elegir entre cuatro síntomas principales: Fiebre, Cansancio, Dolor Abdominal y Dolor de Pecho.

== Preguntas sobre Fiebre

#figure(
  image("/capturas/fiebre_ask.png", width: 70%),
  caption: [Interfaz para evaluar el acompañamiento de la fiebre]
)

Cuando el usuario reporta fiebre, el sistema pregunta sobre síntomas adicionales para determinar la gravedad.

== Preguntas sobre Cansancio

#figure(
  image("/capturas/cansancio_ask.png", width: 70%),
  caption: [Interfaz para evaluar el tipo de cansancio]
)

Para casos de cansancio, el sistema evalúa las características y síntomas asociados.

== Preguntas sobre Dolor Abdominal

#figure(
  image("/capturas/dolor_abdominal_ask.png", width: 70%),
  caption: [Interfaz para determinar la ubicación del dolor abdominal]
)

El sistema localiza el dolor abdominal para proporcionar un diagnóstico más específico.

#pagebreak()
== Preguntas sobre Dolor de Pecho

#figure(
  image("/capturas/dolor_pecho_ask.png", width: 70%),
  caption: [Interfaz para caracterizar el dolor de pecho]
)

Es crucial distinguir entre diferentes tipos de dolor torácico debido a sus implicaciones clínicas.

#pagebreak()
= Motor de Reglas

El sistema experto utiliza un conjunto de reglas que implementan el razonamiento médico. Las reglas están organizadas en grupos según el tipo de síntoma.

== Regla de Inicialización

```gsi
initially in order 
    show sintoma_principal_ask and 
    conclude that the sintoma_principal of paciente_actual is desconocido and
    conclude that the acompanamiento_fiebre of paciente_actual is desconocido and
    conclude that the tipo_cansancio of paciente_actual is desconocido and
    conclude that the ubicacion_dolor of paciente_actual is desconocido and
    conclude that the tipo_dolor of paciente_actual is desconocido and
    conclude that the diagnostico of paciente_actual is desconocido
```

Esta regla se ejecuta al inicio del sistema, inicializando todos los atributos y mostrando la pregunta inicial.

== Clasificación de Síntomas

El sistema primero clasifica el síntoma como general o localizado:

```gsi
for any paciente P if the sintoma_principal of P is fiebre then 
    conclude that the tipo_sintoma of P is general and
    invoke fiebre_reglas rules

for any paciente P if the sintoma_principal of P is cansancio then 
    conclude that the tipo_sintoma of P is general and
    invoke cansancio_reglas rules

for any paciente P if the sintoma_principal of P is dolor_abdominal then 
    conclude that the tipo_sintoma of P is localizado and
    invoke dolor_abdominal_reglas rules

for any paciente P if the sintoma_principal of P is dolor_pecho then 
    conclude that the tipo_sintoma of P is localizado and
    invoke dolor_pecho_reglas rules
```

== Reglas de Diagnóstico

=== Síntomas Generales: Fiebre

El sistema analiza la fiebre según sus síntomas acompañantes:

*Fiebre con escalofríos* → Infección grave (atención inmediata):
```gsi
for any paciente P if the acompanamiento_fiebre of P is escalofrios then 
    conclude that the diagnostico of P is infeccion_grave
```

*Fiebre con sudoración nocturna* → Infecciones crónicas (estudios específicos):
```gsi
for any paciente P if the acompanamiento_fiebre of P is sudoracion_nocturna then 
    conclude that the diagnostico of P is infeccion_cronica
```

*Fiebre sin otros síntomas* → Fiebre simple (reposo e hidratación):
```gsi
for any paciente P if the acompanamiento_fiebre of P is ninguno then 
    conclude that the diagnostico of P is fiebre_simple
```

=== Síntomas Generales: Cansancio

*Cansancio por estrés/falta de sueño* → Cambios en estilo de vida:
```gsi
for any paciente P if the tipo_cansancio of P is estres_falta_sueno then 
    conclude that the diagnostico of P is cansancio_estilo_vida
```

*Cansancio persistente sin causa* → Posible anemia o hipotiroidismo:
```gsi
for any paciente P if the tipo_cansancio of P is persistente_sin_causa then 
    conclude that the diagnostico of P is posible_anemia
```

*Cansancio con fiebre/pérdida de peso* → Causas graves (consulta urgente):
```gsi
for any paciente P if the tipo_cansancio of P is con_fiebre_perdida_peso then 
    conclude that the diagnostico of P is cansancio_grave
```

=== Síntomas Localizados: Dolor Abdominal

*Dolor en lado derecho* → Posible apendicitis (atención urgente):
```gsi
for any paciente P if the ubicacion_dolor of P is lado_derecho then 
    conclude that the diagnostico of P is posible_apendicitis
```

*Dolor en lado izquierdo* → Problemas gástricos o pancreáticos:
```gsi
for any paciente P if the ubicacion_dolor of P is lado_izquierdo then 
    conclude that the diagnostico of P is problemas_gastricos
```

*Dolor generalizado* → Indigestión o inflamación general:
```gsi
for any paciente P if the ubicacion_dolor of P is generalizado then 
    conclude that the diagnostico of P is indigestion
```

=== Síntomas Localizados: Dolor de Pecho

*Dolor punzante* → Dolor muscular (reposo):
```gsi
for any paciente P if the tipo_dolor of P is punzante then 
    conclude that the diagnostico of P is dolor_muscular
```

*Dolor con sensación de presión* → Posible problema cardiovascular (urgente):
```gsi
for any paciente P if the tipo_dolor of P is presion then 
    conclude that the diagnostico of P is problema_cardiovascular
```

== Presentación de Diagnósticos

Cada diagnóstico tiene asociado un mensaje final que se presenta al usuario. Por ejemplo:

```gsi
whenever the diagnostico of any paciente P receives a value and 
    when the diagnostico of P is infeccion_grave then 
    inform the operator on the subworkspace of paciente_actual that 
    "DIAGNÓSTICO: Posible infección grave. 
     RECOMENDACIÓN: Requiere atención médica inmediata" and 
    show the subworkspace of paciente_actual and 
    pause knowledge-base
```

#pagebreak()

= Ejemplo: Fiebre con Escalofríos

Este caso representa una situación de urgencia médica:

*Flujo de ejecución:*
1. Usuario selecciona "Fiebre" como síntoma principal
2. Sistema pregunta sobre acompañamiento
3. Usuario selecciona "Escalofríos"
4. Sistema diagnostica infección grave y recomienda atención inmediata

#figure(
  image("/capturas/resultado_fiebre_escalofrios.png", width: 85%),
  caption: [Subworkspace DIAGNOSTICO_FINAL mostrando el resultado para fiebre con escalofríos]
)
