#include "csv.hpp"
#include "pricer.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cmath>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: ./bs_pricer_ql --chain ../data/option_chain_sample.csv --out results.csv\n";
        return 1;
    }
    std::string chainPath, outPath;
    for (int i=1; i<argc; i++) {
        std::string arg = argv[i];
        if (arg == "--chain" && i+1<argc) chainPath = argv[++i];
        else if (arg == "--out" && i+1<argc) outPath = argv[++i];
    }

    std::vector<OptionRow> rows = read_option_chain_csv(chainPath);
    std::ofstream out(outPath.c_str());
    out << "trade_date,underlying,spot,r,q,option_type,maturity_date,strike,"
           "market_price,ttm,iv,model_price,error,delta,gamma,vega,theta,rho\n";
    out.setf(std::ios::fixed); out<<std::setprecision(8);

    std::vector<double> ytrue, ypred;
    for (auto& r : rows) {
        ResultRow res = price_option(r);
        out << res.opt.trade_date << "," << res.opt.underlying << ","
            << res.opt.spot << "," << res.opt.r << "," << res.opt.q << ","
            << res.opt.type << "," << res.opt.maturity_date << ","
            << res.opt.strike << "," << res.opt.market_price << ","
            << res.ttm << "," << res.iv << "," << res.model_price << ","
            << res.error << "," << res.delta << "," << res.gamma << ","
            << res.vega << "," << res.theta << "," << res.rho << "\n";
        ytrue.push_back(res.opt.market_price);
        ypred.push_back(res.model_price);
    }

    double se=0, ae=0, ape=0; size_t n=ytrue.size();
    for(size_t i=0;i<n;i++){
        double e=ypred[i]-ytrue[i];
        ae+=fabs(e); se+=e*e;
        if (fabs(ytrue[i])>1e-12) ape+=fabs(e)/fabs(ytrue[i]);
    }
    std::cout<<"MAE="<<(n?ae/n:0)<<"\n";
    std::cout<<"RMSE="<<(n?sqrt(se/n):0)<<"\n";
    std::cout<<"MAPE="<<(n?100.0*ape/n:0)<<"%\n";
    return 0;
}
