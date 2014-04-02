#pragma once
#include <bts/blockchain/blockchain_db.hpp>

#include <functional>
#include <unordered_map>

namespace bts { namespace blockchain {

   namespace detail { class wallet_impl; }

   struct output_index
   {
        output_index( uint32_t block_id = 0, uint16_t trx_id = 0, uint16_t output_id = 0)
        :block_idx(block_id),trx_idx(trx_id),output_idx(output_id){}

        friend bool operator < ( const output_index& a, const output_index& b )
        {
           if( a.block_idx == b.block_idx )
           {
              if( a.trx_idx == b.trx_idx )
              {
                 return a.output_idx < b.output_idx;
              }
              else
              {
                 return a.trx_idx < b.trx_idx;
              }
           }
           else
           {
              return a.block_idx < b.block_idx;
           }
        }
        friend bool operator == ( const output_index& a, const output_index& b )
        {
          return a.block_idx == b.block_idx && a.trx_idx == b.trx_idx && a.output_idx == b.output_idx;
        }
        operator std::string()const;

        uint32_t   block_idx;
        uint16_t   trx_idx;
        uint16_t   output_idx;
   };

   struct transaction_state
   {
      transaction_state():block_num(-1),valid(false){}
      signed_transaction  trx;
      std::string         memo;
      uint32_t            block_num; // block that included it, -1 if not included
      bool                valid; // is this transaction currently valid if it is not confirmed...
   };

   /** takes 4 parameters, current block, last block, current trx, last trx */
   typedef std::function<void(uint32_t,uint32_t,uint32_t,uint32_t)> scan_progress_callback;

   /**
    *  The wallet stores all signed_transactions that reference one of its
    *  addresses in the inputs or outputs section.  It also tracks all
    *  private keys, spend states, etc...
    */
   class wallet
   {
        public:
           wallet();
           ~wallet();

           void open( const fc::path& wallet_file, const std::string& password );
           void create( const fc::path& wallet_file, const std::string& base_pass, const std::string& key_pass, bool is_brain = false );
           void save();
           void backup_wallet( const fc::path& backup_path );

           void import_bitcoin_wallet( const fc::path& dir, const std::string& passphrase );

           bts::address                                 import_key( const fc::ecc::private_key& key, const std::string& label = "" );
           bts::address                                 new_recv_address( const std::string& label = "" );
           std::unordered_map<bts::address,std::string> get_recv_addresses()const;

           void                                         add_send_address( const bts::address&, const std::string& label = "" );
           std::unordered_map<bts::address,std::string> get_send_addresses()const;

           asset                                        get_balance( asset::type t );
           asset                                        get_margin( asset::type t, asset& collat );
           void                                         set_stake( uint64_t stake, uint32_t head_idx  );
           void                                         set_fee_rate( const asset& pts_per_byte );
           uint64_t                                     last_scanned()const;

           /** provides the password required to gain access to the private keys
            *  associated with this wallet.
            */
           void                  unlock_wallet( const std::string& key_password );
           /**
            *  removes private keys from memory
            */
           void                  lock_wallet();
           bool                  is_locked()const;

           signed_transaction    collect_coindays( uint64_t cdd, uint64_t& cdd_collected, const std::string& label = "mining" );
           signed_transaction    transfer( const asset& amnt, const bts::address& to, const std::string& memo = "change" );
           signed_transaction    bid( const asset& amnt, const price& ratio );
           signed_transaction    short_sell( const asset& amnt, const price& ratio );
           signed_transaction    cancel_bid( const output_reference& bid );
           signed_transaction    cancel_bid( const output_index& bid_idx );
           signed_transaction    cancel_short_sell( const output_reference& bid );

           /** returns all transactions issued */
           std::unordered_map<transaction_id_type, transaction_state> get_transaction_history()const;

           // automatically covers position with lowest margin which is the position entered 
           // at the lowest price...
           signed_transaction    cover( const asset& amnt );
           /**
            * Combines all margin positions into a new position with additional collateral. In the
            * future smarter wallets may want to only apply this to a subset of the positions.
            *
            * @param u - the asset class to increase margin for (anything but BTS)
            * @param amnt - the amount of additional collateral (must be BTS)
            */
           signed_transaction    add_margin( const asset& collateral_amount /*bts*/, asset::type u );

           // all outputs are claim_by_bid
           std::unordered_map<output_reference,trx_output> get_open_bids();

           // all outputs are claim_by_long
           std::unordered_map<output_reference,trx_output> get_open_short_sell();

           // all outputs are claim_by_cover,
           std::unordered_map<output_reference,trx_output> get_open_shorts();

           // all outputs are claim_by_bid, these bids were either canceled or executed
           std::unordered_map<output_reference,trx_output> get_closed_bids();

           // all outputs are claim_by_long, these bids were either canceled or executed
           std::unordered_map<output_reference,trx_output> get_closed_short_sell();

           // all outputs are claim_by_cover, these short positions have been covered
           std::unordered_map<output_reference,trx_output> get_covered_shorts();

           void sign_transaction( signed_transaction& trx, const bts::address& addr );
           bool scan_chain( blockchain_db& chain, uint32_t from_block_num = 0,  scan_progress_callback cb = scan_progress_callback() );
           void mark_as_spent( const output_reference& r );
           void dump();

        private:
           std::unique_ptr<detail::wallet_impl> my;
   };
} } // bts::blockchain

FC_REFLECT( bts::blockchain::output_index, (block_idx)(trx_idx)(output_idx) )
FC_REFLECT( bts::blockchain::transaction_state, (trx)(memo)(block_num) )
