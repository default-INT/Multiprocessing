#include <iostream>
#include <c++/4.8.3/iomanip>
#include "Matrix.h"
#include "SlauLDLtCalculator.h"

template <typename T>
void print(Matrix<T> *matrix);
int** generateTemplateMatrix(int, int);
void deleteMatrix(int**, int);
Matrix<float>* get_example_1();


int main() {
    int** array = generateTemplateMatrix(3, 3);
    int** add_array = generateTemplateMatrix(3, 1);
    auto *matrix = new Matrix<int>(array, 3, 3);
    auto *add_matrix = new Matrix<int>(add_array, 3, 1);

    float *vector_b = new float [5] {4, 3, 5, 7, 5};

    auto *matrix_A = get_example_1();
    auto *slau_calculator = new SlauLDLtCalculator(matrix_A, vector_b);
    float *answer = slau_calculator->solve();

    std::cout << "A:" << std::endl;
    print(matrix_A);
    std::cout << "LD:" << std::endl;
    print(slau_calculator->get_LD());
    // print(matrix);
    std::cout << "answer:" << std::endl;
    for (int i = 0; i < matrix_A->get_row(); i++) {
        std::cout << answer[i] << std::endl;
    }
    //int size = matrix.getRow();

    // std::cout << "Matrix size = " << size << std::endl;

    // delete result;
    delete [] answer;
    delete slau_calculator;
    delete matrix;
    delete add_matrix;
    return 0;
}

template <typename T>
void print(Matrix<T> *matrix) {
    for (int i = 0; i < (*matrix).get_row(); i++) {
        for (int j = 0; j < (*matrix).get_column(); j++) {
            std::cout << std::setw(6)  << (*matrix).get_element(i, j) << " ";
        }
        std::cout << std::endl;
    }

}

Matrix<float>* get_example_1() {
    int size = 5;
    auto **m3m3 = new float *[size] {
            new float [size] {1, 3, 4, 5, 6},
            new float [size] {3, 1, 2, 4, 7},
            new float [size] {4, 2, 1, 5, 8},
            new float [size] {5, 4, 5, 1, 9},
            new float [size] {6, 7, 8, 9, 1}
    };
    return new Matrix<float>(m3m3, size, size);
}

int** generateTemplateMatrix(int row, int column) {
    int** array = new int *[row];
    int n = 0;
    for (int i = 0; i < row; i++) {
        array[i] = new int[column];
        for (int j = 0; j < column; j++, n++) {
            array[i][j] = n;
        }
    }
    return array;
}

void deleteMatrix(int** matrix, int row) {
    for (int i = 0; i < row; i++) {
        delete[] matrix[i];
    }
}