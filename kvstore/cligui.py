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
from logger import logger
import requests

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


        
class QJsonTreeItem(object):
    def __init__(self, parent=None):

        self.mParent = parent
        self.mChilds = []
        self.mType =None
        self.mValue =None

    def appendChild(self, item):
        self.mChilds.append(item)

    def child(self, row:int):
        return self.mChilds[row]

    def parent(self):
        return self.mParent

    def childCount(self):
        return len(self.mChilds)

    def row(self):
        if self.mParent is not None:
            return self.mParent.mChilds.index(self)
        return 0

    def setKey(self, key:str):
        self.mKey = key

    def setValue(self, value:str):
       self. mValue = value

    def setType(self, type:QJsonValue.Type):
        self.mType = type

    def key(self):
        return self.mKey

    def value(self):
        return self.mValue

    def type(self):
        return self.mType

    def load(self, value, parent=None):

        rootItem = QJsonTreeItem(parent)
        rootItem.setKey("root")
        jsonType = None

        try:
            value = value.toVariant()
            jsonType = value.type()
        except AttributeError:
            pass

        try:
            value = value.toObject()
            jsonType = value.type()

        except AttributeError:
            pass

        if isinstance(value, dict):
            # process the key/value pairs
            for key in value:
                v = value[key]
                child = self.load(v, rootItem)
                child.setKey(key)
                try:
                    child.setType(v.type())
                except AttributeError:
                    child.setType(v.__class__)
                rootItem.appendChild(child)

        elif isinstance(value, list):
            # process the values in the list
            for i, v in enumerate(value):
                child = self.load(v, rootItem)
                child.setKey(str(i))
                child.setType(v.__class__)
                rootItem.appendChild(child)

        else:
            # value is processed
            rootItem.setValue(value)
            try:
                rootItem.setType(value.type())
            except AttributeError:
                if jsonType is not None:
                    rootItem.setType(jsonType)
                else:
                    rootItem.setType(value.__class__)

        return rootItem


class QJsonModel(QAbstractItemModel):
    def __init__(self, json=None, parent =None):
        super().__init__(parent)
        self.mRootItem = QJsonTreeItem()
        self.mHeaders = ["key","value","type"]
        if json is not None:
            self.loadJson(json)

    def load(self,fileName):
        if fileName is None or fileName is False:
            return False

        with open(fileName,"rb",) as file:
            if file is None:
                return False
            self.loadJson(file.read())

    def loadJson(self, json):
        error = QJsonParseError()
        if isinstance(json, str):
            self.mDocument = QJsonDocument.fromJson(json,error)
        else:
            self.mDocument = QJsonDocument(json)

        if self.mDocument is not None:
            self.beginResetModel()
            if self.mDocument.isArray():
                self.mRootItem.load( list( self.mDocument.array()))
            else:
                self.mRootItem = self.mRootItem.load( self.mDocument.object())
            self.endResetModel()

            return True

        print("QJsonModel: error loading Json")
        return False

    def data(self, index: QModelIndex, role: int = ...):
        if not index.isValid():
            return QVariant()

        item = index.internalPointer()
        col = index.column()

        if role == Qt.DisplayRole:
            if col == 0:
                return str(item.key())
            elif col == 1:
                return str(item.value())
            elif col == 2:
                return str(item.type())

        return QVariant()

    def headerData(self, section: int, orientation: Qt.Orientation, role: int = ...):
        if role != Qt.DisplayRole:
            return QVariant()

        if orientation == Qt.Horizontal:
            return self.mHeaders[section]

        return QVariant()

    def index(self, row: int, column: int, parent: QModelIndex = ...):
        if not self.hasIndex(row, column, parent):
            return QModelIndex()

        if not parent.isValid():
            parentItem = self.mRootItem
        else:
            parentItem = parent.internalPointer()
        try:
            childItem = parentItem.child(row)
            return self.createIndex(row, column, childItem)
        except IndexError:
            return QModelIndex()

    def parent(self, index: QModelIndex):
        if not index.isValid():
            return QModelIndex()

        childItem = index.internalPointer()
        parentItem = childItem.parent()

        if parentItem == self.mRootItem:
            return QModelIndex()

        return self.createIndex(parentItem.row(),0, parentItem)

    def rowCount(self, parent: QModelIndex = ...):
        if parent.column() > 0:
            return 0
        if not parent.isValid():
            parentItem = self.mRootItem
        else:
            parentItem = parent.internalPointer()

        return parentItem.childCount()

    def columnCount(self, parent: QModelIndex = ...):
        return 3
        

        
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



class RESTWidget(QWidget):
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
        self.initUI()
        
    def initUI(self):
        #self.setWindowTitle(self._title)
        if self.__listkey is None:
            self.treeView = QTreeView() #QTableView()
        else:
            self.tableView = QTableView() 

        options_lb = QLabel('POST参数')
        options_le = QLineEdit()
        options_le.textChanged.connect(lambda x:self._postdata.set_value(x))
        hbox5 = QHBoxLayout()
        hbox5.addWidget(options_lb)
        hbox5.addWidget(options_le)

        self.epCB = QComboBox()
        self.epCB.addItems([x.get('name') for x in self.__endpoints])
        self.epCB.currentIndexChanged.connect(lambda i:self._endpoint.set_value(self.__endpoints[i].get('value')))
        self.funcCB = QComboBox()
        self.funcCB.addItems([x.get('name') for x in self.__funcs])
        self.funcCB.currentIndexChanged.connect(lambda i:self._func.set_value(self.__funcs[i].get('value'),self.__funcs[i].get('op')))

        indis_lb = QLabel('GET参数')
        indis_le = QLineEdit()
        indis_le.textChanged.connect(lambda x:self._params.set_value(x))
        self.execBtn = QPushButton('提交')
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
        self.layout.addLayout(hbox5) 

        self.layout.addLayout(hbox2) 

        if self.__listkey is None:
            self.layout.addWidget(self.treeView) 
        else:
            self.layout.addWidget(self.tableView) 
        self.setLayout(self.layout) 



    @pyqtSlot()
    def on_click(self):
        func = self._func.get_value()
        rsp = None
        try:
            if self._func.get_op() == 'get':
                params = self._params.get_value()
                rsp = requests.get(self._endpoint.get_value() + '/' + self._func.get_value() + ( '' if len(params) == 0 else '?'+ params))
            elif self._func.get_op() == 'post':
                logger.info(f'{self._func.get_value()}|{json.dumps(self._postdata.get_value())}')
                rsp = requests.post(self._endpoint.get_value() + '/' + self._func.get_value(), headers={'Content-Type': 'application/json'}, data=self._postdata.get_value())
            else:
                self.showMsgDialog('调用异常','不支持的操作: ' + self._func.get_value() + '|' + self._func.get_op() + '|' + self._endpoint.get_value(), detailed_text=str(e))
                return
            if rsp.status_code != 200:
                logger.info(f'{rsp}|{rsp.text}')
                self.showMsgDialog('调用失败','无效回包: ' + self._func.get_value() + '|' + self._func.get_op() + '|' + self._endpoint.get_value(), detailed_text=str(rsp))
                return
            logger.info(f'{rsp}|{rsp.json()}')
            if self.__listkey is None:
                model = QJsonModel(json=rsp.json())
                self.treeView.setModel(model)
            else:
                model = DataFrameModel(pd.DataFrame(rsp.json()[self.__listkey]))
                self.tableView.setModel(model)
        except Exception as e:
            logger.error(f'call {self._func} failed|{self._params}|{self._postdata}|{rsp}', exc_info=True)
            self.showMsgDialog('调用异常','请求异常: ' + self._func.get_value() + '|' + self._endpoint.get_value(), detailed_text=str(e))


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
        self._raftWidget = RESTWidget(conf.get('raft').get('endpoints'),conf.get('raft').get('funcs'))
        self._quotesWidget = RESTWidget(conf.get('quotes').get('endpoints'), conf.get('quotes').get('funcs'))
        self._tradeWidget = RESTWidget(conf.get('trade').get('endpoints'), conf.get('trade').get('funcs'), listkey='data')

        self.initUI()
        
    def initUI(self):
        self.setWindowTitle(self._title)

        # add indi subWidget
        self.addTab(self._raftWidget, 'Raft')
        # add sql subWidget
        self.addTab(self._quotesWidget, '行情查询')
        self.addTab(self._tradeWidget, '交易查询')

        self.resize(self._width,self._height)
        # Show widget
        self.show()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--conf', help='json configure file', type=str, default='config.json')
    args = parser.parse_args()
    conf = {
        'raft': {
            'endpoints': [
                {'name':'server-1','value':'http://192.168.7.80:9991/raft'},
                {'name':'server-2','value':'http://192.168.7.80:9992/raft'},
                {'name':'server-3','value':'http://192.168.7.80:9993/raft'},
                {'name':'server-x','value':'http://192.168.7.80:9999/raft'}
            ],
            'funcs': [
                {'name':'状态','value':'status','op':'get'},
                {'name':'服务列表','value':'serverlist','op':'get'},
                {'name':'添加服务','value':'addserver','op':'post'},
                {'name':'移除服务','value':'removeserver','op':'post'}
            ]
        },
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
            'funcs': [
                {'name':'日线行情','value':'daily','op':'get'},
                {'name':'持仓','value':'positionlist','op':'get'}
            ]
        }
    }
    if os.path.isfile(args.conf):
        with open(args.conf,'r',encoding='utf8')as fp:
            conf = json.loads(fp.read())

    app = QApplication(sys.argv)
    ex = App(conf)
    sys.exit(app.exec_())  
