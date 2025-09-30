# Black-Scholes Option Pricer (C++ QuantLib + Python yfinance)

An end-to-end mini project:  
- Use **Python/yfinance** to fetch real market option chain data and export to CSV  
- Use **C++/QuantLib** to price options under the **Black-Scholes model**  
- Export results to CSV for further analysis or visualization  

> This project is intended as a reproducible demo for portfolio showcase: data acquisition → pricing model → result export.

---

## Features
- Black-Scholes European option pricing via **QuantLib**  
- Supports Call / Put options  
- Batch pricing of full option chains  
- CSV input/output for easy downstream analysis  
- Optional Greeks calculation (Delta, Gamma, Vega, Theta, Rho)

---

## Project Structure
```text
.
├─ data/
│ ├─ option_chain_sample.csv # sample input
│ └─ result.csv # sample output
├─ fetch_option_chain.py # fetch option chain using yfinance
├─ src/
│ ├─ csv.hpp
│ ├─ csv.cpp # data read
│ ├─ pricer.hpp
│ ├─ pricer.cpp # pricing implementation
│ └─ main.cpp # CLI entry point
├─ CMakeLists.txt
└─ README.md
```
---
## CSV Schema

**Input CSV** (`--chain`):
```text
trade_date,underlying,spot,r,q,option_type,maturity_date,strike,vol
```
- `trade_date`: ISO date (e.g. 2025-09-29)
- `underlying`: ticker symbol (e.g. AAPL)
- `spot`: spot price of underlying
- `r`: risk-free rate (annualized, continuously compounded)
- `q`: dividend yield (annualized, continuously compounded)
- `option_type`: `C` (call) or `P` (put)
- `maturity_date`: ISO date for option maturity
- `strike`: strike price
- `vol`: volatility (optional; if missing, use default or estimated value)

**Output CSV** (`--out`):
```text
trade_date,underlying,spot,r,q,option_type,maturity_date,strike,vol,price,delta,gamma,vega,theta,rho
```

---

## Execute Example

```bash
./bs_pricer_ql --chain ../data/option_chain_sample_yfinance.csv --out results.csv
```
---
## Acknowledgments
- [QuantLib](https://www.quantlib.org/)  
- [yfinance](https://github.com/ranaroussi/yfinance)
