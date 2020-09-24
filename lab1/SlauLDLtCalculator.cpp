//
// Источник алгоритма разложения LDL' https://ee.kpi.ua/~vk/calcmat/lab/slau_ru.pdf
//                                    https://slemeshevsky.github.io/num-mmf/sles/pdf/sles-4print.pdf
//                                    https://nabbla1.livejournal.com/245822.html
//
// Created by evgen on 23.09.2020.
//

#include "SlauLDLtCalculator.h"

SlauLDLtCalculator::SlauLDLtCalculator(Matrix<float> *matrix_A, float *vector_B) {
    this->matrix_A = matrix_A;
    this->vector_B = vector_B;
}

SlauLDLtCalculator::~SlauLDLtCalculator() {
    delete matrix_A;
    delete matrix_LD;
    delete [] vector_B;
}

void SlauLDLtCalculator::factorization() {
    matrix_LD = new Matrix<float>(matrix_A);
    for (int i = 0; i < matrix_LD->get_column(); i++) {
        // Вычисление диаганального элемента (для вектора D). Формула 9
        for (int j = 0; j < i; j++) {
            matrix_LD->get(i, i) -= matrix_LD->get(i, j) * matrix_LD->get(i, j) * matrix_LD->get(j, j);
        }
        // Вычисление элементов под главной диагональю (для матрицы L). Формула 10
        for (int j = i + 1; j < matrix_LD->get_row(); j++) {
            for (int k = 0; k < i; k++) {
                matrix_LD->get(j, i) -= matrix_LD->get(k, k) * matrix_LD->get(j, k) * matrix_LD->get(i, k);
            }
            matrix_LD->get(j, i) /= matrix_LD->get(i, i);
        }
    }
}

Matrix<float>* SlauLDLtCalculator::get_LD() {
    return matrix_LD;
}

// Решение СЛАУ. Формула 8
float* SlauLDLtCalculator::solve() {
    // Для разложения мматрицы A на LD, сначало требуется выполнить факторизацию
    factorization();
    // Объявления вектора X, Y и Z
    float *answer = new float [matrix_A->get_row()];
    float vector_y[matrix_A->get_row()];
    float vector_z[matrix_A->get_row()];
    // Прямой проход для вычисления L*Y=B и D*Z=Y
    for (int i = 0; i < matrix_LD->get_row(); i++) {
        // Формула 10
        vector_y[i] = vector_B[i];
        for (int j = 0; j < i; j++) {
            vector_y[i] -= matrix_LD->get(i, j) * vector_y[j];
        }
        // Формула 11
        vector_z[i] = vector_y[i] / matrix_LD->get(i, i);
    }
    // обратный проход для вычисления L'*X=Z
    for (int i = matrix_LD->get_row() - 1; i >= 0; i--) {
        // Формула 12
        answer[i] = vector_z[i];
        for (int j = i + 1; j < matrix_LD->get_row(); j++) {
            answer[i] -= matrix_LD->get(j, i) * answer[j];
        }
    }
    return answer;
}