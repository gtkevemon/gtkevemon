#include <ctime>
#include "util/exception.h"
#include "nettcpsocket.h"

TCPSocket::TCPSocket (void)
{
  curl_handle = NULL;
}

/* ---------------------------------------------------------------- */

TCPSocket::TCPSocket (const std::string& host, int port)
{
  curl_handle = NULL;
  connect(host, port);
}

/* ---------------------------------------------------------------- */

TCPSocket::~TCPSocket (void)
{
  curl_easy_cleanup(curl_handle);
}

/* ---------------------------------------------------------------- */

void
TCPSocket::init()
{
  timeout = 0;
  is_connected = false;
  curl_handle = curl_easy_init();
  if (!curl_handle)
    throw Exception("Failed to create CURL handle.");
  curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_CONNECT_ONLY, 1L);
}

/* ---------------------------------------------------------------- */

void
TCPSocket::set_connect_timeout (size_t timeout)
{
  this->timeout = timeout;
}

/* ---------------------------------------------------------------- */

void
TCPSocket::connect (std::string const & host, int port)
{
  CURLcode res;
  
  curl_easy_setopt(curl_handle, CURLOPT_URL, host.c_str());
  curl_easy_setopt(curl_handle, CURLOPT_PORT, (long) port);
  if (timeout)
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, (long) timeout);

  res = curl_easy_perform(curl_handle);
  if (res != CURLE_OK)
    throw Exception(curl_easy_strerror(res));
}

/* ---------------------------------------------------------------- */

size_t
TCPSocket::read (unsigned char * buffer, size_t max_bytes)
{
  CURLcode res;
  size_t bytes_received;

  // This timer could be off by up to one second.
  // Make sure that local timeout is at least 1 second.
  size_t timeout = ((this->timeout / 1000) > 2) ? (this->timeout / 1000) : 2;
  time_t now_time;
  time_t begin_time = time(NULL);
  do {
    now_time = time(NULL);
    res = curl_easy_recv(curl_handle, buffer, max_bytes, &bytes_received);
  } while (res == CURLE_AGAIN && now_time - begin_time < (long) timeout);

  if (res != CURLE_OK)
    throw Exception(curl_easy_strerror(res));

  return bytes_received;
}

/* ---------------------------------------------------------------- */

size_t
TCPSocket::write (const unsigned char * buffer, size_t bytes)
{
  CURLcode res;
  size_t bytes_sent;

  // This timer could be off by up to one second.
  // Make sure that local timeout is at least 1 second.
  size_t timeout = ((this->timeout / 1000) > 2) ? (this->timeout / 1000) : 2;
  time_t now_time;
  time_t begin_time = time(NULL);
  do {
    now_time = time(NULL);
    res = curl_easy_send(curl_handle, buffer, bytes, &bytes_sent);
  } while (res == CURLE_AGAIN && now_time - begin_time < (long) timeout);

  if (res != CURLE_OK)
    throw Exception(curl_easy_strerror(res));

  return bytes_sent;
}
