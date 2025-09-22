#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
抓取期权链并导出为 C++ pricer 需要的 CSV:
trade_date,underlying,spot,r,q,option_type,maturity_date,strike,market_price
"""

from __future__ import annotations
import os
import sys
import datetime as dt
from dataclasses import dataclass
from typing import Optional, List

import pandas as pd
import numpy as np
import yfinance as yf

# ============= 配置区（直接改这里） ============= #
CONFIG_SYMBOL        = "TSLA"          # 公司代码，如 "AAPL" / "TSLA"
CONFIG_EXPIRATION    = "2025-06-19"            # 到期日: "YYYY-MM-DD"；None=自动选最近一个 >= 今日
CONFIG_SIDE          = "calls"          # "calls" / "puts" / "both"
CONFIG_KMIN          = 180            # 执行价下限: float 或 None
CONFIG_KMAX          = 300            # 执行价上限: float 或 None
CONFIG_LIMIT_PER_SIDE= 40              # 每个方向最多抓多少条；0=不限
CONFIG_OUT_PATH      = "data/option_chain_sample_yfinance.csv"  # 输出 CSV 路径
# ============================================ #

# 可选：支持命令行覆盖（不传就用上面的配置）
import argparse
def parse_args():
    ap = argparse.ArgumentParser(description="Fetch option chain to CSV (yfinance). All args optional; defaults come from config block.")
    ap.add_argument("--symbol")
    ap.add_argument("--expiration", help='YYYY-MM-DD; if omitted, pick nearest >= today')
    ap.add_argument("--side", choices=["calls","puts","both"])
    ap.add_argument("--kmin", type=float)
    ap.add_argument("--kmax", type=float)
    ap.add_argument("--limit", type=int)
    ap.add_argument("--out")
    return ap.parse_args()

@dataclass
class Config:
    symbol: str
    expiration: Optional[str]
    side: str
    kmin: Optional[float]
    kmax: Optional[float]
    limit: int
    out: str

def merge_config(args) -> Config:
    return Config(
        symbol     = (args.symbol     or CONFIG_SYMBOL).upper(),
        expiration =  args.expiration or CONFIG_EXPIRATION,
        side       = (args.side       or CONFIG_SIDE).lower(),
        kmin       =  args.kmin       if args.kmin is not None else CONFIG_KMIN,
        kmax       =  args.kmax       if args.kmax is not None else CONFIG_KMAX,
        limit      =  args.limit      if args.limit is not None else CONFIG_LIMIT_PER_SIDE,
        out        =  args.out        or CONFIG_OUT_PATH
    )

def iso(d: dt.date) -> str:
    return d.strftime("%Y-%m-%d")

def get_risk_free_rate_annual() -> float:
    """近似无风险利率：优先 3M 国债 ^IRX（百分比），失败退化 10Y ^TNX，再失败用 2%."""
    try:
        irx = yf.Ticker("^IRX").history(period="5d")["Close"].dropna()
        if len(irx):
            return float(irx.iloc[-1]) / 100.0
    except Exception:
        pass
    try:
        tnx = yf.Ticker("^TNX").history(period="5d")["Close"].dropna()
        if len(tnx):
            return float(tnx.iloc[-1]) / 100.0
    except Exception:
        pass
    return 0.02

def get_div_yield(tkr: yf.Ticker) -> float:
    """股息率：用 fast_info.dividend_yield；没有就 0."""
    try:
        dy = tkr.fast_info.get("dividend_yield", None)
        if dy is None or (isinstance(dy, float) and np.isnan(dy)):
            return 0.0
        return float(dy)
    except Exception:
        return 0.0

def pick_expiration(tkr: yf.Ticker, exp: Optional[str]) -> str:
    exps = tkr.options or []
    if not exps:
        raise RuntimeError("No option expirations found for this ticker.")
    if exp:
        key = exp.replace("-", "")
        for e in exps:
            if e.replace("-", "") == key:
                return e
        raise RuntimeError(f"Requested expiration {exp} not in available list: {exps[:8]} ...")
    # 默认选最近且>=今天
    today = dt.date.today()
    for e in exps:
        if dt.date.fromisoformat(e) >= today:
            return e
    return exps[-1]

def midpoint(row: pd.Series) -> Optional[float]:
    b, a, last = row.get("bid"), row.get("ask"), row.get("lastPrice")
    if pd.notna(b) and pd.notna(a) and a > 0 and b >= 0:
        return 0.5 * (float(b) + float(a))
    if pd.notna(last) and last > 0:
        return float(last)
    return None

def filter_by_strike(df: pd.DataFrame, kmin: Optional[float], kmax: Optional[float]) -> pd.DataFrame:
    if kmin is not None:
        df = df[df["strike"] >= kmin]
    if kmax is not None:
        df = df[df["strike"] <= kmax]
    return df

def build_rows(symbol: str, trade_date: str, spot: float, r: float, q: float,
               side: str, maturity_date: str, df: pd.DataFrame, limit: int) -> List[List]:
    rows: List[List] = []
    for _, row in df.iterrows():
        mp = midpoint(row)
        if mp is None or mp <= 0:
            continue
        rows.append([
            trade_date, symbol, spot, r, q,
            "C" if side == "calls" else "P",
            maturity_date, float(row["strike"]), float(mp),
        ])
        if limit and len(rows) >= limit:
            break
    return rows

def main():
    args = parse_args()
    cfg = merge_config(args)

    tkr = yf.Ticker(cfg.symbol)
    trade_date = iso(dt.date.today())

    # spot
    try:
        spot = float(tkr.fast_info["last_price"])
    except Exception:
        px = tkr.history(period="1d")["Close"].dropna()
        if not len(px):
            sys.exit("Failed to get spot price.")
        spot = float(px.iloc[-1])

    # r & q
    r = get_risk_free_rate_annual()
    q = get_div_yield(tkr)

    # expiration
    exp = pick_expiration(tkr, cfg.expiration)
    maturity_date = exp

    # option chain
    ch = tkr.option_chain(exp)
    calls = filter_by_strike(ch.calls.copy(), cfg.kmin, cfg.kmax)
    puts  = filter_by_strike(ch.puts.copy(),  cfg.kmin, cfg.kmax)

    # 让采样更集中在现价附近
    calls["dist"] = (calls["strike"] - spot).abs()
    puts["dist"]  = (puts["strike"]  - spot).abs()
    calls = calls.sort_values(["dist", "strike"]).drop(columns=["dist"])
    puts  = puts.sort_values (["dist", "strike"]).drop(columns=["dist"])

    rows: List[List] = []
    if cfg.side in ("calls", "both"):
        rows += build_rows(cfg.symbol, trade_date, spot, r, q, "calls", maturity_date, calls, cfg.limit)
    if cfg.side in ("puts", "both"):
        rows += build_rows(cfg.symbol, trade_date, spot, r, q, "puts",  maturity_date, puts,  cfg.limit)

    if not rows:
        sys.exit("No valid options found under filters.")

    header = ["trade_date","underlying","spot","r","q","option_type","maturity_date","strike","market_price"]
    df_out = pd.DataFrame(rows, columns=header)

    # 类型 & 清洗
    for col in ("spot","r","q","strike","market_price"):
        df_out[col] = pd.to_numeric(df_out[col], errors="coerce")
    df_out = df_out.dropna()

    # 写文件
    os.makedirs(os.path.dirname(cfg.out), exist_ok=True)
    df_out.to_csv(cfg.out, index=False)
    print(f"[OK] Wrote {len(df_out)} rows to {cfg.out}")
    print(df_out.head(min(6, len(df_out))))

if __name__ == "__main__":
    main()
