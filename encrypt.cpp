// https://www.cryptopp.com/wiki/Diffie-Hellman
#include <gtest/gtest.h>
#include <iostream>
#include <iomanip>

#include <modes.h>
#include <aes.h>
#include <filters.h>
#include <dh2.h>

class EncryptTests : testing::Test { };

TEST(EncryptTests, Test1)
{
	//////////////////////////////////////////////////////////////////////////
	// Alice

	// Initialize the Diffie-Hellman class with a random prime and base
	AutoSeededRandomPool rngA;
	DH dhA;
	dh.Initialize(rngA, 128);

	// Extract the prime and base. These values could also have been hard coded 
	// in the application
	Integer iPrime = dhA.GetGroupParameters().GetModulus();
	Integer iGenerator = dhA.GetGroupParameters().GetSubgroupGenerator();

	SecByteBlock privA(dhA.PrivateKeyLength());
	SecByteBlock pubA(dhA.PublicKeyLength());
	SecByteBlock secretKeyA(dhA.AgreedValueLength());

	// Generate a pair of integers for Alice. The public integer is forwarded to Bob.
	dhA.GenerateKeyPair(rngA, privA, pubA);

	//////////////////////////////////////////////////////////////////////////
	// Bob

	AutoSeededRandomPool rngB;
	// Initialize the Diffie-Hellman class with the prime and base that Alice generated.
	DH dhB(iPrime, iGenerator);

	SecByteBlock privB(dhB.PrivateKeyLength());
	SecByteBlock pubB(dhB.PublicKeyLength());
	SecByteBlock secretKeyB(dhB.AgreedValueLength());

	// Generate a pair of integers for Bob. The public integer is forwarded to Alice.
	dhB.GenerateKeyPair(rngB, privB, pubB);

	//////////////////////////////////////////////////////////////////////////
	// Agreement

	// Alice calculates the secret key based on her private integer as well as the
	// public integer she received from Bob.
	if (!dhA.Agree(secretKeyA, privA, pubB))
		return false;

	// Bob calculates the secret key based on his private integer as well as the
	// public integer he received from Alice.
	if (!dhB.Agree(secretKeyB, privB, pubA))
		return false;

	// Just a validation check. Did Alice and Bob agree on the same secret key?
	if (VerifyBufsEqualp(secretKeyA.begin(), secretKeyB.begin(), dhA.AgreedValueLength()))
		return false;
	



	//Key and IV setup
	//AES encryption uses a secret key of a variable length (128-bit, 196-bit or 256-   
	//bit). This key is secretly exchanged between two parties before communication   
	//begins. DEFAULT_KEYLENGTH= 16 bytes
	byte key[ CryptoPP::AES::DEFAULT_KEYLENGTH ], iv[ CryptoPP::AES::BLOCKSIZE ];
	memset( key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
	memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

	//
	// String and Sink setup
	//
	std::string plaintext = "Now is the time for all good men to come to the aide...";
	std::string ciphertext;
	std::string decryptedtext;

	//
	// Dump Plain Text
	//
	std::cout << "Plain Text (" << plaintext.size() << " bytes)" << std::endl;
	std::cout << plaintext;
	std::cout << std::endl << std::endl;

	//
	// Create Cipher Text
	//
	CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption( aesEncryption, iv );

	CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink( ciphertext ) );
	stfEncryptor.Put( reinterpret_cast<const unsigned char*>( plaintext.c_str() ), plaintext.length() + 1 );
	stfEncryptor.MessageEnd();

	//
	// Dump Cipher Text
	//
	std::cout << "Cipher Text (" << ciphertext.size() << " bytes)" << std::endl;
	for( int i = 0; i < ciphertext.size(); i++ )
	{
	std::cout << "0x" << std::hex << (0xFF & static_cast<byte>(ciphertext[i])) << " ";
	}
	std::cout << std::endl << std::endl;

	//
	// Decrypt
	//
	CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption( aesDecryption, iv );

	CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink( decryptedtext ) );
	stfDecryptor.Put( reinterpret_cast<const unsigned char*>( ciphertext.c_str() ), ciphertext.size() );
	stfDecryptor.MessageEnd();

	//
	// Dump Decrypted Text
	//
	std::cout << "Decrypted Text: " << std::endl;
	std::cout << decryptedtext;
	std::cout << std::endl << std::endl;
}


/*
#include <iostream>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <assert.h>
#include <random>
#include <limits>
#include <tuple>

namespace crypt {

using bigint = unsigned int;

std::string encrypt_decrypt(const std::string& word, const char* key)
{
	const size_t module = sizeof(key) / sizeof(char);
	std::string output = word;
	for (size_t i = 0; i < word.size(); ++i)
		output[i] = word[i] ^ key[i % module];
	return output;
}

std::string generate_key(bigint seed, const size_t size = 256)
{
	static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWYXZabcdefghijklmnopqrstuvwyxz";
	char buffer[size+1];
	std::minstd_rand0 gen(seed);
	std::uniform_int_distribution<bigint> dist(std::numeric_limits<bigint>::min() , std::numeric_limits<bigint>::max() - 1);
	for(size_t i = 0; i < size; ++i)
	{
		buffer[i] = alphanum[dist(gen) % (sizeof(alphanum) - 1)];
	}
	buffer[size] = '\0';
	return std::string(buffer);
}

bigint pow(bigint base, bigint exp)
{
	return bigint(std::pow(double(base), double(exp)));
}

bigint generate_big_prime()
{
	return 2147483647;
}

}

auto client()
{
	using namespace crypt;

	// seed client
	std::mt19937 g1( 123456789 );
	std::uniform_int_distribution<bigint> dist(std::numeric_limits<bigint>::min() , std::numeric_limits<bigint>::max() - 1);

	bigint a = dist(g1);
	bigint g = dist(g1);
	bigint p = crypt::generate_big_prime();
	bigint A = crypt::pow(g, a) % p;
	return std::make_tuple(a, g, p, A);
}

auto server(crypt::bigint g, crypt::bigint p, crypt::bigint A)
{
	using namespace crypt;

	// seed server
	std::mt19937 g2( 987654321 );
	std::uniform_int_distribution<bigint> dist(std::numeric_limits<bigint>::min() , std::numeric_limits<bigint>::max() - 1);

	bigint b = dist(g2);
	bigint B = crypt::pow(g, b) % p;
	bigint K = crypt::pow(A, b) % p;
	return std::make_tuple(b, B, K);
}

int main2(int argc, const char * argv[])
{
	if(argc != 2)
	{
		std::cerr << "invalid use." << "\n";
		std::cerr << argv[0] << " [text]" << "\n";
		return 1;
	}

	const char* text = argv[1];

	// Diffie-Hellman
	// https://www.cryptopp.com/wiki/Diffie-Hellman
	//
	// a, g, p, A
	auto tpl = client();
	auto a = std::get<0>(tpl);
	auto g = std::get<1>(tpl);
	auto p = std::get<2>(tpl);
	auto A = std::get<3>(tpl);
	//
	// B
	auto tpl2 = server(g, p, A);
	auto B = std::get<1>(tpl2);
	auto K2 = std::get<2>(tpl2);
	auto K1 = crypt::pow(B, a) % p;

	std::string key1 = crypt::generate_key(K1);
	std::string key2 = crypt::generate_key(K2);
	std::cout << "using key: " << key1 << "\n\n";

	std::string encrypted = crypt::encrypt_decrypt(text, key1.c_str());
	std::cout << "Encrypted: " << encrypted << "\n";

	std::string decrypted = crypt::encrypt_decrypt(encrypted, key2.c_str());
	std::cout << "Decrypted: " << decrypted << "\n";

	// check is same key
	assert(K1 == K2);

	return 0;
}
*/




