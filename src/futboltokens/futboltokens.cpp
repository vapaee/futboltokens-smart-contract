#include "goldencenser.hpp"


futboltokens::create(name owner, asset maximum_supply) {
    PRINT("vapaee::token::core::create()\n");
    PRINT(" owner: ", owner.to_string(), "\n");
    PRINT(" maximum_supply: ", maximum_supply.to_string(), "\n");

    require_auth( owner );

    auto sym = maximum_supply.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( maximum_supply.is_valid(), "invalid supply");
    eosio_assert( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    eosio_assert( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) {
        s.supply.symbol = maximum_supply.symbol;
        s.max_supply    = maximum_supply;
        s.owner         = owner;
    });

    action(
        permission_level{owner,"active"_n},
        get_self(),
        "addtoken"_n,
        std::make_tuple(get_self(), maximum_supply.symbol.code(), maximum_supply.symbol.precision(), owner)
    ).send();

    PRINT("vapaee::token::core::create() ...\n");
}

futboltokens::issue( name to, const asset& quantity, string memo ) {
    PRINT("vapaee::token::core::issue()\n");
    PRINT(" to: ", to.to_string(), "\n");
    PRINT(" quantity: ", quantity.to_string(), "\n");
    PRINT(" memo: ", memo.c_str(), "\n");

    // check on symbol
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    // check token existance
    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    // getting currency's acosiated app contract account
    // vapaee::bgbox::apps apps_table(vapaee::bgbox::contract, vapaee::bgbox::contract.value);
    // auto app = apps_table.get(st.app, "app not found");
    // name appcontract = app.contract;
    // PRINT("  appid: ", std::to_string((int) st.app), "\n");
    // PRINT("  appcontract: ", appcontract.to_string(), "\n");

    // check authorization (issuer of appcontract)
    name everyone = "everyone"_n;
    name issuer = st.owner;
    require_auth( issuer );
    
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    
    // update current supply
    statstable.modify( st, same_payer, [&]( auto& s ) {
        s.supply += quantity;
    });

    // update issuer balance silently
    add_balance( issuer, quantity, issuer );

    // if target user is not issuer the send an inline action
    if( to != issuer ) {
        action(
            permission_level{issuer,"active"_n},
            get_self(),
            "transfer"_n,
            std::make_tuple(issuer, to, quantity, memo)
        ).send();
    }
    PRINT("vapaee::token::core::issue() ...\n");
}

futboltokens::transfer(name from, name to, asset quantity, string memo) {
    PRINT("vapaee::token::core::transfer()\n");
    PRINT(" from: ", from.to_string(), "\n");
    PRINT(" to: ", to.to_string(), "\n");
    PRINT(" quantity: ", quantity.to_string(), "\n");
    PRINT(" memo: ", memo.c_str(), "\n");


    eosio_assert( from != to, "cannot transfer to self" );
    require_auth( from );
    eosio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable( _self, sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto ram_payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, ram_payer );
    
    PRINT("vapaee::token::core::transfer() ...\n");
}

futboltokens::close( name owner, const symbol& symbol ) {
    PRINT("vapaee::token::core::close()\n");
    PRINT(" owner: ", owner.to_string(), "\n");
    PRINT(" symbol: ", symbol.code().to_string(), "\n");

    require_auth( owner );
    accounts acnts( _self, owner.value );
    auto it = acnts.find( symbol.code().raw() );
    eosio_assert( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
    eosio_assert( it->balance.amount == 0, "Cannot close because the balance is not zero." );
    acnts.erase( it );
    PRINT("vapaee::token::core::close() ...\n");
}


EOSIO_DISPATCH_VAPAEE (
    goldencenser,
    (create)(issue)(transfer)(close),
    (htransfer)
)