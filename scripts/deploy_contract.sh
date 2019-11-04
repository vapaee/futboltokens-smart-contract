#!/bin/bash
# cleos create account eosio futboltokens EOS8RoCAXxWYUW2v4xkG19F57BDVBzpt9NN2iDsD1ouQNyV2BkiNc
# cleos set account permission futboltokens active '{"threshold": 1,"keys": [{"key": "EOS8RoCAXxWYUW2v4xkG19F57BDVBzpt9NN2iDsD1ouQNyV2BkiNc","weight": 1}],"accounts": [{"permission":{"actor":"futboltokens","permission":"eosio.code"},"weight":1}]}' owner -p futboltokens
# telosmain set account permission futboltokens active '{"threshold": 1,"keys": [{"key": "EOS61qh4BFJJhyJgUwDVpNCDwXHz29hnB3ARiKjpwb9Fha9vMRaVH","weight": 1}],"accounts": [{"permission":{"actor":"futboltokens","permission":"eosio.code"},"weight":1}]}' owner -p futboltokens
force=false
NET=

if [ "$1" == "prod" ]; then
   NET='--url https://telos.eos.barcelona'
fi

if [ "$1" == "test" ]; then
   NET='--url https://testnet.telos.caleos.io'
fi


FUTBOLTOKENS_HOME=../src/futboltokens
echo "-------- futboltokens ---------"
cd $FUTBOLTOKENS_HOME
eosio-cpp -o futboltokens.wasm futboltokens.cpp --abigen
echo "cleos $NET set contract futboltokens $PWD -p futboltokens@active"
cleos $NET set contract futboltokens $PWD -p futboltokens@active




