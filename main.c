#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <wally_bip39.h>
#include <wally_bip32.h>
#include <wally_core.h>
#include <wally_crypto.h>

static void print_pk(unsigned char *pk)
{
	printf("pk: 0x");
	for (size_t i = EC_PRIVATE_KEY_LEN / 2; i < EC_PRIVATE_KEY_LEN; i++) {
		printf("%02x", pk[i]);
	}
	for (size_t i = 0; i < EC_PRIVATE_KEY_LEN / 2; i++) {
		printf("%02x", pk[i]);
	}
	printf("\n");
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
	memset(pass, 0, NSS_BUFLEN_PASSWD);

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
