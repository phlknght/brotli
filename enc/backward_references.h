/* Copyright 2013 Google Inc. All Rights Reserved.

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

// Function to find backward reference copies.

#ifndef BROTLI_ENC_BACKWARD_REFERENCES_H_
#define BROTLI_ENC_BACKWARD_REFERENCES_H_

#include "./hash.h"
#include "./command.h"
#include "./types.h"

namespace brotli {

struct ZopfliNode {
	ZopfliNode(void);

	// best length to get up to this byte (not including this byte itself)
	uint32_t length;
	// distance associated with the length
	uint32_t distance;
	uint32_t distance_code;
	int distance_cache[4];
	// length code associated with the length - usually the same as length,
	// except in case of length-changing dictionary transformation.
	uint32_t length_code;
	// number of literal inserts before this copy
	uint32_t insert_length;
	// smallest cost to get to this byte from the beginning, as found so far
	double cost;
};

// Maintains the smallest 2^k cost difference together with their positions
class StartPosQueue {
public:
	explicit StartPosQueue(int bits)
		: mask_((1u << bits) - 1), q_(1 << bits), idx_(0) {}

	void Clear(void) {
		idx_ = 0;
	}

	void Push(size_t pos, double costdiff);

	size_t size(void) const { return std::min(idx_, mask_ + 1); }

	size_t GetStartPos(size_t k) const {
		return q_[(k + 1 - idx_) & mask_].first;
	}

private:
	const size_t mask_;
	std::vector<std::pair<size_t, double> > q_;
	size_t idx_;
};

struct BackwardReferencesContext
{
	BackwardReferencesContext(void) : queue(3), histogram_literal(256), histogram_cmd(kNumCommandPrefixes), histogram_dist(kNumDistancePrefixes) {}

	// From CreateBackwardReferences:
	std::vector<uint32_t> num_matches;
	std::vector<BackwardMatch> matches;

	// From ZopfliIterate:
	std::vector<ZopfliNode> zopfliNodes;
	StartPosQueue queue;
	std::vector<uint32_t> backwards;
	std::vector<uint32_t> path;

	// From SetFromLiteralCosts:
	std::vector<float> literal_cost;

	// From SetFromCommands:
	std::vector<uint32_t> histogram_literal;
	std::vector<uint32_t> histogram_cmd;
	std::vector<uint32_t> histogram_dist;
	std::vector<double> cost_literal;
};

// "commands" points to the next output command to write to, "*num_commands" is
// initially the total amount of commands output by previous
// CreateBackwardReferences calls, and must be incremented by the amount written
// by this call.
void CreateBackwardReferences(size_t num_bytes,
                              size_t position,
                              bool is_last,
                              const uint8_t* ringbuffer,
                              size_t ringbuffer_mask,
                              const int quality,
                              const int lgwin,
                              Hashers* hashers,
                              int hash_type,
                              int* dist_cache,
                              size_t* last_insert_len,
                              Command* commands,
                              size_t* num_commands,
                              size_t* num_literals,
                              BackwardReferencesContext* ctx);

}  // namespace brotli

#endif  // BROTLI_ENC_BACKWARD_REFERENCES_H_
