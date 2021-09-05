#!/usr/bin/env python

import sys
import os
import pandas as pd
from PyQt5 import QtCore
from PyQt5.QtCore import (QAbstractItemModel, QModelIndex, Qt, QAbstractListModel, QMimeData,
    QDataStream, QByteArray, QJsonDocument, QVariant, QJsonValue, QJsonParseError)
from PyQt5.QtWidgets import (
    QMainWindow, QApplication, QWidget, QAction, 
    QTableView, QTableWidget,QTableWidgetItem,QVBoxLayout,QHBoxLayout,
    QPushButton,QLabel,QComboBox,QLineEdit,QMessageBox,
    QTabWidget,QTextEdit,QPlainTextEdit,QFileDialog, QTreeView
    )
from PyQt5.QtGui import QIcon,QIntValidator
from PyQt5.QtCore import pyqtSlot,QAbstractTableModel
import argparse 
import json
import datetime
from logger import logger
import requests
import duckdb
import tushare as ts
pro = ts.pro_api(
    token='951e1bbba0020a264921d7202a3d4dba4b014479f50cca5286d33f45')

conn = duckdb.connect('./db_kline.ddb')
conn.execute('''
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
)''')

conn.execute('''
CREATE TABLE IF NOT EXISTS t_adj_factor(
trade_date INTEGER,    
code VARCHAR(24),
adj_factor DOUBLE,
PRIMARY KEY(trade_date, code)
)''')

conn.execute(''' 
CREATE TABLE IF NOT EXISTS t_stock_basic(
list_date INTEGER,
code VARCHAR(24),
name VARCHAR(64),
area VARCHAR(64),
industry VARCHAR(64),
fullname VARCHAR(128),
cnspell VARCHAR(16),
market VARCHAR(16),
exchange VARCHAR(16),
list_status INTEGER,
delist_date INTEGER,
PRIMARY KEY(list_date, code)
)''')        


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


class ValueWrapper(object):
    def __init__(self, val, op=None):
        super().__init__()
        self._val = val
        self._op = op

    def set_value(self,val, op=None):
        self._val = val
        self._op = op
        return self

    def get_value(self):
        return self._val

    def set_op(self, op):
        self._op = op
        return self
    
    def get_op(self):
        return self._op
        
    @property
    def value(self):
        return self._val
    
    @property
    def op(self):
        return self._op
        
    def __str__(self):
        return str(self._val)

    def __repr__(self):
        return self._val.__repr__()


class DataFrameModel(QAbstractTableModel): 
    DtypeRole = QtCore.Qt.UserRole + 1000
    ValueRole = QtCore.Qt.UserRole + 1001

    def __init__(self, df=pd.DataFrame(), parent=None):
        super(DataFrameModel, self).__init__(parent)
        self._dataframe = df

    def setDataFrame(self, dataframe):
        self.beginResetModel()
        self._dataframe = dataframe.copy()
        self.endResetModel()

    def dataFrame(self):
        return self._dataframe

    dataFrame = QtCore.pyqtProperty(pd.DataFrame, fget=dataFrame, fset=setDataFrame)

    @QtCore.pyqtSlot(int, QtCore.Qt.Orientation, result=str)
    def headerData(self, section: int, orientation: QtCore.Qt.Orientation, role: int = QtCore.Qt.DisplayRole):
        if role == QtCore.Qt.DisplayRole:
            if orientation == QtCore.Qt.Horizontal:
                return self._dataframe.columns[section]
            else:
                return str(self._dataframe.index[section])
        return QtCore.QVariant()

    def rowCount(self, parent=QtCore.QModelIndex()):
        if parent.isValid():
            return 0
        return len(self._dataframe.index)

    def columnCount(self, parent=QtCore.QModelIndex()):
        if parent.isValid():
            return 0
        return self._dataframe.columns.size

    def data(self, index, role=QtCore.Qt.DisplayRole):
        if not index.isValid() or not (0 <= index.row() < self.rowCount() \
            and 0 <= index.column() < self.columnCount()):
            return QtCore.QVariant()
        row = self._dataframe.index[index.row()]
        col = self._dataframe.columns[index.column()]
        dt = self._dataframe[col].dtype

        val = self._dataframe.iloc[row][col]
        if role == QtCore.Qt.DisplayRole:
            return str(val)
        elif role == DataFrameModel.ValueRole:
            return val
        if role == DataFrameModel.DtypeRole:
            return dt
        return QtCore.QVariant()

    def roleNames(self):
        roles = {
            QtCore.Qt.DisplayRole: b'display',
            DataFrameModel.DtypeRole: b'dtype',
            DataFrameModel.ValueRole: b'value'
        }
        return roles

    def sort(self, column, order):
        colname = self._df.columns.tolist()[column]
        self.layoutAboutToBeChanged.emit()
        self._dataframe.sort_values(colname, ascending= order == QtCore.Qt.AscendingOrder, inplace=True)
        self._dataframe.reset_index(inplace=True, drop=True)
        self.layoutChanged.emit()



class DownloadWidget(QWidget):
    def __init__(self,endpoints,funcs, listkey=None):
        super().__init__()
        self._start_dt = ValueWrapper(0)
        self._end_dt =  ValueWrapper(0)
        self._postdata = ValueWrapper('')
        self._params = ValueWrapper('')
        self._endpoint = ValueWrapper(endpoints[0].get('value'))
        self._func = ValueWrapper(funcs[0].get('value'), op=funcs[0].get('op'))

        self.__fct = None
        self.__last_ep = ''
        self.__endpoints = endpoints
        self.__funcs=funcs 
        self.__listkey = listkey
        self.__ts_funcs = {}
        self.initUI()
        

    def initUI(self):
        #self.setWindowTitle(self._title)
        self.tableView = QTableView() 

        start_lb = QLabel('起始日期')
        start_le = QLineEdit()
        start_le.textChanged.connect(lambda x:self._start_dt.set_value(x))

        end_lb = QLabel('截至日期')
        end_le = QLineEdit()
        end_le.textChanged.connect(lambda x:self._end_dt.set_value(x))

        dlBtn = QPushButton('下载')
        dlBtn.clicked.connect(self.on_download)

        upBtn = QPushButton('更新码表')
        upBtn.clicked.connect(self.on_update)

        hbox5 = QHBoxLayout()
        hbox5.addWidget(start_lb)
        hbox5.addWidget(start_le)

        hbox4 = QHBoxLayout()
        hbox4.addWidget(end_lb)
        hbox4.addWidget(end_le)

        hbox3 = QHBoxLayout()
        hbox3.addLayout(hbox5)
        hbox3.addLayout(hbox4)
        hbox3.addWidget(dlBtn)
        hbox3.addWidget(upBtn)

        self.epCB = QComboBox()
        self.epCB.addItems([x.get('name') for x in self.__endpoints])
        self.epCB.currentIndexChanged.connect(lambda i:self._endpoint.set_value(self.__endpoints[i].get('value')))
        self.funcCB = QComboBox()
        self.funcCB.addItems([x.get('name') for x in self.__funcs])
        self.funcCB.currentIndexChanged.connect(lambda i:self._func.set_value(self.__funcs[i].get('value'),self.__funcs[i].get('op')))

        indis_lb = QLabel('参数')
        indis_le = QLineEdit()
        indis_le.textChanged.connect(lambda x:self._params.set_value(x))
        self.execBtn = QPushButton('查询')
        self.execBtn.clicked.connect(self.on_click)
        hbox2 = QHBoxLayout()
        hbox2.addWidget(self.epCB)
        hbox2.addWidget(self.funcCB)
        hbox2.addWidget(indis_lb)
        hbox2.addWidget(indis_le)
        hbox2.addWidget(self.execBtn)

        # Add box layout, add table to box layout and add box layout to widget
        self.layout = QVBoxLayout()

        #self.layout.addLayout(hbox1) 
        self.layout.addLayout(hbox3) 

        self.layout.addLayout(hbox2) 

        self.layout.addWidget(self.tableView) 
        self.setLayout(self.layout) 



    @pyqtSlot()
    def on_click(self):
        mapping = {'daily':'t_daily_kline','adj_factor':'t_adj_factor','stock_basic':'t_stock_basic'}
        func = self._func.get_value()
        rsp = None
        try:
            df = conn.execute(f"SELECT * from {mapping.get(func)} where {self._params.get_value()}").fetchdf()
            rsp = df
            if df.empty:
                self.showMsgDialog('提示','没有找到数据: ' + self._params.get_value())
            model = DataFrameModel(df=rsp)
            self.tableView.setModel(model)
        except Exception as e:
            logger.error(f'call {self._func} failed|{start_dt}|{end_dt}', exc_info=True)
            self.showMsgDialog('查询异常','异常: ' + self._func.get_value() + '|' + self._endpoint.get_value(), detailed_text=str(e))

    @pyqtSlot()
    def on_download(self):
        func = self._func.get_value()
        rsp = None
        try:
            start_dt = int(self._start_dt.get_value())
            end_dt = int(self._end_dt.get_value())
            dt = start_dt
            while dt <= end_dt: 
                try:
                    rsp = pro.query(func, trade_date=dt)
                    curdt = dt
                    dt = next_work_date_int(dt)
                    if not rsp.empty:
                        rsp.rename(columns={'ts_code':'code'}, inplace=True)
                    else:
                        continue
                    if 'trade_date' in rsp.columns:
                        rsp['trade_date'] = pd.to_numeric(rsp['trade_date']) 
                    #conn.register('temp_df_view', rsp)
                    #conn.execute('REPLACE INTO t_daily_kline SELECT trade_date,code,open,high,low,close,pre_close,change,pct_chg,vol,amount FROM temp_df_view')
                    #conn.unregister('temp_df_table')
                    if func == 'daily':
                        rsp = self.save_daily_kline(rsp)
                    elif func == 'adj_factor':
                        rsp = self.save_adj_factor(rsp)
                    logger.info(f'download {func} {curdt}, size={len(rsp)} success')
                except Exception:
                    logger.error(f'call {self._func} failed|{start_dt}|{end_dt}', exc_info=True)
        except Exception as e:
            logger.error(f'call {func} failed|{start_dt}|{end_dt}', exc_info=True)
            self.showMsgDialog('调用异常','请求异常: ' + self._func.get_value() + '|' + self._endpoint.get_value(), detailed_text=str(e))

    @pyqtSlot()
    def on_update(self):
        func = 'stock_basic' # self._func.get_value()
        rsp = None
        try:
            rsp = pro.query(func, fields='ts_code,name,area,industry,fullname,cnspell,market,exchange,list_status,list_date,delist_date')
            if not rsp.empty:
                rsp.rename(columns={'ts_code':'code'}, inplace=True)
            else:
                self.showMsgDialog('提示','没有找到数据: ' + func)
                return
            rsp['list_date'] = pd.to_numeric(rsp['list_date']) 
            rsp['delist_date'] = rsp['delist_date'].apply(lambda x: int(x) if x is not None else 29991231)
            lsit_status_strto_int = {'L':1,'D':2,'P':3,'':0}
            rsp['list_status'] = rsp['list_status'].apply(lambda x: lsit_status_strto_int.get(x) if x in lsit_status_strto_int else 0)
            #conn.register('temp_df_view', rsp)
            #conn.execute('REPLACE INTO t_daily_kline SELECT trade_date,code,open,high,low,close,pre_close,change,pct_chg,vol,amount FROM temp_df_view')
            #conn.unregister('temp_df_table')
            rsp = self.save_stock_basic(rsp)
            model = DataFrameModel(df=rsp)
            self.tableView.setModel(model)
            logger.info(f'download {func}, size={len(rsp)} success')
        except Exception as e:
            logger.error(f'call {func} failed', exc_info=True)
            self.showMsgDialog('调用异常','请求异常: ' + self._func.get_value() + '|' + self._endpoint.get_value(), detailed_text=str(e))

    def save_daily_kline(self, df):    
        conn.register('temp_df_view', df)
        conn.execute('INSERT INTO t_daily_kline SELECT trade_date,code,open,high,low,close,pre_close,change,pct_chg,vol,amount FROM temp_df_view')
        conn.unregister('temp_df_table')
        return df

    def save_adj_factor(self, df):    
        conn.register('temp_df_view', df)
        conn.execute('INSERT INTO t_adj_factor SELECT trade_date,code,adj_factor FROM temp_df_view')
        conn.unregister('temp_df_table')
        return df

    def save_daily_indis(self, df):    
        conn.register('temp_df_view', df)
        conn.execute('INSERT INTO t_daily_indis SELECT trade_date,code,close,turnover_rate,turnover_rate_f,volume_ratio,pe,pe_ttm,pb,ps,ps_ttm,dv_ratio,dv_ttm,total_share,float_share,free_share,total_mv,circ_mv FROM temp_df_view')
        conn.unregister('temp_df_table')
        return df


    def save_stock_basic(self, df):    
        conn.register('temp_df_view', df)
        conn.execute('INSERT INTO t_stock_basic SELECT list_date,code,name,area,industry,fullname,cnspell,market,exchange,list_status,delist_date FROM temp_df_view')
        conn.unregister('temp_df_table')
        return df


    def showMsgDialog(self, title:str,text:str,detailed_text=''):
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Information)
        msg.setText(text)
        msg.setWindowTitle(title)
        #msg.setDetailedText(detailed_text)
        msg.setInformativeText(detailed_text)
        msg.setStandardButtons(QMessageBox.Ok)
        return msg.exec_()



class App(QTabWidget):
    def __init__(self,conf):
        super().__init__()
        self._title = '接口查询工具'
        self._width = 900
        self._height = 600
        self._downloadWidget = DownloadWidget(conf.get('tushare').get('endpoints'),conf.get('tushare').get('funcs'))
        #self._candlestickWidget = CandlestickWidget(conf.get('quotes').get('endpoints'), conf.get('quotes').get('funcs'))

        self.initUI()
        
    def initUI(self):
        self.setWindowTitle(self._title)

        # add indi subWidget
        self.addTab(self._downloadWidget, '下载K线')
        # add sql subWidget
        # self.addTab(self._candlestickWidget, 'K线图')

        self.resize(self._width,self._height)
        # Show widget
        self.show()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--conf', help='json configure file', type=str, default='config.json')
    args = parser.parse_args()
    conf = {
        'quotes': {
            'endpoints': [
                {'name':'server-1','value':'http://192.168.7.80:9999/quote'},
                {'name':'server-2','value':'http://192.168.7.80:9999/quote'}
            ],
            'funcs': [
                {'name':'行情','value':'quotes','op':'get'}
            ]
        },
        'trade':{
            'endpoints':[{'name':'server-1','value':'http://192.168.7.80:9999/trade'}],
            'funcs': [
                {'name':'订单','value':'orderlist','op':'get'},
                {'name':'持仓','value':'positionlist','op':'get'}
            ]
        },
        'tushare': {
            'token':'',
            'mysql_url':'',
            'local_db': 'db_kline',
            'funcs': [
                {'name':'日线行情','value':'daily','op':'get'},
                {'name':'复权因子','value':'adj_factor','op':'get'},
                {'name':'基础信息','value':'stock_basic','op':'get'}
            ],
            'endpoints': [
                {'name':'server-1','value':'http://192.168.7.80:9999/quote'},
                {'name':'server-2','value':'http://192.168.7.80:9999/quote'}
            ]
        }
    }
    if os.path.isfile(args.conf):
        with open(args.conf,'r',encoding='utf8')as fp:
            conf = json.loads(fp.read())

    app = QApplication(sys.argv)
    ex = App(conf)
    sys.exit(app.exec_())  
