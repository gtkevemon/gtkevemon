/*
 * This file is part of GtkEveMon.
 *
 * GtkEveMon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NET_DNS_LOOKUP_HEADER
#define NET_DNS_LOOKUP_HEADER

#include "netdefines.h"

NET_NAMESPACE_BEGIN

class DNSLookup
{
  public:
    static struct addrinfo* get_hostname (char const* dnsname);
    static struct addrinfo* get_hostname (std::string const& dnsname);
};

NET_NAMESPACE_END

#endif /* NET_DNS_LOOKUP_HEADER */
