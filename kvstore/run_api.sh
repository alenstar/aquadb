#!/bin/bash

curl -X GET -i 'http://192.168.7.80:9999/quote/quotes?codes=sh600183'

curl -X POST -i 'http://192.168.7.80:9999/raft/addserver' --data '{
"server_id":2,
"endpoint":"127.0.0.1:9909"
}'


curl -X POST -i 'http://192.168.7.80:9999/raft/addserver' --data '{
"server_id":3,
"endpoint":"127.0.0.1:9908"
}'

curl -X POST -i 'http://192.168.7.80:9999/trade/createtrader' --data '{
"cash":300000,
"name":"baidu"
}'

curl -X GET -i 'http://192.168.7.80:9999/trade/trader?trader_name=baidu'

curl -X POST -i 'http://192.168.7.80:9999/trade/enterorder' --data '{
"trader_id":363795499323392,
"symbol":"600183.SH",
"side":1,
"quantity":1000,
"price":0
}'

