/*
 * Clasificador Bayesiano Naive (Gaussiano) sobre el dataset Palmer Penguins.
 *
 * Autores: Miguel Barranquero Diez, Diego Fernandez Sutil
 * Asignatura: Sistemas Inteligentes (MUII) - Universidad de Salamanca
 *
 * Compilacion:
 *   gcc -O2 -Wall -o naive_bayes naive_bayes.c -lm
 *   icx  -O2 -Wall -o naive_bayes.exe naive_bayes.c
 *
 * Ejecucion:
 *   ./naive_bayes [ruta_csv] [semilla]
 *
 * Implementa:
 *   - Carga del CSV de Palmer Penguins (delimitador ';', valores NA filtrados).
 *   - Naive Bayes Gaussiano (verosimilitud N(mu, sigma^2) por atributo y clase).
 *   - Calculo en log-espacio para evitar underflow numerico.
 *   - Particion estratificada train/test (70/30).
 *   - Validacion cruzada estratificada (5-fold y 10-fold).
 *   - Matriz de confusion, accuracy y metricas precision/recall/F1 por clase.
 *
 * Atributos numericos utilizados:
 *   culmen_length_mm, culmen_depth_mm, flipper_length_mm, body_mass_g
 * Clases: Adelie, Chinstrap, Gentoo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_SAMPLES   400
#define N_FEATURES    4
#define N_CLASSES     3
#define MAX_LINE      512
#define MAX_NAME      32
#define N_CSV_FIELDS  11

typedef struct {
    double features[N_FEATURES];
    int class_id;
} Sample;

typedef struct {
    double prior[N_CLASSES];
    double mean[N_CLASSES][N_FEATURES];
    double var[N_CLASSES][N_FEATURES];
} NBModel;

static const char *class_names[N_CLASSES] = {
    "Adelie", "Chinstrap", "Gentoo"
};

static const char *feature_names[N_FEATURES] = {
    "culmen_length_mm", "culmen_depth_mm", "flipper_length_mm", "body_mass_g"
};

/* --------------------- Carga del dataset --------------------- */

static int get_class_id(const char *name) {
    for (int i = 0; i < N_CLASSES; i++) {
        if (strcmp(name, class_names[i]) == 0) return i;
    }
    return -1;
}

/*
 * Carga el CSV Palmer Penguins (separador ';').
 * Esquema esperado (11 campos por linea):
 *   individual_id ; clutch_completion ; date_egg ; culmen_length_mm ;
 *   culmen_depth_mm ; flipper_length_mm ; body_mass_g ; sex ;
 *   delta_15_N ; delta_13_C ; species
 * Las filas con NA en alguno de los 4 atributos morfometricos se descartan.
 */
static int load_csv(const char *path, Sample *data, int *skipped_na) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }
    char line[MAX_LINE];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return 0; }

    int n = 0, na = 0;
    while (fgets(line, sizeof(line), f) && n < MAX_SAMPLES) {
        /* Recortar saltos de linea y espacios al final */
        size_t L = strlen(line);
        while (L > 0 && (line[L-1] == '\n' || line[L-1] == '\r'
                      || line[L-1] == ' '  || line[L-1] == '\t')) {
            line[--L] = '\0';
        }
        if (L == 0) continue;

        /* Particion por ';' in-place */
        char *fields[N_CSV_FIELDS] = {0};
        int nf = 0;
        char *tok = line;
        for (char *p = line; ; p++) {
            if (*p == ';' || *p == '\0') {
                int last = (*p == '\0');
                *p = '\0';
                if (nf < N_CSV_FIELDS) fields[nf++] = tok;
                if (last) break;
                tok = p + 1;
            }
        }
        if (nf < N_CSV_FIELDS) continue;

        /* Indices: 3=culmen_length, 4=culmen_depth, 5=flipper_length,
         *          6=body_mass, 10=species. Filtra NA en cualquiera. */
        if (strcmp(fields[3], "NA") == 0 || strcmp(fields[4], "NA") == 0 ||
            strcmp(fields[5], "NA") == 0 || strcmp(fields[6], "NA") == 0) {
            na++;
            continue;
        }
        int cid = get_class_id(fields[10]);
        if (cid < 0) continue;
        data[n].features[0] = atof(fields[3]);
        data[n].features[1] = atof(fields[4]);
        data[n].features[2] = atof(fields[5]);
        data[n].features[3] = atof(fields[6]);
        data[n].class_id    = cid;
        n++;
    }
    fclose(f);
    if (skipped_na) *skipped_na = na;
    return n;
}

/* --------------------- Utilidades --------------------- */

static void shuffle(Sample *data, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Sample t = data[i]; data[i] = data[j]; data[j] = t;
    }
}

/* --------------------- Entrenamiento Naive Bayes --------------------- */

static void train_nb(NBModel *m, const Sample *train, int ntrain) {
    int counts[N_CLASSES] = {0};
    double sum[N_CLASSES][N_FEATURES]   = {{0}};
    double sumsq[N_CLASSES][N_FEATURES] = {{0}};

    for (int i = 0; i < ntrain; i++) {
        int c = train[i].class_id;
        counts[c]++;
        for (int j = 0; j < N_FEATURES; j++) {
            sum[c][j] += train[i].features[j];
        }
    }
    for (int c = 0; c < N_CLASSES; c++) {
        m->prior[c] = (double)counts[c] / ntrain;
        for (int j = 0; j < N_FEATURES; j++) {
            m->mean[c][j] = (counts[c] > 0) ? sum[c][j] / counts[c] : 0.0;
        }
    }
    for (int i = 0; i < ntrain; i++) {
        int c = train[i].class_id;
        for (int j = 0; j < N_FEATURES; j++) {
            double diff = train[i].features[j] - m->mean[c][j];
            sumsq[c][j] += diff * diff;
        }
    }
    for (int c = 0; c < N_CLASSES; c++) {
        for (int j = 0; j < N_FEATURES; j++) {
            double denom = (counts[c] > 1) ? (counts[c] - 1) : 1; /* varianza muestral */
            m->var[c][j] = sumsq[c][j] / denom;
            if (m->var[c][j] < 1e-9) m->var[c][j] = 1e-9; /* suavizado de varianza */
        }
    }
}

/* --------------------- Inferencia --------------------- */

static double log_gaussian(double x, double mean, double var) {
    return -0.5 * log(2.0 * M_PI * var)
           - 0.5 * (x - mean) * (x - mean) / var;
}

static int predict(const NBModel *m, const double *x, double *posterior) {
    double log_p[N_CLASSES];
    double max_lp = -INFINITY;
    for (int c = 0; c < N_CLASSES; c++) {
        log_p[c] = (m->prior[c] > 0) ? log(m->prior[c]) : -INFINITY;
        for (int j = 0; j < N_FEATURES; j++) {
            log_p[c] += log_gaussian(x[j], m->mean[c][j], m->var[c][j]);
        }
        if (log_p[c] > max_lp) max_lp = log_p[c];
    }
    /* Normalizacion softmax-estable para obtener P(c|x) */
    double sum = 0;
    for (int c = 0; c < N_CLASSES; c++) {
        posterior[c] = exp(log_p[c] - max_lp);
        sum += posterior[c];
    }
    int best = 0;
    for (int c = 0; c < N_CLASSES; c++) {
        posterior[c] /= sum;
        if (posterior[c] > posterior[best]) best = c;
    }
    return best;
}

/* --------------------- Evaluacion --------------------- */

static void evaluate(const NBModel *m, const Sample *test, int ntest,
                     int conf[N_CLASSES][N_CLASSES]) {
    for (int i = 0; i < N_CLASSES; i++)
        for (int j = 0; j < N_CLASSES; j++) conf[i][j] = 0;
    for (int i = 0; i < ntest; i++) {
        double post[N_CLASSES];
        int pred = predict(m, test[i].features, post);
        conf[test[i].class_id][pred]++;
    }
}

static double accuracy_from_conf(int conf[N_CLASSES][N_CLASSES]) {
    int correct = 0, total = 0;
    for (int i = 0; i < N_CLASSES; i++)
        for (int j = 0; j < N_CLASSES; j++) {
            total += conf[i][j];
            if (i == j) correct += conf[i][j];
        }
    return (total > 0) ? (double)correct / total : 0.0;
}

static void print_confusion(int conf[N_CLASSES][N_CLASSES]) {
    printf("Matriz de confusion (filas = clase real, columnas = clase predicha):\n");
    printf("%-18s", "");
    for (int j = 0; j < N_CLASSES; j++) printf("%-18s", class_names[j]);
    printf("\n");
    for (int i = 0; i < N_CLASSES; i++) {
        printf("%-18s", class_names[i]);
        for (int j = 0; j < N_CLASSES; j++) printf("%-18d", conf[i][j]);
        printf("\n");
    }
}

static void print_metrics(int conf[N_CLASSES][N_CLASSES]) {
    printf("\nMetricas por clase:\n");
    printf("  %-18s %-12s %-12s %-12s\n", "Clase", "Precision", "Recall", "F1-score");
    double mp = 0, mr = 0, mf = 0;
    for (int c = 0; c < N_CLASSES; c++) {
        int tp = conf[c][c];
        int fp = 0, fn = 0;
        for (int k = 0; k < N_CLASSES; k++) {
            if (k != c) {
                fp += conf[k][c];
                fn += conf[c][k];
            }
        }
        double prec = (tp + fp > 0) ? (double)tp / (tp + fp) : 0;
        double rec  = (tp + fn > 0) ? (double)tp / (tp + fn) : 0;
        double f1   = (prec + rec > 0) ? 2.0 * prec * rec / (prec + rec) : 0;
        printf("  %-18s %-12.4f %-12.4f %-12.4f\n", class_names[c], prec, rec, f1);
        mp += prec; mr += rec; mf += f1;
    }
    printf("  %-18s %-12.4f %-12.4f %-12.4f\n", "Macro promedio",
           mp/N_CLASSES, mr/N_CLASSES, mf/N_CLASSES);
}

/* --------------------- Particion estratificada --------------------- */

static void stratified_split(const Sample *data, int n, double train_frac,
                             Sample *train, int *ntrain,
                             Sample *test,  int *ntest) {
    int idx[N_CLASSES][MAX_SAMPLES];
    int cnt[N_CLASSES] = {0};
    for (int i = 0; i < n; i++) {
        int c = data[i].class_id;
        idx[c][cnt[c]++] = i;
    }
    *ntrain = *ntest = 0;
    for (int c = 0; c < N_CLASSES; c++) {
        int ntr = (int)(cnt[c] * train_frac);
        for (int i = 0;   i < ntr;     i++) train[(*ntrain)++] = data[idx[c][i]];
        for (int i = ntr; i < cnt[c];  i++) test [(*ntest)++ ] = data[idx[c][i]];
    }
}

/* --------------------- Validacion cruzada estratificada --------------------- */

static double kfold_cv(const Sample *data, int n, int k) {
    int idx[N_CLASSES][MAX_SAMPLES];
    int cnt[N_CLASSES] = {0};
    for (int i = 0; i < n; i++) idx[data[i].class_id][cnt[data[i].class_id]++] = i;

    int fold_of[MAX_SAMPLES];
    for (int c = 0; c < N_CLASSES; c++)
        for (int i = 0; i < cnt[c]; i++)
            fold_of[idx[c][i]] = i % k;

    double accs[32] = {0};
    printf("\n--- Validacion cruzada %d-fold (estratificada) ---\n", k);
    for (int f = 0; f < k; f++) {
        Sample train[MAX_SAMPLES], test[MAX_SAMPLES];
        int ntr = 0, nte = 0;
        for (int i = 0; i < n; i++) {
            if (fold_of[i] == f) test[nte++] = data[i];
            else                  train[ntr++] = data[i];
        }
        NBModel m;
        train_nb(&m, train, ntr);
        int conf[N_CLASSES][N_CLASSES];
        evaluate(&m, test, nte, conf);
        accs[f] = accuracy_from_conf(conf);
        printf("  Fold %2d: train=%3d, test=%3d, accuracy=%.4f\n",
               f + 1, ntr, nte, accs[f]);
    }
    double mean = 0;
    for (int f = 0; f < k; f++) mean += accs[f];
    mean /= k;
    double var = 0;
    for (int f = 0; f < k; f++) var += (accs[f] - mean) * (accs[f] - mean);
    var /= k;
    double std = sqrt(var);
    printf("  Accuracy media: %.4f (desv. tipica: %.4f)\n", mean, std);
    return mean;
}

/* --------------------- Volcado del modelo --------------------- */

static void print_model(const NBModel *m) {
    printf("\n--- Parametros del modelo entrenado ---\n");
    for (int c = 0; c < N_CLASSES; c++) {
        printf("\n%s (prior = %.4f):\n", class_names[c], m->prior[c]);
        for (int j = 0; j < N_FEATURES; j++) {
            printf("  %-15s media=%.4f  varianza=%.4f  std=%.4f\n",
                   feature_names[j], m->mean[c][j], m->var[c][j],
                   sqrt(m->var[c][j]));
        }
    }
}

/* --------------------- Programa principal --------------------- */

int main(int argc, char **argv) {
    const char *csv_path = (argc > 1) ? argv[1] : "raw_penguin_dataset.csv";
    unsigned seed = (argc > 2) ? (unsigned)atoi(argv[2]) : 42u;
    srand(seed);

    Sample data[MAX_SAMPLES];
    int na_skipped = 0;
    int n = load_csv(csv_path, data, &na_skipped);
    if (n <= 0) {
        fprintf(stderr, "No se pudieron cargar datos desde %s\n", csv_path);
        return 1;
    }
    printf("=== Clasificador Bayesiano Naive Gaussiano - Palmer Penguins ===\n");
    printf("Cargados %d ejemplos validos (%d descartados por NA, semilla = %u)\n",
           n, na_skipped, seed);

    int counts[N_CLASSES] = {0};
    for (int i = 0; i < n; i++) counts[data[i].class_id]++;
    printf("\nDistribucion de clases:\n");
    for (int c = 0; c < N_CLASSES; c++)
        printf("  %-18s %d\n", class_names[c], counts[c]);

    /* Barajado reproducible antes de la particion */
    shuffle(data, n);

    /* --- Particion estratificada 70 / 30 --- */
    Sample train[MAX_SAMPLES], test[MAX_SAMPLES];
    int ntrain, ntest;
    stratified_split(data, n, 0.7, train, &ntrain, test, &ntest);
    printf("\nParticion estratificada train/test: %d / %d\n", ntrain, ntest);

    NBModel model;
    train_nb(&model, train, ntrain);
    print_model(&model);

    int conf[N_CLASSES][N_CLASSES];
    evaluate(&model, test, ntest, conf);
    printf("\n--- Evaluacion en el conjunto de test ---\n");
    print_confusion(conf);
    printf("\nAccuracy global: %.4f\n", accuracy_from_conf(conf));
    print_metrics(conf);

    /* --- Validacion cruzada --- */
    kfold_cv(data, n, 5);
    kfold_cv(data, n, 10);

    /* --- Ejemplo individual de prediccion --- */
    printf("\n--- Ejemplo de prediccion (instancia conocida) ---\n");
    /* Pinguino tipo Adelie del dataset original: 39.1, 18.7, 181, 3750 */
    double sample[N_FEATURES] = {39.1, 18.7, 181.0, 3750.0};
    double post[N_CLASSES];
    int pred = predict(&model, sample, post);
    printf("Instancia [%.1f, %.1f, %.1f, %.1f]\n",
           sample[0], sample[1], sample[2], sample[3]);
    printf("Probabilidades posteriores:\n");
    for (int c = 0; c < N_CLASSES; c++)
        printf("  P(%s | x) = %.4f\n", class_names[c], post[c]);
    printf("Prediccion: %s\n", class_names[pred]);

    return 0;
}
