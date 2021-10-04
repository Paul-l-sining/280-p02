#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "simplified_des.h"

// The values (e.g. 5) in s1_box are the decimal representation of the binary
// value given in the lab writeup
const uint8_t s1_box[16] = { 5, 2, 1, 6, 3, 4, 7, 0, 1, 4, 6, 2, 0, 7, 5, 3 };

const uint8_t s2_box[16] = { 4, 0, 6, 5, 7, 1, 3, 2, 5, 3, 0, 7, 6, 2, 1, 4 };

/**
 * Expands a 6-bit input to 8-bits.
 *
 * @note The input uses only 6 of the 8 available bits. The 2 most significant
 * bits will be ignored.
 *
 * @param input The 6-bit input value
 *
 * @return The expanded version of the input.
 */
uint8_t expand(uint8_t input) {
    // For example: input = 0011 0101 (6 bits); we desire to get 1110 1001 (8 bits)

    // Create left mask to zero out all bits of right value:
    uint8_t left_mask = 15; // left_mask = 0000 1111
    left_mask = left_mask << 4; // left_mask = 1111 0000

    uint8_t left_val = input & left_mask; // left_val = 0011 0000

    uint8_t first_2_val = left_val << 2; // first_2_val = 1100 0000

    // Create right mask to zero out all bits of left value:
    uint8_t right_mask = 15; // right_mask = 0000 1111

    uint8_t  right_val = input & right_mask; // right_val = 0000 0101

    // Now we want to separate out 3,4 digits and make it 4 3 4 3 in the middle
    right_val = right_val << 2; // right_val = 0001 0100

    // do the mask again:
    uint8_t  middle_val = right_val & left_mask; // middle_val = 0001 0000
    middle_val >>= 1; // middle_val = 0000 1000
    // do the mask one more time ! (Swap 3 and 4 position)
    uint8_t middle_left_val = middle_val & left_mask; // middle_left_val = 0001 0000
    middle_left_val >>= 1; // middle_left_val = 0000 1000

    uint8_t  middle_right_val = middle_val & right_mask; // middle_right_val = 0000 0000
    middle_right_val <<= 1;

    // pack position 4 & 3, now it is swapped !
    middle_val = middle_right_val | middle_left_val; // middle_val = 0001 0000

    middle_left_val = middle_val << 1; // middle_left_val = 0010 0000
    middle_right_val = middle_val >> 1; // middle_right_val = 0000 1000


    uint8_t last_2_val = right_val & right_mask; // last_2_val = 0000 0100
    last_2_val >>= 2; // last_2_val = 0000 0001

    // pack all values: first_2_val + middle_left_val + middle_right_val + last_2_val
    input = (first_2_val|middle_left_val)|(middle_right_val|last_2_val);

    return input;
}

/**
 * Uses sboxes to transform the input.
 *
 * @param input The 8-bit value to transform
 *
 * @return The transformed input, which will be 6 bits. The 2 most significant
 *   bits are to be ignored.
 */
uint8_t confuse(uint8_t input) {
    // for example : input = 0111 0100

    // Create left mask to zero out all bits of right value:
    uint8_t left_mask = 15; // left_mask = 0000 1111
    left_mask = left_mask << 4; // left_mask = 1111 0000

    uint8_t left_val = input & left_mask; // left_val = 0111 0000
    left_val >>= 4; // left_val = 0000 0111

    // Create right mask to zero out all bits of left value:
    uint8_t right_mask = 15; // right_mask = 0000 1111

    uint8_t  right_val = input & right_mask; // right_val = 0000 0100

    uint8_t new_left_val = s1_box[left_val]; // new_left_val = s1[0111] = s1[7] = 0 = 0000 0000
    uint8_t new_right_val = s2_box[right_val]; // new_right_val = s2[0100] = s2[4] = 7 = 0000 0111

    new_left_val <<= 3; // new_left_val = 0000 0000

    return new_left_val | new_right_val; // result = 0000 0111 (pack left and right)
}

/**
 * Performs the feistel function, which consists of an expansion step
 * followed by a confusion step.
 *
 * @param input A 6-bit value to transform.
 * @param key An 8-bit key.
 *
 * @return A 6-bit value that has gone through the two major steps.
 */
uint8_t feistel(uint8_t input, uint8_t key) {
    // For example : Input = 0010 0101, Key = 0011 1011, Answer should be: 0010 0110
    input = expand(input);	// Input = 1010 1001
    input = input ^ key;	// Input = 1001 0010
    return confuse(input); // Return = 100 110 --> 0010 0110

}

/**
 * Performs one round in the feistel cipher.
 *
 * @note The input and output use only 12 of the 16 available bits. The 4 most
 * significant bits of both should be ignored.
 *
 * @param A 12-bit input to transform.
 * @param An 8-bit key, used only for this round.
 *
 * @param The 12-bit result of going through the round.
 */
uint16_t feistel_round(uint16_t input, uint8_t key) {
    // For example : input: 0000 1101 0110 0101; key = 0011 1011; output should be: 0000 1001 0101 0011

    uint16_t mask = -1; // right_mask = 1111 1111 1111 1111
    uint16_t right_mask = mask >> 8; // right_mask = 0000 0000 1111 1111
    uint16_t left_mask = mask << 8; // left_mask = 1111 1111 0000 0000

    input <<= 2; // input = 0011 0101 1001 0100
    uint16_t left_val = input & left_mask; // left_val = 0011 0101 0000 0000
    uint16_t right_val = input & right_mask; // right_val = 0000 0000 1001 0100

    // compute L1:
    uint16_t  left_new_val = right_val << 4; // left_new_val = 0000 1001 0100 0000;

    // compute R1:
    left_val >>= 8; // left_val = 0000 0000 0011 0101
    uint8_t l0 = left_val; // l0 = 0011 0101

    uint16_t right_new_val = right_val >> 2; // right_val = 0000 0000 0010 0101
    uint8_t r0 = right_new_val; // r0 = 0010 0101i

    uint8_t result = feistel(r0, key); // result = 0011 1110
    uint8_t r1 = result ^ l0; // right_new_val = 0001 0011
    uint16_t right_new_val_2 = r1;

    return left_new_val | right_new_val_2;

}

/**
 * Generates 8-bit keys for a specific number of rounds of computation.
 * These keys are generated based off of the given 9-bit key.
 *
 * @param original_key The 9-bit key to use as a basis for the round keys.
 * @param num_rounds The number of keys to generate. This must be less than 10.
 *
 * @return An array of 8-bit keys, each of which will be used for one round of
 * 			computation. If num_rounds > 9, NULL is returned.
 */
uint8_t *generate_round_keys(uint16_t original_key, unsigned int num_rounds) {

    if (num_rounds > 9){
        return NULL;
    }

    uint8_t *round_keys = calloc(num_rounds, sizeof(uint8_t));

    uint16_t firstHalf = original_key << 7;
    uint16_t secondHalf = original_key >> 2;

    uint16_t total_key = firstHalf | secondHalf;
    uint16_t mask = ~0;
    mask <<= 8;

    for (int i = 0; i < num_rounds; ++i) {
        uint16_t cur_key = total_key & mask;
        cur_key >>= 8-i;
        uint8_t cur_key_8_bit = cur_key;
        round_keys[i] = cur_key_8_bit;
        mask >>= 1;
    }

    return round_keys;
}

/**
 * Encrypts 12-bits of data using the given number of rounds of the simplified
 * DES encryption algorithm.
 *
 * @note The unencrypted and encrypted data uses only 12 of the 16 available
 * bits. The 4 most significant bits of both should be ignored.
 *
 * @param unencrypted_data The 12-bit block of data to encrypt.
 * @param round_keys The keys for each round of the cipher.
 * @param num_rounds The number of rounds of the cipher to perform.
 *
 * @return The encrypted version of the original 12-bit block. The encrypted
 * 			version will be the same as the unencrypted if the number of
 * 			rounds is less than 1.
 */
uint16_t encrypt(uint16_t unencrypted_data, uint8_t *round_keys, int num_rounds) {
    // TODO: Implement this function
    return unencrypted_data;
}

/**
 * Decrypts 12-bits of data using the given number of rounds of the simplified
 * DES encryption algorithm.
 *
 * @note The unencrypted and encrypted data uses only 12 of the 16 available
 * bits. The 4 most significant bits of both should be ignored.
 *
 * @param encrypted_data The 12-bit block of data to decrypt.
 * @param round_keys The keys for each round of the cipher.
 * @param num_rounds The number of rounds of the cipher to perform.
 *
 * @return The decrypted version of the original 12-bit block. The decrypted
 * 			version will be the same as the encrypted if the number of
 * 			rounds is less than 1.
 */
uint16_t decrypt(uint16_t encrypted_data, uint8_t *round_keys, int num_rounds) {
    // TODO: Implement this function
    return encrypted_data;
}