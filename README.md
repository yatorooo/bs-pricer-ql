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
│ └─ priced_sample.csv # sample output
├─ fetch_chain.py # fetch option chain using yfinance
├─ src/
│ ├─ csv.hpp
│ ├─ csv.cpp # data read
│ ├─ pricer.hpp
│ ├─ pricer.cpp # pricing implementation
│ └─ main.cpp # CLI entry point
├─ CMakeLists.txt
└─ README.md

## Execute Example

```bash
./bs_pricer_ql --chain ../data/option_chain_sample_yfinance.csv --out results.csv
