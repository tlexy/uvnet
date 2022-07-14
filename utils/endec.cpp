#include "endec.h"
#include <openssl/ssl.h>
#include <openssl/des.h>
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/sha.h>


std::string base64Encode(const char* buff, int len)
{
	BIO* bmem = NULL;
	BIO* b64 = NULL;
	BUF_MEM* bptr;

	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, buff, len);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);
	BIO_set_close(b64, BIO_NOCLOSE);

	std::string b64_str(bptr->data, bptr->length);
	BIO_free_all(b64);

	return b64_str;
}

std::string base64Decode(const char *input, int length, bool newLine)
{
	BIO *b64 = NULL;
	BIO *bmem = NULL;
	char *buffer = (char *)malloc(length);
	memset(buffer, 0, length);
	b64 = BIO_new(BIO_f_base64());
	if (!newLine) {
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	}
	bmem = BIO_new_mem_buf(input, length);
	bmem = BIO_push(b64, bmem);
	int len = BIO_read(bmem, buffer, length);
	BIO_free_all(bmem);

	std::string text(buffer, len);

	return text;
}

std::string sha1(const std::string& text)
{
	SHA_CTX sha1_ctx;
	SHA1_Init(&sha1_ctx);
	SHA1_Update(&sha1_ctx, text.c_str(), text.size());

	unsigned char buff[32] = {'\0'};
	SHA1_Final(buff, &sha1_ctx);
	return std::string((const char*)buff, 20);
}