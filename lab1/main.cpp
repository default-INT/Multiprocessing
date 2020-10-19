#include <iostream>
#include <iomanip>
#include <fstream>
#include <processthreadsapi.h>
#include "Matrix.h"
#include <chrono>
#include <Windows.h>
#include <zconf.h>
#include "SlauLDLtCalculator.h"

const std::string DATA_PACKAGE = "../data/";
const std::string RESULT_PACKAGE = "../result/";

struct FileInfo {
    std::string slau_name;
    std::string answer_name;
    std::string result_name;
};

struct ProcessInfo {
    std::string slau_name;
    std::string answer_name;
    int cpu_burst;
};

template <typename T>
void print(Matrix<T> *matrix);
Matrix<float>* get_example_1();
Matrix<float>* read_matrix(std::string fileName);
float* read_vector(std::string fileName);
int save_result(std::string fileName, float *answer, int size);
void manage_threads_FCFS(ProcessInfo *processes_info, int process_count);
DWORD WINAPI slau_ldlt_task(LPVOID arg);
void manage_displacing_guaranteed_planning(ProcessInfo *processes_info, int process_count);

int main(int argc, char* argv[]) {
    ProcessInfo processInfo[5] = {
            ProcessInfo{"A", "va", 2},
            ProcessInfo{"B", "vb", 2},
            ProcessInfo{"C", "vc", 2},
            ProcessInfo{"D", "vd", 4},
            ProcessInfo{"E", "ve", 1}
    };
    manage_displacing_guaranteed_planning(processInfo, 5);
    manage_threads_FCFS(processInfo, 5);
//    FileInfo *fileInfo = new FileInfo{"A", "ba", "P1_FCFS"};
//    HANDLE thread;
//    thread = CreateThread(nullptr, 0, slau_ldlt_task, (LPVOID) fileInfo, 0, nullptr);
//    WaitForSingleObject(thread, INFINITE);
    system("pause");
    return 0;
}

void manage_displacing_guaranteed_planning(ProcessInfo *processes_info, int process_count) {
    int time_slice_ms = 1;
    HANDLE *threads = new HANDLE[process_count];
    for (int i = 0; i < process_count; i++) {
        std::string process_name = "P" + std::to_string(i) + "_DGP";
        threads[i] = CreateThread(
                nullptr,
                0,
                slau_ldlt_task,
                (LPVOID) new FileInfo{processes_info[i].slau_name, processes_info[i].answer_name, process_name},
                CREATE_SUSPENDED,
                nullptr
        );
    }
    // Засекаем время
    std::chrono::time_point<std::chrono::high_resolution_clock> time_start = std::chrono::high_resolution_clock::now();

    int active_thread = 1;

    int *thread_lifetime = new int[process_count];
    for (int i = 0; i < process_count; i++) thread_lifetime[i] = 0;

    int runned_count, executed_count = 0;
    do {
        int current_thread = 0;
        float min_cpu_burst = processes_info[0].cpu_burst * thread_lifetime[0] * 1.0f / (process_count - executed_count);
        for (int i = 0; i < process_count; i++) {
            float koeff_justice = processes_info[i].cpu_burst * thread_lifetime[i] * 1.0f / (process_count - executed_count);
            if (min_cpu_burst > koeff_justice && processes_info[i].cpu_burst > 0 || min_cpu_burst < 0) {
                min_cpu_burst = koeff_justice;
                current_thread = i;
            }
        }
        ResumeThread(threads[current_thread]);

        WaitForSingleObject(threads[current_thread], time_slice_ms);

        thread_lifetime[current_thread] += time_slice_ms;
        processes_info[current_thread].cpu_burst -= time_slice_ms;

        SuspendThread(threads[current_thread]);

        if (processes_info[current_thread].cpu_burst <= 0) {
            ResumeThread(threads[current_thread]);
            WaitForSingleObject(threads[current_thread], INFINITE);
            processes_info[current_thread].cpu_burst = -1;
            executed_count++;
        }
    } while (executed_count < process_count);

    // Смотрим сколько потрачено
    std::chrono::time_point<std::chrono::high_resolution_clock> time_end = std::chrono::high_resolution_clock::now();
    // Смотрим затраты времени
    std::chrono::nanoseconds delta_time = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start);
    std::cout << "Total spent time: " << delta_time.count() * 0.001f * 0.001f << " ms" << std::endl;

    delete [] thread_lifetime;
    delete [] threads;
}

void manage_threads_FCFS(ProcessInfo *processes_info, int process_count) {
    HANDLE *threads = new HANDLE[process_count];
    for (int i = 0; i < process_count; i++) {
        std::string process_name = "P" + std::to_string(i) + "_FCFS";
        threads[i] = CreateThread(
            nullptr,
            0,
            slau_ldlt_task,
            (LPVOID) new FileInfo{processes_info[i].slau_name, processes_info[i].answer_name, process_name},
            CREATE_SUSPENDED,
            nullptr
        );
    }
    // Засекаем время
    std::chrono::time_point<std::chrono::high_resolution_clock> time_start = std::chrono::high_resolution_clock::now();

    int active_thread = 1;
    HANDLE runned_threads[active_thread];
    int runned_count, executed_count = 0;
    do {
        runned_count = 0;
        do {
            runned_threads[runned_count] = threads[executed_count + runned_count];
            ResumeThread(runned_threads[runned_count]);
            runned_count++;
        } while (runned_count < active_thread && executed_count + runned_count < process_count);

        WaitForMultipleObjects(runned_count, runned_threads, true , INFINITE);

        executed_count += runned_count;
    } while (executed_count < process_count);

    // Смотрим сколько потрачено
    std::chrono::time_point<std::chrono::high_resolution_clock> time_end = std::chrono::high_resolution_clock::now();
    // Смотрим затраты времени
    std::chrono::nanoseconds delta_time = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start);
    std::cout << "Total spent time: " << delta_time.count() * 0.001f * 0.001f << " ms" << std::endl;

    delete [] threads;
}

DWORD WINAPI slau_ldlt_task(LPVOID arg) {
    // Засекаем время
    std::chrono::time_point<std::chrono::high_resolution_clock> time_start = std::chrono::high_resolution_clock::now();

    FileInfo *fileInfo = (FileInfo*)arg;

    std::string slauFileName = DATA_PACKAGE + fileInfo->slau_name + ".txt";
    std::string answerFileName = DATA_PACKAGE + fileInfo->answer_name + ".txt";
    std::string resultFileName = RESULT_PACKAGE + fileInfo->result_name + ".txt";

    auto *matrix_A = read_matrix(slauFileName);
//    print(matrix_A);

    auto *vector_b = read_vector(answerFileName);

    //float *vector_b = new float [5] {4, 3, 5, 7, 5};

    // auto *matrix_A = get_example_1();
    auto *slau_calculator = new SlauLDLtCalculator(matrix_A, vector_b);
    float *answer = slau_calculator->solve();
    sleep(1);

//    std::cout << "A:" << std::endl;
//    print(matrix_A);
//    std::cout << "LD:" << std::endl;
//    print(slau_calculator->get_LD());

//    std::cout << "result:" << std::endl;
//    for (int i = 0; i < matrix_A->get_row(); i++) {
//        std::cout << answer[i] << std::endl;
//    }


    save_result(resultFileName, answer, matrix_A->get_row());
    std::ofstream of(resultFileName);
    // Смотрим сколько потрачено
    std::chrono::time_point<std::chrono::high_resolution_clock> time_end = std::chrono::high_resolution_clock::now();
    // Смотрим затраты времени
    std::chrono::nanoseconds delta_time = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start);
    float delta_time_ms = delta_time.count() * 0.001f * 0.001f;
    std::cout << fileInfo->result_name << " time: " << delta_time_ms  << " ms" << std::endl;
    std::string msg = fileInfo->result_name + ", execute time: " + std::to_string(delta_time_ms);

    of << msg << std::endl;
    of.close();

    delete [] answer;
    delete slau_calculator;
    delete fileInfo;
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

int save_result(std::string fileName, float *answer, int size) {
    std::ofstream of(fileName);
    for (int i = 0; i < size; i++) {
        of << answer[i] << " ";
    }
    of << std::endl;
    of.close();
    return 0;
}

float* read_vector(std::string fileName) {
    //Создаем файловый поток и связываем его с файлом
    std::ifstream in(fileName);

    if (in.is_open())
    {
        //Если открытие файла прошло успешно
        //Вначале посчитаем сколько чисел в файле
        int count = 0;// число чисел в файле
        int temp;//Временная переменная

        while (!in.eof())// пробегаем пока не встретим конец файла eof
        {
            in >> temp;//в пустоту считываем из файла числа
            count++;// увеличиваем счетчик числа чисел
        }

        //Опять переходим в потоке в начало файла
        in.seekg(0, std::ios::beg);
        in.clear();


        float *x = new float [count];

        //Считаем матрицу из файла
        for (int i = 0; i < count; i++)
            in >> x[i];

        in.close();//под конец закроем файла

        return x;
    } else {
        //Если открытие файла прошло не успешно
        std::cout << "File not found.\n";
    }
    return 0;
}

Matrix<float>* read_matrix(std::string fileName) {
    //Создаем файловый поток и связываем его с файлом
    std::ifstream in(fileName);

    if (in.is_open())
    {
        //Если открытие файла прошло успешно

        //Вначале посчитаем сколько чисел в файле
        int count = 0;// число чисел в файле
        int temp;//Временная переменная

        while (!in.eof())// пробегаем пока не встретим конец файла eof
        {
            in >> temp;//в пустоту считываем из файла числа
            count++;// увеличиваем счетчик числа чисел
        }

        //Число чисел посчитано, теперь нам нужно понять сколько
        //чисел в одной строке
        //Для этого посчитаем число пробелов до знака перевода на новую строку

        //Вначале переведем каретку в потоке в начало файла
        in.seekg(0, std::ios::beg);
        in.clear();

        //Число пробелов в первой строчке вначале равно 0
        int count_space = 0;
        char symbol;
        while (!in.eof())//на всякий случай цикл ограничиваем концом файла
        {
            //теперь нам нужно считывать не числа, а посимвольно считывать данные
            in.get(symbol);//считали текущий символ
            if (symbol == ' ') count_space++;//Если это пробел, то число пробелов увеличиваем
            if (symbol == '\n') break;//Если дошли до конца строки, то выходим из цикла
        }
        //cout << count_space << endl;

        //Опять переходим в потоке в начало файла
        in.seekg(0, std::ios::beg);
        in.clear();

        //Теперь мы знаем сколько чисел в файле и сколько пробелов в первой строке.
        //Теперь можем считать матрицу.

        int n = count / (count_space + 1);//число строк
        int m = count_space + 1;//число столбцов на единицу больше числа пробелов
        float **x;
        x = new float *[n];
        for (int i = 0; i<n; i++) x[i] = new float [m];

        //Считаем матрицу из файла
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                in >> x[i][j];

        in.close();//под конец закроем файла
        return new Matrix<float>(x, n, m);

    } else {
        //Если открытие файла прошло не успешно
        std::cout << "File not found.\n";
    }
    return 0;
}

Matrix<float>* get_example_1() {
    int const size = 5;
    auto **m3m3 = new float *[size] {
            new float [size] {1, 3, 4, 5, 6},
            new float [size] {3, 1, 2, 4, 7},
            new float [size] {4, 2, 1, 5, 8},
            new float [size] {5, 4, 5, 1, 9},
            new float [size] {6, 7, 8, 9, 1}
    };
    return new Matrix<float>(m3m3, size, size);
}
