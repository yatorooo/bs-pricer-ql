#include "csv.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

static inline std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    auto a = s.find_first_not_of(ws);
    auto b = s.find_last_not_of(ws);
    if (a == std::string::npos) return "";
    return s.substr(a, b - a + 1);
}

std::vector<OptionRow> read_option_chain_csv(const std::string& path) {
    std::ifstream in(path.c_str());
    if (!in) throw std::runtime_error("Failed to open " + path);

    std::string line;
    if (!std::getline(in, line)) throw std::runtime_error("Empty file");

    std::vector<OptionRow> rows;
    while (std::getline(in, line)) {
        if (trim(line).empty()) continue;
        std::stringstream ss(line);
        std::string c;
        OptionRow r;
        int idx = 0;
        while (std::getline(ss, c, ',')) {
            c = trim(c);
            switch (idx) {
                case 0: r.trade_date = c; break;
                case 1: r.underlying = c; break;
                case 2: r.spot = std::atof(c.c_str()); break;
                case 3: r.r = std::atof(c.c_str()); break;
                case 4: r.q = std::atof(c.c_str()); break;
                case 5: r.type = c.empty() ? 'C' : c[0]; break;
                case 6: r.maturity_date = c; break;
                case 7: r.strike = std::atof(c.c_str()); break;
                case 8: r.market_price = std::atof(c.c_str()); break;
            }
            ++idx;
        }
        if (idx >= 9) rows.push_back(r);
    }
    return rows;
}
