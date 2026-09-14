# -*- coding: utf-8 -*-
"""
Árbol de Decisión sobre el dataset Cleveland Heart Disease
Práctica de Sistemas Inteligentes - MUII (USAL)
Diego Fernández Sutil, Miguel Barranquero Diez

Dataset: UCI Machine Learning Repository, ID 45 (Heart Disease)
Subconjunto: Cleveland processed (303 instancias originales, 297 tras
eliminar 6 filas con valores faltantes en 'ca' o 'thal').

Target original: severidad de enfermedad cardíaca (num) en escala 0-4.
Target usado: binario (0 = sano, 1 = enfermo, agrupando severidades 1-4).
"""

# ============================================================
# 1. IMPORTACIONES
# ============================================================
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import warnings

from sklearn.model_selection import train_test_split, GridSearchCV, StratifiedKFold, cross_val_score
from sklearn.tree import DecisionTreeClassifier, plot_tree, export_text
from sklearn.metrics import (accuracy_score, classification_report,
                             confusion_matrix, ConfusionMatrixDisplay)

warnings.filterwarnings("ignore")
np.random.seed(42)
sns.set_style("whitegrid")

print("="*70)
print(" ÁRBOL DE DECISIÓN - HEART DISEASE (CLEVELAND)")
print("="*70)


# ============================================================
# 2. CARGA DEL DATASET
# ============================================================
COLUMN_NAMES = ["age", "sex", "cp", "trestbps", "chol", "fbs", "restecg",
                "thalach", "exang", "oldpeak", "slope", "ca", "thal", "num"]

df = pd.read_csv("processed.cleveland.data",
                 header=None,
                 names=COLUMN_NAMES,
                 na_values="?")

print(f"\n[2] Carga inicial: {df.shape[0]} instancias, {df.shape[1]} columnas")

# ============================================================
# 3. PREPROCESADO
# ============================================================

# 3.1 Eliminar filas con valores faltantes
df_clean = df.dropna().reset_index(drop=True)
print(f"\n[3.1] Tras eliminar NAs: {df_clean.shape[0]} instancias "
      f"(se descartaron {df.shape[0] - df_clean.shape[0]} filas)")

# 3.2 Binarizar el target: 0 (sano) vs 1-4 (enfermo)
df_clean["target"] = (df_clean["num"] > 0).astype(int)
df_clean = df_clean.drop(columns=["num"])

print(f"\n[3.2] Distribución del target binario:")
print(df_clean["target"].value_counts().rename({0: "Sano", 1: "Enfermo"}).to_string())
print(f"      % enfermos: {df_clean['target'].mean()*100:.1f}%")

# 3.3 Separar X / y
feature_names = [c for c in df_clean.columns if c != "target"]
X = df_clean[feature_names].values
y = df_clean["target"].values


# ============================================================
# 4. SPLIT TRAIN/TEST
# ============================================================
X_train, X_test, y_train, y_test = train_test_split(
    X, y,
    test_size=0.25,
    random_state=42,
    stratify=y  # mantiene la proporción de clases en ambos splits
)

print(f"\n[4] Split estratificado 75/25:")
print(f"    Train: {X_train.shape[0]} instancias "
      f"({y_train.mean()*100:.1f}% enfermos)")
print(f"    Test:  {X_test.shape[0]} instancias "
      f"({y_test.mean()*100:.1f}% enfermos)")


# ============================================================
# 5. EXPERIMENTO 1: ÁRBOL SIN PODAR (baseline / overfit)
# ============================================================
print("\n" + "="*70)
print(" EXPERIMENTO 1: Árbol sin podar")
print("="*70)

tree_full = DecisionTreeClassifier(criterion="gini", random_state=42)
tree_full.fit(X_train, y_train)

acc_train_full = accuracy_score(y_train, tree_full.predict(X_train))
acc_test_full = accuracy_score(y_test, tree_full.predict(X_test))

print(f"  Profundidad del árbol: {tree_full.get_depth()}")
print(f"  Número de hojas:       {tree_full.get_n_leaves()}")
print(f"  Accuracy (train): {acc_train_full*100:.2f}%")
print(f"  Accuracy (test):  {acc_test_full*100:.2f}%")
print(f"  Gap train-test:   {(acc_train_full-acc_test_full)*100:.2f}%")


# ============================================================
# 6. EXPERIMENTO 2: BÚSQUEDA DE HIPERPARÁMETROS
# ============================================================
print("\n" + "="*70)
print(" EXPERIMENTO 2: Grid Search con validación cruzada 5-fold")
print("="*70)

param_grid = {
    "criterion": ["gini", "entropy"],
    "max_depth": [2, 3, 4, 5, 6, 8, 10, None],
    "min_samples_split": [2, 5, 10, 20],
    "min_samples_leaf": [1, 2, 5, 10],
}

cv = StratifiedKFold(n_splits=5, shuffle=True, random_state=42)

grid = GridSearchCV(
    estimator=DecisionTreeClassifier(random_state=42),
    param_grid=param_grid,
    scoring="accuracy",
    cv=cv,
    n_jobs=-1,
    verbose=0,
)

grid.fit(X_train, y_train)

print(f"  Mejores hiperparámetros: {grid.best_params_}")
print(f"  Accuracy media CV (5-fold): {grid.best_score_*100:.2f}%")

# Modelo final con los mejores hiperparámetros
tree_best = grid.best_estimator_

acc_train_best = accuracy_score(y_train, tree_best.predict(X_train))
acc_test_best = accuracy_score(y_test, tree_best.predict(X_test))

print(f"\n  Modelo final:")
print(f"    Profundidad: {tree_best.get_depth()}")
print(f"    Nº hojas:    {tree_best.get_n_leaves()}")
print(f"    Accuracy (train): {acc_train_best*100:.2f}%")
print(f"    Accuracy (test):  {acc_test_best*100:.2f}%")
print(f"    Gap train-test:   {(acc_train_best-acc_test_best)*100:.2f}%")


# ============================================================
# 7. MÉTRICAS DETALLADAS DEL MEJOR MODELO
# ============================================================
print("\n" + "="*70)
print(" EVALUACIÓN DETALLADA DEL MEJOR MODELO (sobre test)")
print("="*70)

y_pred = tree_best.predict(X_test)

print("\n[7.1] Classification report:")
print(classification_report(y_test, y_pred,
                            target_names=["Sano", "Enfermo"]))

print("[7.2] Matriz de confusión:")
cm = confusion_matrix(y_test, y_pred)
print(f"    Real Sano    -> Predicho Sano: {cm[0,0]:2d}  |  Enfermo: {cm[0,1]:2d}")
print(f"    Real Enfermo -> Predicho Sano: {cm[1,0]:2d}  |  Enfermo: {cm[1,1]:2d}")

# Sensibilidad / Especificidad (importantes en contexto médico)
tn, fp, fn, tp = cm.ravel()
sens = tp / (tp + fn)
spec = tn / (tn + fp)
print(f"     Sensibilidad (TPR): {sens*100:.1f}%  "
      f"(detección de enfermos reales)")
print(f"     Especificidad (TNR): {spec*100:.1f}%  "
      f"(detección de sanos reales)")


# ============================================================
# 8. IMPORTANCIA DE ATRIBUTOS
# ============================================================
print("\n" + "="*70)
print(" IMPORTANCIA DE ATRIBUTOS")
print("="*70)

importances = pd.Series(tree_best.feature_importances_, index=feature_names)
importances = importances.sort_values(ascending=False)
print(importances.round(3).to_string())


# ============================================================
# 9. REGLAS DEL ÁRBOL EN TEXTO
# ============================================================
print("\n" + "="*70)
print(" REGLAS APRENDIDAS (texto)")
print("="*70)
print(export_text(tree_best, feature_names=feature_names, max_depth=10))


# ============================================================
# 10. VISUALIZACIONES
# ============================================================
print("\n" + "="*70)
print(" GENERANDO GRÁFICOS...")
print("="*70)

# 10.1 Árbol de decisión
fig, ax = plt.subplots(figsize=(20, 10))
plot_tree(tree_best,
          feature_names=feature_names,
          class_names=["Sano", "Enfermo"],
          filled=True,
          rounded=True,
          fontsize=10,
          ax=ax)
plt.title(f"Árbol de decisión final "
          f"(profundidad={tree_best.get_depth()}, "
          f"hojas={tree_best.get_n_leaves()})",
          fontsize=14)
plt.tight_layout()
plt.savefig("arbol_final.png", dpi=120, bbox_inches="tight")
plt.close()
print("  [OK] arbol_final.png")

# 10.2 Matriz de confusión
fig, ax = plt.subplots(figsize=(5, 4))
disp = ConfusionMatrixDisplay(confusion_matrix=cm,
                              display_labels=["Sano", "Enfermo"])
disp.plot(ax=ax, cmap="Blues", colorbar=False)
plt.title("Matriz de confusión (test)")
plt.tight_layout()
plt.savefig("matriz_confusion.png", dpi=120, bbox_inches="tight")
plt.close()
print("  [OK] matriz_confusion.png")

# 10.3 Importancia de atributos
fig, ax = plt.subplots(figsize=(8, 5))
importances.plot(kind="barh", ax=ax, color="steelblue")
ax.invert_yaxis()
plt.title("Importancia de los atributos")
plt.xlabel("Importancia (reducción de Gini)")
plt.tight_layout()
plt.savefig("importancia.png", dpi=120, bbox_inches="tight")
plt.close()
print("  [OK] importancia.png")

# 10.4 Curva train vs test según profundidad (para mostrar overfitting)
depths = list(range(1, 16))
train_scores, test_scores = [], []
for d in depths:
    t = DecisionTreeClassifier(max_depth=d, random_state=42)
    t.fit(X_train, y_train)
    train_scores.append(accuracy_score(y_train, t.predict(X_train)))
    test_scores.append(accuracy_score(y_test, t.predict(X_test)))

fig, ax = plt.subplots(figsize=(8, 5))
ax.plot(depths, train_scores, "o-", label="Train", color="tab:blue")
ax.plot(depths, test_scores, "o-", label="Test", color="tab:red")
ax.axvline(tree_best.get_depth(), ls="--", color="green",
           label=f"Profundidad óptima = {tree_best.get_depth()}")
ax.set_xlabel("Profundidad máxima")
ax.set_ylabel("Accuracy")
ax.set_title("Overfitting vs profundidad del árbol")
ax.legend()
plt.tight_layout()
plt.savefig("curva_overfitting.png", dpi=120, bbox_inches="tight")
plt.close()
print("  [OK] curva_overfitting.png")
