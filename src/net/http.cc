#include <cstring> // memset
#include <iostream>
#include <sstream>
#include <string>

#include "util/exception.h"
#include "netdnslookup.h"
#include "netssltcpsocket.h"
#include "http.h"

void
HttpData::dump_headers (void)
{
  std::cout << "==== HTTP Headers ====" << std::endl;
  for (std::size_t i = 0; i < this->headers.size(); ++i)
    std::cout << this->headers[i] << std::endl;
}

/* ---------------------------------------------------------------- */

void
HttpData::dump_data (void)
{
  std::cout << "==== HTTP Data dump ====" << std::endl;
  std::cout.write(&this->data[0], this->data.size());
  std::cout << std::endl;
}

/* ================================================================ */

Http::Http (void)
{
  this->initialize_defaults();
}

/* ---------------------------------------------------------------- */

Http::Http (std::string const& url)
{
  this->set_url(url);
  this->initialize_defaults();
}

/* ---------------------------------------------------------------- */

Http::Http (std::string const& host, std::string const& path)
{
  this->host = host;
  this->path = path;
  this->initialize_defaults();
}

/* ---------------------------------------------------------------- */

void
Http::initialize_defaults (void)
{
  this->method = HTTP_METHOD_GET;
  this->port = 80;
  this->agent = "GtkEveMon HTTP Requester";
  this->proxy_port = 80;
  this->use_ssl = false;

  this->http_state = HTTP_STATE_READY;
  this->bytes_read = 0;
  this->bytes_total = 0;
}

/* ---------------------------------------------------------------- */

void
Http::set_url (std::string const& url)
{
  if (url.size() <= 7)
    throw Exception("Invalid URL");

  if (url.substr(0, 7) != "http://")
    throw Exception("Invalid protocol");

  std::string host = url.substr(7);
  size_t spos = host.find_first_of('/');
  if (spos == std::string::npos)
    throw Exception("No path specification");

  if (spos == 0)
    throw Exception("Invalid host specification");

  std::string path = host.substr(spos);
  host = host.substr(0, spos);

  //std::cout << "Setting host and path: " << host << ":" << path << std::endl;
  this->host = host;
  this->path = path;
}

/* ---------------------------------------------------------------- */

HttpDataPtr
Http::request (void)
{
  this->bytes_read = 0;
  this->bytes_total = 0;

  Net::TCPSocket* sock;
  if (this->use_ssl)
    sock = new Net::SSLTCPSocket();
  else
    sock = new Net::TCPSocket();

  try
  {
    /* Initialize a valid HTTP connection. */
    this->http_state = HTTP_STATE_CONNECTING;
    this->initialize_connection(sock);

    /* If we use SSL and have a proxy, send CONNECT command. */
    if (this->use_ssl && !this->proxy.empty())
       this->send_proxy_connect(sock);

    /* If we use SSL, do the handshake now. */
    if (this->use_ssl)
    {
      this->http_state = HTTP_STATE_SSL_HANDSHAKE;
      dynamic_cast<Net::SSLTCPSocket*>(sock)->check_hostname(this->host);
      dynamic_cast<Net::SSLTCPSocket*>(sock)->ssl_handshake();
    }

    /* Successful connect. Start building the request. */
    this->http_state = HTTP_STATE_REQUESTING;
    this->send_http_headers(sock);

    /* Request has been sent. Read reply. */
    this->http_state = HTTP_STATE_RECEIVING;
    HttpDataPtr result = this->read_http_reply(sock);

    /* Close socket. */
    sock->close();

    /* Done reading the reply. */
    this->http_state = HTTP_STATE_DONE;
    return result;
  }
  catch (Exception& e)
  {
    this->http_state = HTTP_STATE_ERROR;
    if (sock->is_connected())
      sock->close();
    delete sock;
    throw e;
  }
}

/* ---------------------------------------------------------------- */

/* Initializes a valid HTTP connection or throws an exception. */
void
Http::initialize_connection (Net::TCPSocket* sock)
{
  /* Some error checking. */
  if (this->host.size() == 0)
    throw Exception("Internal: Host not set");

  if (this->path.size() == 0)
    throw Exception("Internal: Path not set");

  if (this->proxy.empty())
  {
    /*
     * TODO: Why was this using it's own dns lookup?
     * in_addr_t host_addr = Net::DNSLookup::get_hostname(this->host);
     */
    sock->connect(this->host, this->port);
  }
  else
  {
    sock->connect(this->proxy, this->proxy_port);
  }
}

/* ---------------------------------------------------------------- */

void
Http::send_http_headers (Net::TCPSocket* sock)
{
  std::string request_path(this->path);

  /* Prefix "http://" in case of proxy use (non-SSL only). */
  if (!this->proxy.empty() && !this->use_ssl)
      request_path = "http://" + this->host + this->path;

  /* Create the headers. */
  std::stringstream headers;
  if (this->method == HTTP_METHOD_POST)
    headers << "POST " << request_path << " HTTP/1.1\n";
  else
  {
    headers << "GET " << request_path;
    if (this->data.size() > 0)
      headers << "?" << this->data;
    headers << " HTTP/1.1\n";
  }
  headers << "Host: " << this->host << "\n";
  headers << "User-Agent: " << this->agent << "\n";
  headers << "Connection: close\n";

  /* Append additional headers specified from the outside. */
  for (unsigned int i = 0; i < this->headers.size(); ++i)
    headers << this->headers[i] << "\n";

  if (this->method == HTTP_METHOD_POST)
  {
    headers << "Content-Type: application/x-www-form-urlencoded\n";
    headers << "Content-Length: " << this->data.size() << "\n";
    headers << "\n";
    headers << this->data << "\n";
  }
  else
  {
    headers << "\n";
  }

  std::string header_str = headers.str();
  //std::cout << "Sending: " << header_str << std::endl;

  /* Send the headers. */
  sock->full_write(header_str.c_str(), header_str.size());
}

/* ---------------------------------------------------------------- */

void
Http::send_proxy_connect (Net::TCPSocket* sock)
{
    std::stringstream ss;
    ss << "CONNECT " << this->host << ":"
        << this->port << " HTTP/1.1" << std::endl;
    ss << "User-Agent: " << this->agent << std::endl;
    ss << "Proxy-Connection: close" << std::endl;
    ss << "Host: " << this->host << std::endl;
    ss << std::endl;

    //FIXME: Does not send in plain
    std::cout << "Sending CONNECT headers..." << std::endl;
    std::string headers(ss.str());
    sock->full_write(headers.c_str(), headers.size());

#if 0
CONNECT api.eveonline.com:443 HTTP/1.1
User-Agent: Mozilla/5.0 (X11; Linux i686 on x86_64; rv:2.0) Gecko/20100101 Firefox/4.0
Proxy-Connection: keep-alive
Host: bankingportal.sparkasse-bensheim.de
#endif
}

/* ---------------------------------------------------------------- */

HttpDataPtr
Http::read_http_reply (Net::TCPSocket* sock)
{
  HttpDataPtr result = HttpData::create();
  bool chunked_read = false;

  /* Read the headers. */
  while (true)
  {
    std::string line;
    sock->read_line(line, 1024);

    /* Exit loop if we read an empty line. */
    if (line.empty())
      break;

    result->headers.push_back(line);
    if (line == "Transfer-Encoding: chunked")
      chunked_read = true;
    else if (line.substr(0, 16) == "Content-Length: ")
      this->bytes_total = (size_t)this->get_uint_from_str(line.substr(16));
  }

  /* Debug dump of HTTP headers. */
  //result->dump_headers();

  /* Determine the HTTP status code. */
  result->http_code = this->get_http_status_code(result->headers[0]);

  /* Read the HTTP reply body. */
  if (chunked_read)
  {
    /* Deal with chunks *sigh*. */
    std::size_t size = 512;
    std::size_t pos = 0;
    result->data.resize(size);

    while (true)
    {
      /* Read line with chunk size information. */
      std::string line;
      sock->read_line(line, 32);
      if (line.empty())
        break;

      /* Convert to number of bytes to be read. */
      std::size_t chunk_size = this->get_uint_from_hex(line);
      if (chunk_size == 0)
        break;

      /* Grow buffer if we run out of space. */
      while (pos + chunk_size >= size)
      {
        size *= 2;
        result->data.resize(size, '\0');
      }

      /* Read bytes. */
      std::size_t nbytes = this->http_data_read(sock,
          &result->data[pos], chunk_size);
      pos += nbytes;
      if (nbytes != chunk_size)
        break;
    }

    result->data.resize(pos);
  }
  else if (this->bytes_total == 0)
  {
    /* The Server did not specify a content-length header. */
    std::size_t size = 512;
    std::size_t pos = 0;

    while (true)
    {
      if (result->data.size() < pos + size)
        result->data.resize(pos + size, '\0');

      std::size_t nbytes = this->http_data_read(sock, &result->data[pos], size);
      if (nbytes == 0)
        break;
      pos += nbytes;
    }
    result->data.resize(pos + 1, '\0');
  }
  else
  {
    /* Simply copy the buffer to the result. Allocating one more byte and
     * setting the memory to '\0' makes it safe for use as string.
     * WARNING: For binary data you should always remove the last character.
     */
    result->data.resize(this->bytes_total + 1);
    ::memset(&result->data[0], '\0', this->bytes_total + 1);
    this->http_data_read(sock, &result->data[0], this->bytes_total);
  }

  return result;
}

/* ---------------------------------------------------------------- */

HttpStatusCode
Http::get_http_status_code (std::string const& header)
{
  /* Find first and second whitespace. */
  std::size_t p1 = header.find_first_of(' ');
  if (p1 == std::string::npos)
    return 999;
  if (p1 == header.size() - 1)
    return 999;
  std::size_t p2 = header.find_first_of(' ', p1 + 1);
  if (p2 == std::string::npos || p2 <= p1)
    return 999;

  /* Extract the code and convert to a numeric type. */
  std::string code_str = header.substr(p1 + 1, p2 - p1 - 1);
  std::stringstream ss(code_str);
  HttpStatusCode code;
  if (!(ss >> code) || !ss.eof())
    return 999;

  return code;
}

/* ---------------------------------------------------------------- */

unsigned int
Http::get_uint_from_hex (std::string const& str)
{
  if (str.empty())
    return 0;

  unsigned int v = 0;
  unsigned int mult = 1;

  for (int i = (int)str.size() - 1; i >= 0; --i)
  {
    switch (str[i])
    {
      case '0': break;
      case '1': v += 1 * mult; break;
      case '2': v += 2 * mult; break;
      case '3': v += 3 * mult; break;
      case '4': v += 4 * mult; break;
      case '5': v += 5 * mult; break;
      case '6': v += 6 * mult; break;
      case '7': v += 7 * mult; break;
      case '8': v += 8 * mult; break;
      case '9': v += 9 * mult; break;
      case 'A': case 'a': v += 10 * mult; break;
      case 'B': case 'b': v += 11 * mult; break;
      case 'C': case 'c': v += 12 * mult; break;
      case 'D': case 'd': v += 13 * mult; break;
      case 'E': case 'e': v += 14 * mult; break;
      case 'F': case 'f': v += 15 * mult; break;
      default: throw Exception("Invalid HEX character");
    }
    mult = mult * 16;
  }

  return v;
}

/* ---------------------------------------------------------------- */

unsigned int
Http::get_uint_from_str (std::string const& str)
{
  std::stringstream ss(str);
  int ret;
  ss >> ret;
  return ret;
}

/* ---------------------------------------------------------------- */

std::size_t
Http::http_data_read (Net::TCPSocket* sock, char* buf, std::size_t size)
{
  std::size_t ret = 0;
  while (ret < size)
  {
    ssize_t new_read = sock->read(buf + ret, size - ret);
    if (new_read == 0)
      break;

    ret += new_read;
    this->bytes_read += new_read;
  }

  return ret;
}
