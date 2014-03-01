#pragma once
#include <bts/blockchain/block.hpp>
#include <bts/blockchain/asset.hpp>
#include <fc/optional.hpp>
#include <fc/filesystem.hpp>

namespace bts { namespace blockchain {

  namespace detail { class market_db_impl; }
  struct price_point;

 /**
  *   Bids:  (offers to buy Base Unit with Quote Unit)
  *   Quote Unit, Base Unit  Price  UnspentOutput 
  *
  *   Asks:  (offers to sell Base Unit for Quote Unit ) includes open short orders when BaseUnit = BTS)
  *   Quote Unit, Base Unit  Price  UnspentOutput 
  *
  */
  struct market_order
  {
     market_order( const price& p, const output_reference& loc );
     market_order(){}
     asset_type        base_unit;
     asset_type        quote_unit;
     fc::uint128_t     ratio; // 64.64
     output_reference  location;

     price get_price()const;
  };
  bool operator < ( const market_order& a, const market_order& b );
  bool operator == ( const market_order& a, const market_order& b );

  struct margin_call
  {
     margin_call( const price& callp, const output_reference& loc ):call_price(callp),location(loc){}

     price            call_price;
     output_reference location;
  };
  bool operator < ( const margin_call& a, const margin_call& b );
  bool operator == ( const margin_call& a, const margin_call& b );
  
  /**
   *  Manages the current state of the market to enable effecient
   *  pairing of the highest bid with the lowest ask.
   */
  class market_db
  {
     public:
       market_db();
       ~market_db();

       void open( const fc::path& db_dir );
       std::vector<market_order> get_bids( asset::type quote_unit, asset::type base_unit )const;
       std::vector<market_order> get_asks( asset::type quote_unit, asset::type base_unit )const;
       std::vector<margin_call>  get_calls( price call_price )const;

       
       /**
        * assumes bid and ask of same price units 
        **/
       void set_spread( const price& bid, const price& ask );
       price get_lowest_ask_price( asset::type quote_unit, asset::type base_unit );
       price get_highest_bid_price( asset::type quote_unit, asset::type base_unit );

       /** 
        *  Returns the minimum of total volume of orders on either the bid or
        *  ask side of the market.  
        */
       uint64_t get_depth( asset::type quote_unit );

       /** @param depth - the amount of bts backing the order used to
        * track minimum market depth to facilitate trading.
        */
       void insert_bid( const market_order& m, uint64_t depth );
       void insert_ask( const market_order& m, uint64_t depth );
       void remove_bid( const market_order& m, uint64_t depth );
       void remove_ask( const market_order& m, uint64_t depth );
       void insert_call( const margin_call& c, uint64_t depth );
       void remove_call( const margin_call& c, uint64_t depth );

       /** @pre quote > base  */
       fc::optional<market_order> get_highest_bid( asset::type quote, asset::type base );
       /** @pre quote > base  */
       fc::optional<market_order> get_lowest_ask( asset::type quote, asset::type base );

       void push_price_point( const price_point& pt );

       /**
        *  This method returns the price history for a given asset pair for a given range and block granularity. 
        */
       std::vector<price_point> get_history( asset::type quote, asset::type base, fc::time_point_sec from, fc::time_point_sec to, uint32_t blocks_per_point = 1 );

     private:
       std::unique_ptr<detail::market_db_impl> my;
  };

} }  // bts::blockchain

FC_REFLECT( bts::blockchain::market_order, (base_unit)(quote_unit)(ratio)(location) );
FC_REFLECT( bts::blockchain::margin_call, (call_price)(location) )

