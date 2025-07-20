#include <cassert>
#include <unordered_map>
#include <sys/stat.h>
#include <unistd.h>
#include <Eigen/Dense>
#include <iostream>
#include <sstream>

#include "csv_parser.hpp"
#include "definitions.hpp"
#include "arena.hpp"
#include "johansen_test.hpp"
#include "profiling/profiler.hpp"


enum Confidence {
    p90, p95, p99
};

int pickMaxEigenStatRank(Eigen::MatrixXd& cvm, Eigen::VectorXd& maxEigenStat, Confidence conf) {
    int res = -1;
    for (int r = 0; r < maxEigenStat.size() - 1; ++r) {
        if(maxEigenStat[r] > cvm(r, conf)) {
            continue;
        } else {
            res = r;
            break;
        }
    }

    if (res == -1) return maxEigenStat.size() - 1;
    return res;
}

int pickTraceStatRank(Eigen::MatrixXd& cvt, Eigen::VectorXd& traceStat, Confidence conf) {
    int res = -1;
    for (int r = 0; r < traceStat.size() - 1; ++r) {
        if(traceStat[r] > cvt(r, conf)) {
            continue;
        } else {
            res = r;
            break;
        }
    }

    if (res == -1) return traceStat.size() - 1;
    return res;
}

Eigen::MatrixXd createInputMatrix(std::vector<Column> columns, size_t size) {
    Eigen::MatrixXd data(size, columns.size());
    for (size_t i = 0; i < columns.size(); ++i)  {
        for(size_t j = 0; j < size; ++j) {
            data(j, i) = columns[i].as_f64()[j];
        }
    }
    return data;
}

std::vector<std::string> splitCommaSeparated(const std::string& input) {
    std::vector<std::string> files;
    std::stringstream ss(input);
    std::string token;
     while (std::getline(ss, token, ',')) {
        if (!token.empty()) {
            files.push_back(token);
        }
    }
    return files;
}

int main(int argc, char** argv) {
    Arena arena(constants::ARENA_SIZE);

    std::unordered_map<std::string, ColumnType> dataCol = {
        {"data",  Type_Float64},
    };

    i32 lagsNumber = 1;
    i32 detOrder = 0;

    std::vector<std::string> files;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        const std::string filesPrefix = "--files=";
        const std::string lagsPrefix = "--lags=";
        const std::string detOrderPrefix = "--detOrder=";

        if (arg.rfind(filesPrefix, 0) == 0) {
            std::string files_str = arg.substr(filesPrefix.size());
            files = splitCommaSeparated(files_str);
        }

        if(arg.rfind(lagsPrefix, 0) == 0) {
            std::string numberOfLags = arg.substr(lagsPrefix.size());
            lagsNumber = std::stoi(numberOfLags);
        }

        if(arg.rfind(detOrderPrefix, 0) == 0) {
            std::string detOrderStr = arg.substr(detOrderPrefix.size());
            detOrder = std::stoi(detOrderStr);
        }
    }

    std::vector<Column> rawData(files.size());
    size_t count = 0;

    std::cout << "\n\n---------------------------------\n";
    std::cout << "Input data: \n";
    std::cout << "---------------------------------\n";
    for (size_t i = 0; i < files.size(); ++i) {
        CsvParser file(files[i], arena, dataCol);
        auto fileData = file.parsedContent();
        rawData.at(i) = fileData["data"];
        count = fileData["data"].size;
        std::cout << files[i] << ": " << fileData["data"].size << " prices" << "\n";
    }

    auto data = createInputMatrix(rawData, count);

    std::cout << "Number of lags: " << lagsNumber << "\n";
    std::cout << "Det order: " << detOrder << "\n";

    std::cout << "\n\n---------------------------------\n";
    std::cout << "Profiler results: \n";
    std::cout << "---------------------------------\n";

    startProfiling();

    JohansenTest test(data, lagsNumber, detOrder);
    Eigen::VectorXd eigenvalues          = test.eigenvalues();
    Eigen::VectorXd maxEigenStat         = test.maxEigenStat();
    Eigen::VectorXd traceStat            = test.traceStat();
    Eigen::MatrixXd cointegrationVectors = test.cointegrationVectors();
    Eigen::MatrixXd cvt                  = test.cvt();
    Eigen::MatrixXd cvm                  = test.cvm();

    int traceStatRank = pickTraceStatRank(cvt, traceStat, p99);
    int maxEigenStatRank = pickMaxEigenStatRank(cvm, traceStat, p99);
    int rank = std::min(traceStatRank, maxEigenStatRank);

    endProfilingAndPrint();

    std::cout << "\n\n---------------------------------\n";
    std::cout << "Interpretation: \n";
    std::cout << "---------------------------------\n";

    std::cout << "Trace Stat Rank: " << traceStatRank << "\n";
    std::cout << "Max Eigen Stat Rank: " << maxEigenStatRank << "\n";
    std::cout << "The rank: " << rank << "\n";

    std::cout << "\n\n---------------------------------\n";
    std::cout << "Raw results: \n";
    std::cout << "---------------------------------\n";
    printf("Eigenvalues: \n[");
    for (int i = 0; i < eigenvalues.size(); ++i) {
        printf(" %.15f ", eigenvalues[i]);
    }
    printf("]\n");


    printf("Trace stat: \n[");
    for (int i = 0; i < traceStat.size(); ++i) {
        printf(" %.15f ", traceStat[i]);
    }
    printf("]\n");

    printf("Max eigenvalue stat: \n[");
    for (int i = 0; i < maxEigenStat.size(); ++i) {
        printf(" %.15f ", maxEigenStat[i]);
    }
    printf("]\n\n");

    printf("Cointegration vectors: \n");
    for (int i = 0; i < cointegrationVectors.rows(); ++i) {
        printf("[ ");
        for (int j = 0; j < cointegrationVectors.cols(); ++j) {
            printf(" %f ", cointegrationVectors.row(i)[j]);
        }
        printf(" ]\n");
    }

    std::cout << "\n\n---------------------------------\n";
    std::cout << "Critical Values: \n";
    std::cout << "---------------------------------\n";
    printf("Trace stat critical values: \n");
    for (int i = 0; i < cvt.rows(); ++i) {
        printf("[ ");
        for (int j = 0; j < cvt.cols(); ++j) {
            printf(" %f ", cvt.row(i)[j]);
        }
        printf(" ]\n\n");
    }

    printf("Max eigenvalue stat critical values: \n");
    for (int i = 0; i < cvm.rows(); ++i) {
        printf("[ ");
        for (int j = 0; j < cvm.cols(); ++j) {
            printf(" %f ", cvm.row(i)[j]);
        }
        printf(" ]\n");
    }

    return 0;
}


