# Sistemas inteligentes

Cuatro prácticas que recorren paradigmas distintos de la inteligencia artificial: desde el
aprendizaje automático clásico hasta los sistemas expertos basados en reglas.

Asignatura: *Sistemas Inteligentes* — Máster Universitario en Ingeniería Informática (USAL).

---

## [`ARBOLES_BINARIOS/`](ARBOLES_BINARIOS/) — Árbol de decisión

Clasificación de **enfermedad cardíaca** sobre el dataset *Cleveland Heart Disease* (UCI,
297 instancias tras depuración). El objetivo original es la severidad en escala 0–4; aquí se
binariza a sano / enfermo.

El estudio no se queda en entrenar el modelo: analiza la **curva de sobreajuste** frente a la
profundidad del árbol para elegir el punto de corte, y examina la **importancia de cada
variable** clínica en la decisión.

```bash
cd ARBOLES_BINARIOS
pip install -r requirements.txt
python arbol_heart_disease.py
```

Salidas: `arbol_final.png`, `curva_overfitting.png`, `importancia.png`, `matriz_confusion.png`.
Informe completo en [`INFORME_ARBOL_DECISION.pdf`](ARBOLES_BINARIOS/INFORME_ARBOL_DECISION.pdf).

**Stack:** Python · scikit-learn · pandas · matplotlib

---

## [`BAYES/`](BAYES/) — Clasificador Naive Bayes en C

Implementación **desde cero, sin librerías**, de un clasificador Naive Bayes gaussiano sobre
el dataset *Palmer Penguins* (clasificación de especie a partir de medidas morfológicas).

Escribirlo en C obliga a implementar a mano el cálculo de medias y varianzas por clase, la
densidad gaussiana y la comparación de log-verosimilitudes, en lugar de delegarlo en
`sklearn`.

```bash
cd BAYES
gcc -o naive_bayes naive_bayes.c -lm
./naive_bayes
```

Informe en [`Informe_Naive_Bayes_Palmer_Penguins.pdf`](BAYES/Informe_Naive_Bayes_Palmer_Penguins.pdf).

**Stack:** C

---

## [`DIFUSA/`](DIFUSA/) — Control difuso de Mamdani

Controlador de la velocidad de un **ventilador** en función de temperatura y humedad,
mediante inferencia difusa tipo Mamdani: conjuntos difusos de entrada, base de reglas
lingüísticas y desdifuscación por centroide.

El fichero [`ventilador.fis`](DIFUSA/ventilador.fis) es el sistema completo, cargable desde
MATLAB (Fuzzy Logic Toolbox) o desde Octave con el paquete `fuzzy-logic-toolkit`.

Informe en [`INFORME_MAMDANI.pdf`](DIFUSA/INFORME_MAMDANI.pdf).

**Stack:** MATLAB / Octave · Lógica difusa

---

## [`G2/`](G2/) — Sistema experto en Gensym G2

Sistema experto de **triaje médico** desarrollado en Gensym G2: a partir de los síntomas que
el paciente confirma o descarta, el motor de inferencia encadena reglas hasta proponer una
orientación diagnóstica.

La base de reglas está en [`MECANICA_RULES.txt`](G2/MECANICA_RULES.txt) y la memoria, con las
capturas del entorno y la traza de una consulta completa, en [`g2.pdf`](G2/TYPST/g2.pdf).

> El entorno Gensym G2 es software propietario y no se incluye en el repositorio.

**Stack:** Gensym G2 · Sistemas basados en reglas

---

## Autoría

Prácticas desarrolladas en pareja por **Miguel Barranquero Díez** y **Diego Fernández Sutil**.
