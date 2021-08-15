#!/bin/bash

./output/bin/kvstore --dbpath=raft1 --endpoint=127.0.0.1:9901 --http=0.0.0.0:9991 --server_id=1 > raft1.log 2>&1 &
./output/bin/kvstore --dbpath=raft2 --endpoint=127.0.0.1:9902 --http=0.0.0.0:9992 --server_id=2 > raft2.log 2>&1 &
./output/bin/kvstore --dbpath=raft3 --endpoint=127.0.0.1:9903 --http=0.0.0.0:9993 --server_id=3 > raft3.log 2>&1 &

sleep 3

curl -X GET -i 'http://192.168.7.80:9991/quote/quotes?codes=sh600183'

curl -X POST -i 'http://192.168.7.80:9991/raft/addserver' --data '{
"server_id":2,
"endpoint":"127.0.0.1:9902"
}'


sleep 9
curl -X POST -i 'http://192.168.7.80:9991/raft/addserver' --data '{
"server_id":3,
"endpoint":"127.0.0.1:9903"
}'

curl -X POST -i 'http://192.168.7.80:9991/trade/createtrader' --data '{
"cash":300000,
"name":"baidu"
}'

curl -X GET -i 'http://192.168.7.80:9991/trade/trader?trader_name=baidu'

curl -X POST -i 'http://192.168.7.80:9991/trade/enterorder' --data '{
"trader_id":363795499323392,
"symbol":"600183.SH",
"side":1,
"quantity":1000,
"price":0
}'

