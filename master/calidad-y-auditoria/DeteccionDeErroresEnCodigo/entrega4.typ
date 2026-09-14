#show heading: set text(size: 14pt, weight: "bold")
#set par(justify: true)
#set text(font: "Times New Roman", size: 11pt)

= Introducción 
El presente trabajo se centra en el análisis y la corrección de defectos en cinco programas software proporcionados. 
La estructura de este ejercicio se basa en la aplicación de una metodología de prueba y depuración específica para cada código fuente.
Esta metodología implica:
  - Identiﬁcar defectos.
  - Identiﬁcar un caso de prueba que dé lugar a un fallo.
  - Si es posible, identiﬁcar el caso de prueba que no dé lugar a un fallo.
  - Si es posible, identiﬁcar el caso de prueba no produzca un estado de error.
  - Si es posible, identiﬁcar el caso de prueba que produzca un estado de error pero no produzca un fallo.
  - Corregir el defecto y veriﬁcar que el caso de prueba identiﬁcado en el punto 2 produce el resultado esperado

#pagebreak()
= Programa 1

La  @fig:programa1 corresponde con el primer programa a analizar:

#figure(
  image("capturas/01_Programa.png", width: 90%),
  caption: [Programa 1]
) <fig:programa1>

*Identificar defectos (sin ejecutar el código)*

El defecto del codigo es que el bucle for comienza desde el índice 1 (i=1), es decir, en el segundo elemento del array.
 Este debería de empezar en el indice 0 (i=0) para poder considerar y analizar todos los elementos del array.

 #figure(
  image("capturas/01_Programa1.png", width: 50%),
  caption: [Programa 1 - Defecto]
) <fig:programa1_1>

*Identificar un caso de prueba que dé lugar a un fallo*

Para realizar el caso de prueba que de lugar a fallo introduciré en la primera posicion,
(i=0 del array) un 0, este cero no se tendrá en cuenta por lo que el numero de 0 esperado sera incorrecto.

Para este ejemplo se ha declarado el array x[]= {0,0,0} (@fig:programa1_2):

  -Número de 0 esperado: 3.

  -Número de 0 obtenido: 2.

 #figure(
  image("capturas/01_Programa2.png", width: 90%),
  caption: [Programa 2 - Fallo Ejemplo]
) <fig:programa1_2>

*Si es posible, identificar el caso de prueba que no dé lugar a un fallo*

Para conseguir este caso de prueba bastará con que en la primera posicion (i=0) del array no exista un 0, el defecto sigue estando pero con esta disposicion 
(evitar 0 en la primera poscion) la ejecucion no dará fallo.

Para este ejemplo se ha declarado el array x[]= {1,0,0} (@fig:programa1_3):

  -Número de 0 esperado: 2.

  -Número de 0 obtenido: 2.

 #figure(
  image("capturas/01_Programa3.png", width: 90%),
  caption: [Programa 1 - No Fallo Ejemplo]
) <fig:programa1_3>


* Si es posible, identificar el caso de prueba no produzca un estado de error*

Para este caso de prueba será necesario que el array no tenga ningun 0, de esta manera aunque no se analice el primer elemento, no sera relativo para determinar
cuantos 0 hay en el array puesto que no no existirá ninguno haciendo que no se produca el estado de error.

Para este ejemplo se ha declarado el array x[]= {1,2,3} (@fig:programa1_4):

  -Número de 0 esperado: 0.

  -Número de 0 obtenido: 0.

 #figure(
  image("capturas/01_Programa4.png", width: 90%),
  caption: [Programa 1 - No Fallo Ejemplo]
) <fig:programa1_4>

*Si es posible, identificar el caso de prueba que produzca un estado de error pero no produzca un fallo*

Para este caso sera necesario que en el primer elemento nunca sea 0 para evitar el fallo, pero el estado de error seguirá ahi 
ya que no se evalua la condición (cero/noCero) en el primer elemento (i=0).

Para este ejemplo se ha declarado el array x[]= {99,0,0} (@fig:programa1_5):

  -Número de 0 esperado: 2.

  -Número de 0 obtenido: 2.

 #figure(
  image("capturas/01_Programa5.png", width: 90%),
  caption: [Programa 1 - No Fallo Ejemplo]
) <fig:programa1_5>

*Corregir el defecto y verificar que el caso de prueba identificado en el punto 2 produce el resultado esperado*

Para corregir el defecto de cambia la linea 7: 

-Defecto: for (int i=1 ; i < x.length; i++)

-Corregido: for (int i=0 ; i < x.length; i++)

El caso de prueba del punto 2 correspondia con el siguiente array x[]={0,0,0} (@fig:programa1_6), la ejecución con el defecto corregido:

  -Número de 0 esperado: 3.

  -Número de 0 obtenido: 3.

 #figure(
  image("capturas/01_Programa6.png", width: 90%),
  caption: [Programa 1 - No Fallo No Defecto]
) <fig:programa1_6>


#pagebreak()

= Programa 2

La  @fig:programa2 corresponde con el segundo programa a analizar:

#figure(
  image("capturas/02_Programa.png", width: 90%),
  caption: [Programa 2]
) <fig:programa2>

*Identificar defectos (sin ejecutar el código)*

El defecto del código está en la condición del bucle for en la línea 5, El bucle se detiene cuando i>0, 
lo que significa que nunca evalúa el elemento en la posición 0 (el primer elemento del array). 

 #figure(
  image("capturas/02_Programa1.png", width: 50%),
  caption: [Programa 2 - Defecto]
) <fig:programa2_1>




*Identificar un caso de prueba que dé lugar a un fallo*

Para producir un fallo, el elemento buscado (y) debe estar únicamente en la posición 0 del array.

Para este ejemplo se ha declarado el array x[]= {1,6,7} siendo y = 1 (@fig:programa2_2):

  -Número de índice esperado: 0.

  -Número de índice obtenido: -1.

#figure(
  image("capturas/02_Programa2.png", width: 90%),
  caption: [Programa 2 - Fallo ]
) <fig:programa2_2>

  El programa no encuentra el elemento que sí existe en la posición 0

*Si es posible, identificar el caso de prueba que no dé lugar a un fallo*

Para evitar el fallo, el elemento buscado no debe estar en la posición 0, o debe estar también en otras posiciones.

Para este ejemplo se ha declarado el array x[]= {4,6,1} siendo y = 1 (@fig:programa2_3):

  -Número de índice esperado: 2.

  -Número de índice obtenido: 2.

#figure(
  image("capturas/02_Programa3.png", width: 90%),
  caption: [Programa 2 - No Fallo]
) <fig:programa2_3>

No hay fallo pero el defecto sigue presente.

* Si es posible, identificar el caso de prueba no produzca un estado de error*

Para que no haya estado de error, el bucle debe funcionar correctamente sin necesidad de evaluar la posición 0. Esto ocurre cuando:

-El elemento buscado está en posiciones > 0, O
-El elemento buscado no existe en el array

Para este ejemplo se ha declarado el array int x[] = {5, 3, 7} siendo y = 9 (@fig:programa2_4):

  -Número de índice esperado: -1.

  -Número de índice obtenido: -1.

#figure(
  image("capturas/02_Programa4.png", width: 90%),
  caption: [Programa 2 - No Error]
) <fig:programa2_4>

NO hay estado de error porque el elemento no existe y no importa que no se revise la posición 0



*Si es posible, identificar el caso de prueba que produzca un estado de error pero no produzca un fallo*

Para este caso, el elemento buscado debe estar en la posición 0, pero también en otras posiciones posteriores. 
Así, aunque no se evalúe la posición 0 (estado de error), el método encuentra el elemento en otra posición y devuelve un resultado "aceptable" 
(aunque no sea técnicamente el último).

Para este ejemplo se ha declarado el array int int x[] = {1, 0, 1} siendo y = 1 (@fig:programa2_5):

  -Número de índice esperado: 2.

  -Número de índice obtenido: 2.

#figure(
  image("capturas/02_Programa5.png", width: 90%),
  caption: [Programa 2 - Error No Fallo]
) <fig:programa2_5>



Si hay un estado de error puesto que no revisa la posición 0 del array pero no se produce un fallo, 
pero no hay fallo porque encuentra el elemento en posición 2, 
que efectivamente es la última ocurrencia válida.

*Corregir el defecto y verificar que el caso de prueba identificado en el punto 2 produce el resultado esperado*

Para corregir el defecto se cambia la linea 5: 

-Defecto: for (int i = x.length ‐ 1 ; i > 0; i‐‐)
 
-Corregido: for (int i = x.length ‐ 1 ; i >= 0; i‐‐)

El caso de prueba del punto 2 correspondia con el siguiente array x[]={1,6,7}  y = 1 (@fig:programa2_6), la ejecución con el defecto corregido:

  -Número de índice esperado: 0 .

  -Número de índice obtenido: 0.

 #figure(
  image("capturas/02_Programa6.png", width: 90%),
  caption: [Programa 2 - No Fallo No Defecto]
) <fig:programa2_6>


#pagebreak()

= Programa 3

La  @fig:programa3 corresponde con el tercer programa a analizar:

#figure(
  image("capturas/03_Programa.png", width: 90%),
  caption: [Programa 3]
) <fig:programa3>

*Identificar defectos (sin ejecutar el código)*

*Identificar un caso de prueba que dé lugar a un fallo*

*Si es posible, identificar el caso de prueba que no dé lugar a un fallo*

* Si es posible, identificar el caso de prueba no produzca un estado de error*

*Si es posible, identificar el caso de prueba que produzca un estado de error pero no produzca un fallo*

*Corregir el defecto y verificar que el caso de prueba identificado en el punto 2 produce el resultado esperado*

#pagebreak()

= Programa 4

La  @fig:programa4 corresponde con el cuarto programa a analizar:

#figure(
  image("capturas/04_Programa.png", width: 90%),
  caption: [Programa 4]
) <fig:programa4>

*Identificar defectos (sin ejecutar el código)*


*Identificar un caso de prueba que dé lugar a un fallo*

*Si es posible, identificar el caso de prueba que no dé lugar a un fallo*

* Si es posible, identificar el caso de prueba no produzca un estado de error*

*Si es posible, identificar el caso de prueba que produzca un estado de error pero no produzca un fallo*

*Corregir el defecto y verificar que el caso de prueba identificado en el punto 2 produce el resultado esperado*

#pagebreak()

= Programa 5

La  @fig:programa5 corresponde con el quinto programa a analizar:

#figure(
  image("capturas/05_Programa.png", width: 90%),
  caption: [Programa 5]
) <fig:programa5>

*Identificar defectos (sin ejecutar el código)*

*Identificar un caso de prueba que dé lugar a un fallo*

*Si es posible, identificar el caso de prueba que no dé lugar a un fallo*

* Si es posible, identificar el caso de prueba no produzca un estado de error*

*Si es posible, identificar el caso de prueba que produzca un estado de error pero no produzca un fallo*

*Corregir el defecto y verificar que el caso de prueba identificado en el punto 2 produce el resultado esperado*

#pagebreak()


