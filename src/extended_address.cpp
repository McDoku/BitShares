#include <bts/extended_address.hpp>
#include <fc/exception/exception.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/raw.hpp>

namespace bts 
{
  extended_public_key::extended_public_key()
  {
  }

  extended_public_key::~extended_public_key(){} 

  extended_public_key::extended_public_key( const fc::ecc::public_key& key, const fc::sha256& code )
  :pub_key(key),chain_code(code)
  {
  }


  extended_public_key extended_public_key::child( uint32_t child_idx )const
  { try {
      fc::sha512::encoder enc;
      fc::raw::pack( enc, pub_key );
      fc::raw::pack( enc, child_idx );

      fc::sha512 ikey  = enc.result();
      fc::sha256 ikey_left;
      fc::sha256 ikey_right;

      memcpy( (char*)&ikey_left, (char*)&ikey, sizeof(ikey_left) );
      memcpy( (char*)&ikey_right, ((char*)&ikey) + sizeof(ikey_left), sizeof(ikey_right) );

      extended_public_key child_key;
      child_key.chain_code = ikey_right;
      child_key.pub_key    = pub_key.add(ikey_left);

      return child_key;
  } FC_RETHROW_EXCEPTIONS( warn, "child index ${child_idx}", 
                                  ("exteneded_key",*this)("child_idx", child_idx ) ) }



  extended_private_key::extended_private_key( const fc::sha256& key, const fc::sha256& ccode )
  :priv_key(key),chain_code(ccode)
  {
  }

  extended_private_key::extended_private_key( const fc::sha512& seed )
  {
    memcpy( (char*)&priv_key, (char*)&seed, sizeof(priv_key) );
    memcpy( (char*)&chain_code, ((char*)&seed) + sizeof(priv_key), sizeof(priv_key) );
  }

  extended_private_key::extended_private_key(){}

  extended_private_key extended_private_key::child( uint32_t child_idx, bool pub_derivation )const
  { try {
    extended_private_key child_key;

    fc::sha512::encoder enc;
    if( pub_derivation )
    {
      fc::raw::pack( enc, get_public_key() );
    }
    else
    {
      uint8_t pad = 0;
      fc::raw::pack( enc, pad );
      fc::raw::pack( enc, priv_key );
    }
    fc::raw::pack( enc, child_idx );
    fc::sha512 ikey = enc.result();

    fc::sha256 ikey_left;
    fc::sha256 ikey_right;

    memcpy( (char*)&ikey_left, (char*)&ikey, sizeof(ikey_left) );
    memcpy( (char*)&ikey_right, ((char*)&ikey) + sizeof(ikey_left), sizeof(ikey_right) );

    child_key.priv_key  = fc::ecc::private_key::generate_from_seed( priv_key, ikey_left ).get_secret();
    child_key.chain_code = ikey_right; 

    return child_key;
  } FC_RETHROW_EXCEPTIONS( warn, "child index ${child_idx}", 
                                  ("exteneded_key",*this)("child_idx", child_idx ) ) }

  extended_private_key::operator fc::ecc::private_key()const
  {
    return fc::ecc::private_key::regenerate( priv_key );
  }

  fc::ecc::public_key extended_private_key::get_public_key()const
  {
    return fc::ecc::private_key::regenerate( priv_key ).get_public_key();
  }




   extended_address::extended_address()
   { }

   extended_address::extended_address( const std::string& base58str )
   { try {

      uint32_t checksum = 0;
      std::vector<char> data = fc::from_base58( base58str );
      FC_ASSERT( data.size() == (33+32+4) );

      fc::datastream<const char*> ds(data.data(),data.size());
      fc::raw::unpack( ds, *this    );
      fc::raw::unpack( ds, checksum );

      FC_ASSERT( checksum == fc::hash64( data.data(), sizeof(*this) ) );

   } FC_RETHROW_EXCEPTIONS( warn, "decoding address ${address}", ("address",base58str) ) }

   extended_address::extended_address( const extended_public_key& pub )
   :addr(pub)
   {
   }

   bool extended_address::is_valid()const
   {
      return addr.pub_key.valid();
   }

   extended_address::operator std::string()const
   {

      std::vector<char> data = fc::raw::pack(*this);
      uint32_t checksum = uint32_t(fc::hash64( data.data(), data.size() ));

      data.resize( data.size() + 4 );
      memcpy( data.data() + data.size() - 4, (char*)&checksum, sizeof(checksum) );

      return fc::to_base58( data.data(), data.size() );
   }

   extended_address::operator extended_public_key()const
   {
      return addr;
   }


} // namespace bts

namespace fc 
{ 
   void to_variant( const bts::extended_address& ext_addr,  fc::variant& vo )
   {
      vo = std::string(ext_addr);
   }

   void from_variant( const fc::variant& var,  bts::extended_address& vo )
   {
      vo = bts::extended_address( var.as_string() );
   }
}
