/**
 * @file
 * @brief Sequence utilities
 *
 * This file contains utility functions for working with DNA sequences.
 */

#include "sequence.h"
#include <algorithm>
#include <cctype>
#include <err.h>
#include <limits.h>
#include <memory>
#include <vector>

void normalize(std::string &) noexcept;

sequence::sequence(std::string name_, std::string nucl_) noexcept
	: name{std::move(name_)},
	  nucl{std::move(nucl_)},
	  length{nucl.size()}
{
	const size_t LENGTH_LIMIT = (INT_MAX - 1) / 2;
	if (this->size() > LENGTH_LIMIT) {
		warnx("The input sequence %s is too long. The technical limit is %zu.",
			  this->name.c_str(), LENGTH_LIMIT);
	}
}

std::string sequence::to_fasta() const
{
	static const ssize_t LINE_LENGTH = 70;

	auto ret = ">" + get_name() + "\n";
	ret.reserve(ret.size() + length + length / LINE_LENGTH);

	auto it = this->begin();
	while (it < this->end()) {
		auto ll = std::min(this->end() - it, LINE_LENGTH);

		ret.append(it, it + ll);
		ret += '\n';

		it += ll;
	}

	return ret;
}

/**
 * @brief Compute the reverse complement.
 * @param base - The master string.
 * @return The reverse complement.
 */
std::string reverse(const std::string &base)
{
	std::string ret{};
	ret.reserve(base.size());

	size_t len = base.size();
	auto str = new char[len + 1];

	for (size_t k = 0; k < len; k++) {
		char c = base[len - k - 1], d;

		if (c < 'A') {
			d = c;
		} else {
			d = c ^= (c & 2) ? 4 : 21; // ACGT
		}

		str[k] = d;
	}

	str[len] = '\0';

	ret.replace(0, len, str);
	delete[] str;

	return ret;
}

std::string filter_nucl(const std::string &base)
{
	std::string ret{};
	ret.reserve(base.size());

	for (auto c : base) {
		switch (c) {
			case 'A':
			case 'C':
			case 'G':
			case 'T': ret += c; break;
			case 'a':
			case 'c':
			case 'g':
			case 't': ret += std::toupper(c); break;
			default: break;
		}
	}

	return ret;
}

double gc_content(const std::string &seq) noexcept
{
	size_t gc = 0;
	size_t length = seq.size();

	for (size_t i = 0; i < length; i++) {
		char masked = seq[i] & 'G' & 'C';
		if (masked == ('G' & 'C')) {
			gc++;
		}
	}

	return static_cast<double>(gc) / length;
}

/**
 *
 */
sequence join(const genome &gen)
{
	auto joined = sequence();
	const auto contigs = gen.get_contigs();

	if (contigs.size() == 0) {
		return joined;
	}

	if (contigs.size() == 1) {
		joined = contigs[0];
		return joined;
	}

	size_t total = gen.get_length();

	auto neu = std::string();
	neu.reserve(total);

	// Copy all old sequences and add a `!` in between
	auto it = contigs.begin();
	neu += it->get_nucl();

	std::for_each(++it, end(contigs), [&neu](const sequence &seq) {
		neu += '!';
		neu += seq.get_nucl();
	});

	return sequence(gen.get_name(), neu);
}