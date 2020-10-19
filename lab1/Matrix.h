//
// Created by evgen on 22.09.2020.
//

#ifndef LAB1_MATRIX_H
#define LAB1_MATRIX_H

#include <c++/4.8.3/type_traits>

int add_operation(int, int);
int sub_operation(int, int);
int multi_operation(int, int);
int div_operation(int, int);

template <typename T, class = std::enable_if
        <
        std::is_same<T, int>::value ||
        std::is_same<T, double>::value ||
        std::is_same<T, float>::value
        >>
class Matrix {
private:
    T** matrix;
    int row{};
    int column{};

public:
    static Matrix<T>* matrix_get_operation(Matrix<T> *matrix1, Matrix<T> *matrix2, int (*operation)(int, int)) {
        if ((*matrix1).get_row() < (*matrix2).get_row() || (*matrix1).get_column() < (*matrix2).get_column()) {
            throw "The dimensions of the matrices are incorrect";
        }
        auto *new_matrix = new Matrix<T>((*matrix1).get_row(), (*matrix1).get_column());
        for (int i = 0; i < (*matrix1).row; i++) {
            for (int j = 0; j < (*matrix1).column; j++) {
                (*new_matrix).matrix[i][j] = operation((*matrix1).matrix[i][j], (*matrix2).matrix[i][j]);
            }
        }
        return new_matrix;
    }
    Matrix(T **matrix, int row, int column) {
        this->matrix = matrix;
        this->row = row;
        this->column = column;
    }
    Matrix(Matrix<T> *matrix_copy) {
        row = (*matrix_copy).row;
        column = (*matrix_copy).column;
        matrix = new T *[row];
        for (int i = 0; i < row; i++) {
            matrix[i] = new T[column];
            for (int j = 0; j < column; j++) {
                matrix[i][j] = (T) matrix_copy->get(i, j);
            }
        }
    }
    Matrix(int row, int column) {
        this->row = row;
        this->column = column;
        T** _matrix = new T *[row];
        for (int i = 0; i < row; i++) {
            _matrix[i] = new T[column]();
        }
        this->matrix = _matrix;

    }
    ~Matrix() {
        for (int i = 0; i < row; i++) {
            delete[] matrix[i];
        }
        matrix = nullptr;
        column = 0;
        row = 0;
    }
    T& get(int i, int j) {
        return matrix[i][j];
    }
    int get_size() {
        return row * column;
    }
    int get_column() {
        return column;
    }
    int get_row() {
        return row;
    }
    T get_element(int i, int j) {
        return matrix[i][j];
    }
    void set_element(int i, int j, T element) {
        matrix[i][j] = element;
    }
    Matrix<T>* multiply(Matrix<T> *multi_matrix) {
        if (get_column() != (*multi_matrix).get_row()) {
            throw "The dimensions of the matrices are incorrect";
        }
        auto *new_matrix = new Matrix<T>(get_row(), (*multi_matrix).get_column());
        for (int i = 0; i < (*new_matrix).row; i++) {
            for (int j = 0; j < (*new_matrix).column; j++) {
                for (int k = 0; k < get_column(); k++) {
                    (*new_matrix).matrix[i][j] += matrix[i][k] * (*multi_matrix).matrix[k][j];
                }
            }
        }
        return new_matrix;
    }
    Matrix<T>* multiply_all_elements(Matrix<T> *multi_matrix) {
        return matrix_get_operation(this, multi_matrix, multi_operation);
    }
    Matrix<T>* add(Matrix<T> *add_matrix) {
        return matrix_get_operation(this, add_matrix, add_operation);
    }
    Matrix<T>* sub(Matrix<T> *sub_matrix) {
        return matrix_get_operation(this, sub_matrix, sub_operation);
    }
    // Matrix<T>* div(Matrix<T>*);
    Matrix<T>* div_all_elements(Matrix<T> *div_matrix) {
        return matrix_get_operation(this, div_matrix, div_operation);
    }
    Matrix<T>* transp() {
        Matrix<T> *transp_matrix = new Matrix<T>(column, row);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < column; j++) {
                (*transp_matrix).get(j, i) = matrix[i][j];
            }
        }
        return transp_matrix;
    }
};



#endif //LAB1_MATRIX_H
