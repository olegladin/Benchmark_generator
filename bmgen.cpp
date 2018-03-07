/** VLSI CAD Programming Assignment: Benchmark generator
  * Ladin Oleg
  */

#include <cassert>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include "Windows.h"

// Parameters
int N = 50;     // grid size
int M_min = 5;  // min number of terminals
int M_max = 30; // max number of terminals
int K = 1;      // number of benchmarks to generate for each M
unsigned int SEED = 0; // seed for random generator
bool PRINT = false;    // print and log additional info
std::string DIR;       // output directory for benchmarks

// Constants
const int TIMES_REPEATED_MAX = 5; // number of attempts to place pin randomly
const std::string BENCH_FILE_PREF = "bench_";
const std::string BENCH_FILE_SUFF = ".xml";
const std::string IMG_FILE_PREF = "img_";
const std::string IMG_FILE_SUFF = ".txt";

// Functions
void read_params(int argc, char *argv[]);
void error(std::string &&msg);
void dump_img(bool *field, std::string &filename);
void dump_bench(bool *field, std::string &filename, int pins);


int main(int argc, char *argv[]) {
    // check and init parameters
    read_params(argc, argv);

    // create field
    int NN = N * N;
    bool *field = new bool[NN];
    for (int i = 0; i < NN; ++i) {
        field[i] = false;
    }

    // main cycle
    for (int m = M_min; m <= M_max; ++m) { // foreach M
        for (int k = 0; k < K; ++k) { // K benchmarks
            // generate grid
            int times_repeated = 0;
            for (int m_curr = 0; m_curr < m; ++m_curr) { // m pins
                int x = (rand() % N);
                int y = (rand() % N);
                // If this cell is already occupied, try to regenarate pin.
                // After TIMES_REPEATED_MAX unsuccessfull attempts place it in the first free cell.
                if (field[x + y * N]) {
                    if (times_repeated < TIMES_REPEATED_MAX - 1) {
                        ++times_repeated;
                        --m_curr;
                        continue;
                    }
                    else {
                        int curr = x + y * N;
                        for (int shift = 1; shift <= NN; ++shift) {
                            if (!field[(curr + shift) % NN]) {
                                field[(curr + shift) % NN] = true;
                                break;
                            }
                        }
                    }
                }
                else {
                    field[x + y * N] = true;
                }
                times_repeated = 0;
            }

            // checker
            int pins_seen = 0;
            for (int i = 0; i < NN; ++i) {
                if (field[i]) {
                    ++pins_seen;
                }
            }
            assert((pins_seen == m) && " !!! BUG !!! Number of pins differs from needed");

            // dump files
            std::string filename = 'n' + std::to_string(N) + "_m" + std::to_string(m) + "_k" + std::to_string(k);
            dump_bench(field, filename, m);
            if (PRINT) {
                dump_img(field, filename);
                std::cout << "Created \"" << DIR << '\\' << BENCH_FILE_PREF << filename << BENCH_FILE_SUFF << "\" and \""
                    << DIR << '\\' << IMG_FILE_PREF << filename << IMG_FILE_SUFF << "\"\n";
            }

            // clear grid
            for (int i = 0; i < NN; ++i) {
                field[i] = false;
            }
        }
    }

    // destroy field
    delete[] field;

    return 0;
}


void read_params(int argc, char *argv[]) {
    // Check number of arguments
    if ((argc < 5) || (argc > 8)) {
        error("Wrong amount of arguments: " + std::to_string(argc - 1));
    }

    // Read required parameters
    N = atoi(argv[1]);
    if (N <= 0) {
        error("Wrong parameter N = \"" + std::string(argv[1]) + '\"');
    }

    M_min = atoi(argv[2]);
    if (M_min <= 0) {
        error("Wrong parameter M_min = \"" + std::string(argv[2]) + '\"');
    }

    M_max = atoi(argv[3]);
    if ((M_max < M_min) || (M_max > N * N)) {
        error("Wrong parameter M_max = \"" + std::string(argv[3]) + '\"');
    }

    K = atoi(argv[4]);
    if (K <= 0) {
        error("Wrong parameter K = \"" + std::string(argv[4]) + '\"');
    }

    // Read optional parameters
    bool set_seed = false, set_print = false, set_dir = false;
    for (int i = 5; i < argc; ++i) {
        switch (argv[i][0]) {
          case 'S': {
            if (argv[i][1] != '=') {
                error("Wrong parameter (probably S): \"" + std::string(argv[i]) + '\"');
            }
            if (set_seed) {
                error("Multiple parameters (S): \"" + std::string(argv[i]) + '\"');
            }
            SEED = (unsigned int)(atoi(argv[i] + 2));
            set_seed = true;
            break;
          }
          case 'P': {
            if (argv[i][1] != '=') {
                error("Wrong parameter (probably P): \"" + std::string(argv[i]) + '\"');
            }
            if (set_print) {
                error("Multiple parameters (P): \"" + std::string(argv[i]) + '\"');
            }
            PRINT = argv[i][2] - '0';
            if ((PRINT != 0) && (PRINT != 1)) {
                error("Wrong parameter P = \"" + std::string(argv[i] + 2) + '\"');
            }
            set_print = true;
            break;
          }
          case 'D': {
            if (argv[i][1] != '=') {
                error("Wrong parameter (probably D): \"" + std::string(argv[i]) + '\"');
            }
            if (set_dir) {
                error("Multiple parameters (D): \"" + std::string(argv[i]) + '\"');
            }
            if (argv[i][strlen(argv[i]) - 1] == '\\') {
                argv[i][strlen(argv[i]) - 1] = '\0'; // remove trailed backslash
            }
            if (argv[i][2] == '\0') {
                error("Wrong parameter D (empty)");
            }
            DIR = (argv[i] + 2);
            set_dir = true;
            break;
            }
          default: {
            error("Wrong parameter: \"" + std::string(argv[i]) + '\"');
            break;
          }
        }
    }

    // Init optional parameters
    if (!set_print) {
        PRINT = false;
    }

    if (!set_seed) {
        SEED = (unsigned int)std::time(nullptr);
    }
    srand(SEED);

    if (!set_dir) {
        DIR = '.';
    }
    else {
        bool dir_created = CreateDirectory(DIR.c_str(), nullptr);
        if (!dir_created && (GetLastError() != ERROR_ALREADY_EXISTS)) {
            error("Couldn't create a directory \"" + DIR + '\"');
        }
    }

    if (PRINT) {
        std::cout << " Running with: N=" << N << " M_min=" << M_min << " M_max="
                  << M_max << " K=" << K << " S=" << SEED << " P=true D=" << DIR << '\n';
    }
}

void error(std::string &&msg) {
    std::cout << " !!! ERROR !!!\n"
              << msg << "\n\n"
              << "Use:  .\\PROG N M_min M_max K [S=...] [P=...] [D=...]\n"
              << "N - grid size (N > 0) [50]\n"
              << "M_min - min number of terminals (M_min > 0) [5]\n"
              << "M_max - max number of terminals ((M_max >= M_min) && (Mmax <= N * N)) [30]\n"
              << "K - number of benchmarks to generate for each M (K > 0) [1]\n"
              << "S - (optional) seed for random generator [S=12345]\n"
              << "P - (optional) print and log additional info (P = 0;1) [P=1]\n"
              << "D - (optional) output directory for benchmarks [D=.\\my_dir]\n\n";
    exit(EXIT_FAILURE);
}

void dump_img(bool *field, std::string &filename) {
    std::ofstream img_file;
    img_file.open(DIR + '\\' + IMG_FILE_PREF + filename + IMG_FILE_SUFF);
    if (img_file.is_open()) {
        for (int y = 0; y < N; ++y) {
            int yN = y * N;
            for (int x = 0; x < N; ++x) {
                img_file << (field[x + yN] ? '#' : '.');
            }
            img_file << '\n';
        }
        img_file.close();
    }
    else {
        error("Couldn't create image file \"" + DIR + '\\' + IMG_FILE_PREF + filename + IMG_FILE_SUFF + '\"');
    }
}

void dump_bench(bool *field, std::string &filename, int pins) {
    std::ofstream bench_file;
    bench_file.open(DIR + '\\' + BENCH_FILE_PREF + filename + BENCH_FILE_SUFF);
    if (bench_file.is_open()) {
        // header: "<net grid_size="6" pin_count="3">
        bench_file << "<net grid_size=\"" << N << "\" pin_count=\"" << pins << "\">\n";
        int pins_seen = 0;
        for (int y = 0; y < N; ++y) {
            int yN = y * N;
            for (int x = 0; x < N; ++x) {
                if (field[x + yN]) {
                    // pin: "    <point x="3" y="4" layer="pins" type="pin" />"
                    bench_file << "    <point x=\"" << x << "\" y=\"" << y << "\" layer=\"pins\" type=\"pin\" />\n";
                    ++pins_seen;
                    if (pins_seen == pins) {
                        break;
                    }
                }
            }
            if (pins_seen == pins) {
                break;
            }
        }
        // footer: "</net>"
        bench_file << "</net>\n";
        bench_file.close();
    }
    else {
        error("Couldn't create benchmark file \"" + DIR + '\\' + BENCH_FILE_PREF + filename + BENCH_FILE_SUFF + '\"');
    }
}
