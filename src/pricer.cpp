// pricer.cpp
#include "pricer.hpp"
#include <ql/quantlib.hpp>
#include <ql/utilities/dataparsers.hpp>  // DateParser::parseISO
#include <cmath>

using namespace QuantLib;

ResultRow price_option(const OptionRow& r) {
    // --- Day count / calendar & evaluation date ---
    Calendar   cal = UnitedKingdom();
    DayCounter dc  = Actual365Fixed();

    Date tradeDate = DateParser::parseISO(r.trade_date);
    Settings::instance().evaluationDate() = tradeDate;

    // --- Spot & curves (use QuantLib::ext smart pointers) ---
    ext::shared_ptr<Quote> spotPtr = ext::make_shared<SimpleQuote>(r.spot);
    Handle<Quote> spot(spotPtr);

    ext::shared_ptr<YieldTermStructure> riskFreePtr =
        ext::make_shared<FlatForward>(tradeDate, r.r, dc);
    Handle<YieldTermStructure> riskFree(riskFreePtr);

    ext::shared_ptr<YieldTermStructure> dividendPtr =
        ext::make_shared<FlatForward>(tradeDate, r.q, dc);
    Handle<YieldTermStructure> dividend(dividendPtr);

    ext::shared_ptr<BlackVolTermStructure> volPtr =
        ext::make_shared<BlackConstantVol>(tradeDate, cal, 0.20, dc);
    Handle<BlackVolTermStructure> volTS(volPtr);

    // --- Process & pricing engine ---
    auto process = ext::make_shared<BlackScholesMertonProcess>(
        spot, dividend, riskFree, volTS);
    auto engine  = ext::make_shared<AnalyticEuropeanEngine>(process);

    // --- Build European option ---
    Option::Type typ = (r.type == 'C' ? Option::Call : Option::Put);
    Date maturity = DateParser::parseISO(r.maturity_date);

    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturity);
    ext::shared_ptr<StrikedTypePayoff> payoff =
        ext::make_shared<PlainVanillaPayoff>(typ, r.strike);

    EuropeanOption opt(payoff, exercise);
    opt.setPricingEngine(engine);

    // --- Time to maturity (year fraction) ---
    Time T = dc.yearFraction(tradeDate, maturity);
    if (T <= 0.0) T = 1.0/365.0;  // guard for same-day etc.

    // --- Implied vol from market price ---
    Real iv = 0.20;
    try {
        iv = opt.impliedVolatility(
            r.market_price, process,
            1e-7,      // accuracy
            100,       // max evaluations
            1e-6, 5.0  // minVol, maxVol
        );
    } catch (...) {
        // keep default 20% if it fails
    }

    // --- Rebuild with solved vol and reprice/Greeks ---
    Handle<BlackVolTermStructure> solvedVol(
        ext::make_shared<BlackConstantVol>(tradeDate, cal, iv, dc));
    process = ext::make_shared<BlackScholesMertonProcess>(
        spot, dividend, riskFree, solvedVol);
    engine  = ext::make_shared<AnalyticEuropeanEngine>(process);
    opt.setPricingEngine(engine);

    // --- Pack results ---
    ResultRow res;
    res.opt         = r;
    res.ttm         = T;
    res.iv          = iv;
    res.model_price = opt.NPV();
    res.error       = res.model_price - r.market_price;
    res.delta       = opt.delta();
    res.gamma       = opt.gamma();
    res.vega        = opt.vega();
    res.theta       = opt.theta();
    res.rho         = opt.rho();
    return res;
}
