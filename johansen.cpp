#include "johansen_test.hpp"

#include <Eigen/Dense>
#include <Eigen/src/Core/IO.h>
#include <cmath>

Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
JohansenTest::JohansenTest(
    Eigen::MatrixXd& data,
    int32_t p, int8_t detOrder
) : data_(data), p_(p+1), detOrder_(detOrder)
{
    buildRegressionMatrices();
    regress();
    buildCovarianceMatrices();
    solveGenerEigenvalProb();
    return;
}

Eigen::MatrixXd JohansenTest::cvt() {
    if(!lambda_.size()) throw std::runtime_error("In order to get the critical values you have to run the test first!");
    Eigen::MatrixXd cvtMatrix(lambda_.size(), 3);

    auto& criticalVals = [&]() -> const std::array<std::array<double, 3>, 12>& {
        switch(detOrder_) {
            case -1: return cvtP0_;
            case 0: return cvtP1_;
            case 1: return cvtP2_;
            default: throw std::runtime_error("A wrong deterministic trend assumption was selected!");
        }
    }();

    for (int i = lambda_.size() - 1; i >= 0; --i) {
        for (int j = 0; j < 3; ++j) {
            cvtMatrix(lambda_.size() - i - 1, j) = criticalVals[i][j];
        }
    }

    return cvtMatrix;
}

Eigen::MatrixXd JohansenTest::cvm() {
    if(!lambda_.size()) throw std::runtime_error("In order to get the critical values you have to run the test first!");
    Eigen::MatrixXd cvmMatrix(lambda_.size(), 3);

    auto& criticalVals = [&]() -> const std::array<std::array<double, 3>, 12>& {
        switch(detOrder_) {
            case -1: return cvmP0_;
            case 0: return cvmP1_;
            case 1: return cvmP2_;
            default: throw std::runtime_error("A wrong deterministic trend assumption was selected!");
        }
    }();

    for (int i = lambda_.size() - 1; i >= 0; --i) {
        for (int j = 0; j < 3; ++j) {
            cvmMatrix(lambda_.size() - i - 1, j) = criticalVals[i][j];
        }
    }

    return cvmMatrix;
}

void JohansenTest::solveGenerEigenvalProb() {
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(S_00_, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::MatrixXd S_00_pinv = svd.solve(Eigen::MatrixXd::Identity(S_00_.rows(), S_00_.cols()));
    Eigen::MatrixXd A = S_10_ * (S_00_pinv * S_01_);

    Eigen::GeneralizedSelfAdjointEigenSolver<Eigen::MatrixXd> ges(A, S_11_);
    Eigen::VectorXd eigenvalues = ges.eigenvalues();
    Eigen::MatrixXd eigenvectors = ges.eigenvectors();

    lambda_ = eigenvalues.reverse();
    cointegrationVecs_ = eigenvectors;
}

Eigen::VectorXd JohansenTest::maxEigenStat() {
    Eigen::VectorXd maxEigenStat;
    maxEigenStat.resize(lambda_.size());

    for (int i = 0; i < lambda_.size(); ++i) {
        maxEigenStat[i] = -(data_.rows() - p_) * std::log(1 - lambda_[i]);
    }

    return maxEigenStat;
}

Eigen::VectorXd JohansenTest::traceStat() {
    Eigen::VectorXd traceStat;
    traceStat.resize(lambda_.size());

    Eigen::VectorXd logTerms = (1.0 - lambda_.array()).log();

    double totalLogSum = logTerms.sum();

    double cumulative = 0.0;
    for (int j = 0; j < lambda_.size(); ++j) {
        if(j > 0) cumulative += logTerms[j - 1];
        traceStat[j] = -(data_.rows() - p_) * (totalLogSum - cumulative);
    }

    return  traceStat;
}

void JohansenTest::buildCovarianceMatrices() {
    double normalization = 1./(data_.rows() - p_);
    S_00_ = normalization * R_ * R_.transpose();
    S_11_ = normalization * S_ * S_.transpose();
    S_01_ = normalization * R_ * S_.transpose();
    S_10_ = S_01_.transpose();
}

Eigen::VectorXd JohansenTest::eigenvalues() {
    return lambda_;
}

void JohansenTest::regress() {
    Eigen::MatrixXd ZZt = Z_ * Z_.transpose();

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(ZZt, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::MatrixXd ZZt_pinv = svd.solve(Eigen::MatrixXd::Identity(ZZt.rows(), ZZt.cols()));

    Eigen::MatrixXd beta_gamma = ZZt_pinv * Z_ * deltaX_.transpose();
    Eigen::MatrixXd beta_phi = ZZt_pinv * Z_ * laggedX_.transpose();

    R_ = deltaX_  - beta_gamma.transpose() * Z_;
    S_ = laggedX_ - beta_phi  .transpose() * Z_;
}

void JohansenTest::buildRegressionMatrices() {
    int n = data_.cols();
    int T = data_.rows();

    if (detOrder_ == 0) data_.rowwise() -= data_.colwise().mean();
    if (detOrder_ == 1) {
        Eigen::VectorXd t = Eigen::VectorXd::LinSpaced(T, 1, T);
        for(int col = 0; col < n; ++col) {
            Eigen::VectorXd y = data_.col(col);
            double beta = (t.dot(y) - t.sum() * y.sum() / T) /
                          (t.dot(t) - t.sum() * t.sum() / T);
            double alpha = y.mean() - beta * t.mean();
            data_.col(col) = y - alpha * Eigen::VectorXd::Ones(T) - beta * t;
        }
    }

    Eigen::MatrixXd dx = data_.bottomRows(T - 1) - data_.topRows(T - 1);
    deltaX_  = dx.middleRows(p_ - 1, T - p_).transpose();
    laggedX_ = data_.middleRows(p_ - 1, T - p_).transpose();

    Z_.resize((p_ - 1) * n, T - p_);
    for(int k = 1; k < p_; ++k) {
        Z_.block((k - 1) * n, 0, n, T - p_) = dx.middleRows(p_ - 1 - k, T - p_).transpose();
    }

    if(detOrder_ >= 1) {
        Z_.conservativeResize(Z_.rows() + 1, Eigen::NoChange);
        Z_.row(Z_.rows() - 1) = Eigen::RowVectorXd::Ones(T - p_);
    }
}
