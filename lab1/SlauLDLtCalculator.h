//
// Created by evgen on 23.09.2020.
//

#ifndef LAB1_SLAULDLTCALCULATOR_H
#define LAB1_SLAULDLTCALCULATOR_H

#include "Matrix.h"


class SlauLDLtCalculator {
private:
    Matrix<float> *matrix_A;
    float *vector_B;
    Matrix<float> *matrix_LD;
    void factorization();
public:
    SlauLDLtCalculator(Matrix<float>*, float*);
    ~SlauLDLtCalculator();
    Matrix<float>* get_LD();
    float* solve();
};


#endif //LAB1_SLAULDLTCALCULATOR_H
