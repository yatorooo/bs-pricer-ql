#ifndef CSV_HPP
#define CSV_HPP

#include <string>
#include <vector>

struct OptionRow {
    std::string trade_date;   // e.g., "2025-09-19"
    std::string underlying;   // e.g., "AAPL"
    double spot;               // current price
    double r;                 // rate, e.g., 0.025 (2.5%)
    double q;                 // also rate
    char type;             // 'C' or 'P'
    std::string maturity_date;
    double strike;          // strike price
    double market_price;    // option market price
};

// 读入CSV -> vector<OptionRow>
std::vector<OptionRow> read_option_chain_csv(const std::string& path);

#endif // CSV_HPP
