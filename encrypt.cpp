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

int main(int argc, const char * argv[])
{
	if(argc != 2)
	{
		std::cerr << "invalid use." << "\n";
		std::cerr << argv[0] << " [text]" << "\n";
		return 1;
	}

	const char* text = argv[1];

	// Diffie-Hellman
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
