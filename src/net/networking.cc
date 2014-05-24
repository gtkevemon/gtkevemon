#include <iostream>
#include <openssl/err.h>

#include "util/thread.h"

#include "certificates.h"
#include "networking.h"

NET_NAMESPACE_BEGIN

#if defined(WIN32)

/* ---------------------- Windows Implementation ------------------ */

char const*
strerror (int err_code)
{
  static TCHAR err_msg[1024];
  DWORD ret = FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      0,
      err_code,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPSTR)&err_msg,
      1023,
      0);
  err_msg[ret] = 0;
  return (char const*)&err_msg;
}

bool
init (void)
{
  /* Init winsock. */
  int retval;
  WSADATA wsaData;
  if ((retval = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0)
  {
    std::cerr << "WSAStartup() failed with error: " << Net::strerror(retval)
          << std::endl;
    return false;
  }

  /* Init SSL. */
  ssl_context();

  return true;
}

void
unload (void)
{
  WSACleanup();
}

#endif

/* ---------------------------------------------------------------- */

static Semaphore *ssl_locks;

static void locking_callback(int mode, int type, const char *file, int line)
{
  (void)file; (void)line;

  if (mode & CRYPTO_LOCK)
  {
    ssl_locks[type].wait();
  }
  else
  {
    ssl_locks[type].post();
  }
}

SSL_CTX*
ssl_context (void)
{
    static SSL_CTX* ssl_ctx = 0;
    if (ssl_ctx)
        return ssl_ctx;

    std::cout << "Initializing SSL subsystem..." << std::endl;

    /* Init library and error handler. */
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_crypto_strings();

    //BIO* bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);

    /* Create SSL Context. */
    SSL_CTX* ctx = SSL_CTX_new(SSLv3_client_method());

    /* Load CAs we trust. */

    /* Read certificates from memory. */
    BIO* cert_bio = BIO_new_mem_buf((void*)cert_geotrust, -1);
    X509* x509_geotrust = PEM_read_bio_X509_AUX(cert_bio, 0, 0, 0);
    BIO_free(cert_bio);

    cert_bio = BIO_new_mem_buf((void*)cert_entrust, -1);
    X509* x509_entrust = PEM_read_bio_X509_AUX(cert_bio, 0, 0, 0);
    BIO_free(cert_bio);

    /* Add certificate to store. */
    X509_STORE* store = SSL_CTX_get_cert_store(ctx);
    X509_STORE_add_cert(store, x509_geotrust);
    X509_STORE_add_cert(store, x509_entrust);

#if 0
    /* Read certificate from file. */
    #define CERTFILE "/tmp/certs/root.pem"
    std::cout << "Loading CA Cert " CERTFILE << "..." << std::endl;
    if (!SSL_CTX_load_verify_locations(ctx, CERTFILE, 0))
    {
        std::cout << "Error loading SSL CA certificates!" << std::endl;
        throw Exception("Error loading CA certificates");
    }
#endif

    /* Set verify and trust depth. */
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, 0);
    SSL_CTX_set_verify_depth(ctx, 2);

    ssl_ctx = ctx;

    /* Set up locks so that openssl is thread safe. */
    ssl_locks = new Semaphore[CRYPTO_num_locks()];
    CRYPTO_set_locking_callback(locking_callback);

    return ssl_ctx;
}

/* ---------------------------------------------------------------- */

void
ssl_unload (void)
{
    ERR_free_strings();
    SSL_CTX_free(ssl_context());
}

NET_NAMESPACE_END
