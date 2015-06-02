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

#ifndef HTTP_HEADER
#define HTTP_HEADER

#include <vector>
#include <string>
#include <stdint.h>
#include <curl/curl.h>

#include "util/ref_ptr.h"
#include "httpstatus.h"

enum HttpMethod
{
  HTTP_METHOD_GET,
  HTTP_METHOD_POST
};

/* ---------------------------------------------------------------- */

enum HttpState
{
  HTTP_STATE_READY,
  HTTP_STATE_CONNECTING,
  HTTP_STATE_SSL_HANDSHAKE,
  HTTP_STATE_REQUESTING,
  HTTP_STATE_RECEIVING,
  HTTP_STATE_DONE,
  HTTP_STATE_ERROR
};

/* ---------------------------------------------------------------- */

class HttpData;
typedef ref_ptr<HttpData> HttpDataPtr;

class HttpData
{
  protected:
    HttpData (void);

  public:
    HttpStatusCode http_code;
    std::vector<std::string> headers;
    std::vector<char> data;

  public:
    static HttpDataPtr create (void);

    /* This is for debugging purposes. */
    void dump_headers (void);
    void dump_data (void);
};

/* ---------------------------------------------------------------- */

/* Class for very simple requesting of documents over HTTP. */
class Http
{
  private:
    /* User specified connection information. */
    HttpMethod method;
    std::string data;
    std::string host;
    std::string path;
    std::string agent;
    uint16_t port;
    std::vector<std::string> headers;
    std::string proxy;
    uint16_t proxy_port;
    bool use_ssl;

    /* Tracking the HTTP state. */
    HttpState http_state;
    std::size_t bytes_read;
    std::size_t bytes_total;

  private:
    void initialize_defaults (void);
    unsigned int get_uint_from_str (std::string const& str);

  public:
    Http (void);
    Http (std::string const& host, std::string const& path);

    /* Set host and path separately. */
    void set_host (std::string const& host);
    void set_path (std::string const& path);
    /* Set port, defaults to 80. */
    void set_port (uint16_t port);
    /* Set user agent, defaults to VerySimpleHttpRequester. */
    void set_agent (std::string const& path);
    /* Set either GET or POST data. */
    void set_data (HttpMethod method, std::string const& data);
    /* Adds a user defined header, e.g.
     * Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ== */
    void add_header (std::string const& header);
    /* Set HTTP proxy server. */
    void set_proxy (std::string const& address, uint16_t port);
    /* Specifies if SSL should be used. */
    void set_use_ssl (bool use_ssl = true);

    /* Returns the path. */
    std::string const& get_path (void) const;

    /* Information about the progress. */
    std::size_t get_bytes_read (void) const;
    /* Information about the total size. This may be zero! */
    std::size_t get_bytes_total (void) const;

    /* Static callback functions for libcurl */
    static std::size_t data_callback(char * buffer, std::size_t size, std::size_t nmemb, void * combo);
    static std::size_t header_callback(char * buffer, std::size_t size, std::size_t nitems, void * combo);

    /* Request the document. This will block until transfer is completed. */
    HttpDataPtr request (void);
};

/* ---------------------------------------------------------------- */

struct HttpCombo // A combo struct to pass to libcurl's C callback functions
{
  Http * http;
  HttpDataPtr * httpdataptr;
};

/* ---------------------------------------------------------------- */

inline
HttpData::HttpData (void)
{
}

inline HttpDataPtr
HttpData::create (void)
{
  return HttpDataPtr(new HttpData);
}

inline void
Http::set_data (HttpMethod method, std::string const& data)
{
  this->method = method;
  this->data = data;
}

inline void
Http::set_host (std::string const& host)
{
  this->host = host;
}

inline void
Http::set_path (std::string const& path)
{
  this->path = path;
}

inline void
Http::set_port (uint16_t port)
{
  this->port = port;
}

inline void
Http::set_agent (std::string const& agent)
{
  this->agent = agent;
}

inline void
Http::add_header (std::string const& header)
{
  this->headers.push_back(header);
}

inline void
Http::set_proxy (std::string const& address, uint16_t port)
{
  this->proxy = address;
  this->proxy_port = port;
}

inline void
Http::set_use_ssl (bool use_ssl)
{
    this->use_ssl = use_ssl;
}

inline std::string const&
Http::get_path (void) const
{
    return this->path;
}

inline std::size_t
Http::get_bytes_read (void) const
{
  return this->bytes_read;
}

inline std::size_t
Http::get_bytes_total (void) const
{
  return this->bytes_total;
}

#endif /* HTTP_HEADER */
