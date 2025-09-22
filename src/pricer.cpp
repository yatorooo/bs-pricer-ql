#include "pricer.hpp"
#include <ql/quantlib.hpp>
#include <sstream>
#include <cmath>

using namespace QuantLib;

static Date parse_date(const std::string& s) {
    int y=0,m=0,d=0; char dash;
    std::stringstream ss(s);
    ss >> y >> dash >> m >> dash >> d;
    return Date(d, (Month)m, y);
}

ResultRow price_option(const OptionRow& r) {
    Calendar cal = UnitedKingdom();
    DayCounter dc = Actual365Fixed();

    Date tradeDate = parse_date(r.trade_date);
    Settings::instance().evaluationDate() = tradeDate;

    Handle<Quote> spot(boost::shared_ptr<Quote>(new SimpleQuote(r.spot)));
    Handle<YieldTermStructure> riskFree(boost::shared_ptr<YieldTermStructure>(
        new FlatForward(tradeDate, r.r, dc)));
    Handle<YieldTermStructure> dividend(boost::shared_ptr<YieldTermStructure>(
        new FlatForward(tradeDate, r.q, dc)));
    Handle<BlackVolTermStructure> volTS(boost::shared_ptr<BlackVolTermStructure>(
        new BlackConstantVol(tradeDate, cal, 0.20, dc)));

    boost::shared_ptr<BlackScholesMertonProcess> process(
        new BlackScholesMertonProcess(spot, dividend, riskFree, volTS));
    boost::shared_ptr<PricingEngine> engine(new AnalyticEuropeanEngine(process));

    Option::Type typ = (r.type == 'C' ? Option::Call : Option::Put);
    Date maturity = parse_date(r.maturity_date);
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(maturity));
    boost::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(typ, r.strike));
    EuropeanOption opt(payoff, exercise);
    opt.setPricingEngine(engine);

    Time T = dc.yearFraction(tradeDate, maturity);
    if (T <= 0.0) T = 1.0/365.0;

    Real iv = 0.20;
    try {
        iv = opt.impliedVolatility(r.market_price, process, 1e-7, 100, 1e-6, 5.0);
    } catch (...) {
        // fallback: keep 20%
    }

    Handle<BlackVolTermStructure> solvedVol(boost::shared_ptr<BlackVolTermStructure>(
        new BlackConstantVol(tradeDate, cal, iv, dc)));
    process = boost::shared_ptr<BlackScholesMertonProcess>(
        new BlackScholesMertonProcess(spot, dividend, riskFree, solvedVol));
    engine.reset(new AnalyticEuropeanEngine(process));
    opt.setPricingEngine(engine);

    ResultRow res;
    res.opt = r;
    res.ttm = T;
    res.iv = iv;
    res.model_price = opt.NPV();
    res.error = res.model_price - r.market_price;
    res.delta = opt.delta();
    res.gamma = opt.gamma();
    res.vega  = opt.vega();
    res.theta = opt.theta();
    res.rho   = opt.rho();

    return res;
}
