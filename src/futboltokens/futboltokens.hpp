#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>

#include "vapaee.dispatcher.hpp"

using namespace eosio;
using namespace std;

//* <-- erase just one slash bar to switch those codes (commented/actual-code)
#define PRINT(...) print(__VA_ARGS__)
#define AUX_DEBUG_ACTIONS(...) __VA_ARGS__
/*/
#define PRINT(...)
#define AUX_DEBUG_ACTIONS(...)
//*/


CONTRACT futboltokens : public eosio::contract {

    public:

        // tables -------------------------------------------
        // TABLE accounts (balances) -----------
        // scope: user
        TABLE account {
            eosio::asset balance;
            uint64_t primary_key() const { return balance.symbol.code().raw(); }
        };
        typedef eosio::multi_index< "accounts"_n, account > accounts;
        // ------------------------------------        

        // TABLE stats (currency) -----------
        // scope: supply_code
        TABLE currency_stats {
            eosio::asset           supply;
            eosio::asset           max_supply;
            name                   owner;
            std::vector<name>      issuers;
            uint64_t primary_key()const { return supply.symbol.code().raw(); }
        };
        typedef eosio::multi_index< "stat"_n, currency_stats > stats;
        // ------------------------------------


        using contract::contract;

        ACTION create( name issuer, asset maximum_supply);
        ACTION issue( name to, const asset& quantity, string memo );
        ACTION transfer(name from, name to, asset quantity, string  memo );
        ACTION close( name owner, const symbol& symbol );

        /// auxiliar funcitons --------------------

        void sub_balance( name owner, asset value ) {
            PRINT("vapaee::token::core::action_sub_balance()\n");
            PRINT(" owner: ", owner.to_string(), "\n");
            PRINT(" value: ", value.to_string(), "\n");

            accounts from_acnts( _self, owner.value );

            const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
            eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );

            int64_t amount;
            from_acnts.modify( from, owner, [&]( auto& a ) {
                a.balance.amount -= value.amount;
                amount = a.balance.amount;
            });

            if (amount == 0) {
                action(
                    permission_level{owner,"active"_n},
                    get_self(),
                    "close"_n,
                    std::make_tuple(owner, value.symbol)
                ).send();                
            }
            PRINT("vapaee::token::core::action_sub_balance() ...\n");
        }

        void add_balance( name owner, asset value, name ram_payer ) {
            PRINT("vapaee::token::core::action_add_balance()\n");
            PRINT(" owner: ", owner.to_string(), "\n");
            PRINT(" value: ", value.to_string(), "\n");
            PRINT(" ram_payer: ", ram_payer.to_string(), "\n");

            accounts to_acnts( _self, owner.value );
            auto to = to_acnts.find( value.symbol.code().raw() );
            if( to == to_acnts.end() ) {
                to_acnts.emplace( ram_payer, [&]( auto& a ){
                    a.balance = value;
                });
            } else {
                to_acnts.modify( to, same_payer, [&]( auto& a ) {
                    a.balance += value;
                });
            }
            PRINT("vapaee::token::core::action_add_balance() ...\n");
        }

        // transfer handler --------------------------------------------------
        HANDLER htransfer(name from, name to, asset quantity, string  memo ) {
            PRINT("\nHANDLER vapaeetokens.htransfer()\n");

            // skip handling outcoming transfers from this contract to outside
            if (from == get_self()) {
                PRINT(from.to_string(), " to ", to.to_string(), ": ", quantity.to_string(), " vapaee::token::exchange::handler_transfer() skip...\n");
                return;
            }

            // react to transfer...
        }

};
