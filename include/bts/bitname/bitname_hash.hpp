#pragma once

#include <stdint.h>

#include <string>

namespace bts { namespace bitname {

/**
 *  Performs a hash on n that will generate collisions for names
 *  that look similar in some fonts / cases.  For example the 
 *  following strings would  generate collisions:
 * 
 *  GN00B,  6MOO8,  gmoob
 *  rin, njm 
 */
uint64_t  name_hash( const std::string& n );

std::string get_keyhotee_id_skeleton(const std::string& keyhotee_id);

} }
