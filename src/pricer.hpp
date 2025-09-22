#ifndef PRICER_HPP
#define PRICER_HPP

#include "csv.hpp"
#include <string>

struct ResultRow {
    OptionRow opt;
    double ttm;
    double iv;
    double model_price;
    double error;
    double delta;
    double gamma;
    double vega;
    double theta;
    double rho;
};

// 用 QuantLib 对单个 OptionRow 定价 + 求隐含波动率
ResultRow price_option(const OptionRow& row);

#endif // PRICER_HPP
