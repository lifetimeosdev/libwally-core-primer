#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <wally_bip39.h>
#include <wally_bip32.h>
#include <wally_core.h>
#include <wally_crypto.h>

static void print_pk(unsigned char *pk)
{
	size_t lines = 4;
	size_t line_bytes = EC_PRIVATE_KEY_LEN / 4;
	assert(lines * line_bytes == EC_PRIVATE_KEY_LEN);

	for (size_t i = 0; i < lines; i++) {
		if (i == 0) {
			printf("pk:\n(%ld). 0x", i + 1);
		} else {
			printf("(%ld). ", i + 1);
		}
		for (size_t j = i * line_bytes; j < (i + 1) * line_bytes; j++) {
			printf("%02x", pk[j]);
		}
		printf("\n");
	}

	fflush(stdout);
}

int main(int argc, char const *argv[])
{
	int ret;
	unsigned char seed[BIP39_SEED_LEN_512];
	char mnemonic[1024] = {0};

	printf("Plese input mnemonic:\n");
	if (fgets(mnemonic, 1024, stdin) == NULL) {
		printf("Read mnemonic error:%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	mnemonic[strcspn(mnemonic, "\r\n")] = 0;

	printf("Your mnemonic is:%s\n", mnemonic);

	char *pass = getpass("Please input passphrase(optional):");
	if (pass && strlen(pass)) {
		char *pass_buf1 = calloc(1, NSS_BUFLEN_PASSWD);
		char *pass_buf2 = calloc(1, NSS_BUFLEN_PASSWD);
		if (!pass_buf1 || !pass_buf2) {
			memset(pass, 0, NSS_BUFLEN_PASSWD);
			fprintf(stderr, "calloc error\n");
			exit(EXIT_FAILURE);
		}
		memcpy(pass_buf1, pass, NSS_BUFLEN_PASSWD);
		pass = getpass("Please input passphrase again:");
		memcpy(pass_buf2, pass, NSS_BUFLEN_PASSWD);
		if (strlen(pass_buf1) != strlen(pass_buf2) || strcmp(pass_buf1, pass_buf2)) {
			memset(pass, 0, NSS_BUFLEN_PASSWD);
			memset(pass_buf1, 0, NSS_BUFLEN_PASSWD);
			memset(pass_buf2, 0, NSS_BUFLEN_PASSWD);
			fprintf(stderr, "Two passphrases don't match.\n");
			exit(EXIT_FAILURE);
		}
		memset(pass_buf1, 0, NSS_BUFLEN_PASSWD);
		memset(pass_buf2, 0, NSS_BUFLEN_PASSWD);
		free(pass_buf1);
		free(pass_buf2);
		printf("Your passphase len is:%ld\n", strlen(pass));
	}

	wally_init(0);

	ret = bip39_mnemonic_validate(NULL, mnemonic);
	if (ret != WALLY_OK) {
		fprintf(stderr, "bip39_mnemonic_validate err:%d\n", ret);
		exit(EXIT_FAILURE);
	}
	ret = bip39_mnemonic_to_seed512(mnemonic, pass, seed, BIP39_SEED_LEN_512);
	if (ret != WALLY_OK) {
		fprintf(stderr, "bip39_mnemonic_to_seed512 err:%d\n", ret);
		exit(EXIT_FAILURE);
	}
	memset(mnemonic, 0, sizeof(mnemonic));
	if (pass) {
		memset(pass, 0, NSS_BUFLEN_PASSWD);
	}

	struct ext_key master_key = {0};
	ret = bip32_key_from_seed(seed, BIP39_SEED_LEN_512, BIP32_VER_MAIN_PRIVATE, 0, &master_key);
	if (ret != WALLY_OK) {
		fprintf(stderr, "bip32_key_from_seed err:%d\n", ret);
		exit(EXIT_FAILURE);
	}
	struct ext_key derived_key = {0};
	ret = bip32_key_from_parent_path_str(&master_key, "m/44'/60'/0'/0/0", 0,
					     BIP32_FLAG_KEY_PRIVATE | BIP32_FLAG_STR_WILDCARD,
					     &derived_key);
	if (ret != WALLY_OK) {
		fprintf(stderr, "bip32_key_from_parent_path_str err:%d\n", ret);
		exit(EXIT_FAILURE);
	}

	print_pk(&derived_key.priv_key[1]);

	memset(&master_key, 0, sizeof(master_key));
	memset(&derived_key, 0, sizeof(master_key));

	return 0;
}
