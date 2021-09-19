#!/usr/bin/env python

import sys
import os
import pandas as pd
import argparse 
import json
import datetime
from logger import logger
import requests
import pyarrow as pa
import pyarrow.parquet as pq
import pyarrow.csv as csv
import pyarrow.feather as feather

import tushare as ts
pro = ts.pro_api(
    token='951e1bbba0020a264921d7202a3d4dba4b014479f50cca5286d33f45')


'''
CREATE TABLE IF NOT EXISTS t_daily_kline(
trade_date INTEGER,    
code VARCHAR(24),
open DOUBLE,
high DOUBLE,
low DOUBLE,
close DOUBLE,
pre_close DOUBLE,
change DOUBLE,
pct_chg DOUBLE,
vol DOUBLE,
amount DOUBLE,
PRIMARY KEY(trade_date, code)
)
'''




def date_int_to_datetime(dt):
    if isinstance(dt,datetime.datetime):
        return dt
    if dt is None:
        return datetime.datetime.now()
    if dt == 0:
        return datetime.datetime.now()
    #if isinstance(dt, str):
    if isinstance(dt, str):
        if '-' in dt:
            return datetime.datetime.strptime(dt, "%Y-%m-%d")
        else:
            return datetime.datetime.strptime(dt, "%Y%m%d")
        #dt = int(dt)
    y = int(dt / 10000)
    m = int(dt / 100 - y * 100)
    return datetime.datetime(y, m, int(dt - y * 10000 - m * 100))


def datetime_to_date_int(dt):
    if isinstance(dt, datetime.datetime):
        dt = dt.date()
        return dt.year * 10000 + dt.month * 100 + dt.day
    elif isinstance(dt, datetime.date):
        return dt.year * 10000 + dt.month * 100 + dt.day
    elif isinstance(dt, int):
        return dt
    raise RuntimeError('unsupported parameter type:', type(dt))


def next_work_date_int(base:int, pos=1, skip_weekend=True):
    now = date_int_to_datetime(base) + datetime.timedelta(days=pos)
    if now.date().isoweekday() not in range(1, 6):
        if pos < 0:
            return next_work_date_int(pos=-1, base=now)
        else:
            return next_work_date_int(pos=1, base=now)
    return now.year * 10000 + now.month * 100 + now.day


def download_by_range(code:str,start_dt:int, end_dt:int,fileprefix:str='daily-', format='parquet'):
    rsp = None
    try:
        rsp = pro.query('daily', ts_code=code, start_date=str(start_dt), end_date=str(end_dt))
        if not rsp.empty:
            rsp.rename(columns={'ts_code':'code'}, inplace=True)
        else:
            logger.info(f'download {code} daily empty')
            return
        if 'trade_date' in rsp.columns:
            rsp['trade_date'] = rsp['trade_date'].apply(lambda x: date_int_to_datetime(x).date())
        rsp.sort_values(by='trade_date', inplace=True)
        table = pa.Table.from_pandas(rsp)
        print(table)
            
        if format == 'parquet':
            pq.write_table(table, fileprefix + code + '.' + format)
        elif format == 'feather':
            feather.write_feather(rsp, fileprefix + code + '.' + format)
        elif format == 'csv':
            csv.write_csv(table, fileprefix + code + '.' + format)
        else:
            raise ValueError('unknown format. Only parquet or csv is supported')
        logger.info(f'download daily {start_dt}~{end_dt}, size={len(rsp)} success')
    except Exception as e:
        logger.error(f'download daily failed|{start_dt}~{end_dt}', exc_info=True)


def download_by_date(start_dt:int, end_dt:int, fileprefix:str='daily-', format='parquet'):
    rsp = None
    try:
        dt = start_dt
        while dt <= end_dt: 
            try:
                rsp = pro.query('daily', trade_date=dt)
                curdt = dt
                dt = next_work_date_int(dt)
                if not rsp.empty:
                    rsp.rename(columns={'ts_code':'code'}, inplace=True)
                else:
                    continue
                if 'trade_date' in rsp.columns:
                    rsp['trade_date'] = rsp['trade_date'].apply(lambda x: date_int_to_datetime(x).date())
                rsp.sort_values(by='trade_date', inplace=True)
                #conn.register('temp_df_view', rsp)
                #conn.execute('REPLACE INTO t_daily_kline SELECT trade_date,code,open,high,low,close,pre_close,change,pct_chg,vol,amount FROM temp_df_view')
                #conn.unregister('temp_df_table')
                table = pa.Table.from_pandas(rsp)
                print(table)
                if format == 'parquet':
                    pq.write_table(table, fileprefix + str(dt) + '.' + format)
                elif format == 'feather':
                    feather.write_feather(rsp, fileprefix + str(dt) + '.' + format)
                elif format == 'csv':
                    csv.write_csv(table, fileprefix + str(dt) + '.' + format)
                else:
                    raise ValueError('unknown format. Only parquet or csv is supported')
                logger.info(f'download {func} {curdt}, size={len(rsp)} success')
            except Exception:
                logger.error(f'download daily failed|{start_dt}~{end_dt}', exc_info=True)
    except Exception as e:
        logger.error(f'download dailyfailed|{start_dt}|{end_dt}', exc_info=True)
        
        
        

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--mode', help='download mode date or range', type=str, default='range')
    parser.add_argument('--format', help='download format parquet,feather or csv', type=str, default='feather')
    parser.add_argument('--code', help='stock code', type=str, default='000001.SZ')
    parser.add_argument('--start_date', help='download start date', type=int, default=datetime_to_date_int(datetime.datetime.now()))
    parser.add_argument('--end_date', help='download end date', type=int, default=datetime_to_date_int(datetime.datetime.now()))
    args = parser.parse_args()
    if args.mode == 'range':    
        download_by_range(args.code, args.start_date, args.end_date, format=args.format)
    elif args.mode == 'date':
        download_by_date(args.start_date, args.end_date, format=args.format)
    else:
        raise ValueError('unknown mode. Only range or date is supported')
        
