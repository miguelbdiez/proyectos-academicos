# Calidad y auditoría de software

Dos trabajos sobre las dos caras de la verificación: encontrar defectos en código ajeno, y
escribir pruebas que impidan que aparezcan en el propio.

Asignatura: *Calidad y Auditoría de Software* — Máster Universitario en Ingeniería
Informática (USAL).

---

## [`DeteccionDeErroresEnCodigo/`](DeteccionDeErroresEnCodigo/)

Análisis de defectos sobre cinco programas en C y Java, aplicando de forma sistemática la
distinción entre **defecto, estado de error y fallo**. Para cada programa se documenta:

1. El defecto, identificado por inspección y sin ejecutar el código
2. Un caso de prueba que provoca un fallo observable
3. Un caso de prueba que no lo provoca, pese al mismo defecto
4. Un caso que alcanza un estado de error pero **no** llega a manifestarse como fallo
5. La corrección, y la verificación de que el caso del punto 2 ya da el resultado esperado

El punto 4 es el interesante: demuestra por qué la ausencia de fallos observados no implica
ausencia de defectos, que es el argumento de fondo de toda la disciplina de pruebas.

El código analizado está en [`codigoProgramas/`](DeteccionDeErroresEnCodigo/codigoProgramas/),
separado en las dos versiones de cada programa:

```
codigoProgramas/
├── conDefecto/    versión entregada, con el defecto por identificar
└── corregido/     versión tras aplicar la corrección
```

La diferencia entre ambas es exactamente el defecto documentado en el informe. En los dos
programas resultó ser un **error por uno** en el índice inicial del bucle (`i=1` en lugar de
`i=0`, e `i > 0` en lugar de `i >= 0`), de modo que el primer elemento del array nunca se
examinaba.

---

## [`PruebasUnitarias/`](PruebasUnitarias/)

Batería de pruebas unitarias con **pytest** sobre un módulo de calculadora, con medición de
cobertura e informe HTML.

Cubre casos nominales, casos límite (multiplicación por cero), gestión de excepciones
(división por cero mediante `pytest.raises`) y **pruebas parametrizadas** para cubrir varias
entradas con un solo test.

```bash
cd PruebasUnitarias/proyecto_testing
pip install -r requirements.txt
pytest --cov=src --cov-report=html --html=reporte.html
```

---

## Informes

Las cinco entregas de la asignatura ([`entrega1.pdf`](entrega1.pdf) … [`entrega5.pdf`](entrega5.pdf))
están redactadas en **Typst**; el fuente `.typ` acompaña a cada una y es recompilable con:

```bash
typst compile main.typ
```
