/**********************************************************************
 * Copyright (c) 2014 Pieter Wuille                                   *
 * Distributed under the MIT software license, see the accompanying   *
 * file COPYING or http://www.opensource.org/licenses/mit-license.php.*
 **********************************************************************/

#include "include/secp256k1.h"
#include "util.h"
#include "bench.h"

typedef struct {
    secp256k1_context_t *ctx;
    unsigned char msg[32];
    unsigned char sig[64];
} bench_recover_t;

void bench_recover(void* arg) {
    int i;
    bench_recover_t *data = (bench_recover_t*)arg;
    secp256k1_pubkey_t pubkey;
    unsigned char pubkeyc[33];

    for (i = 0; i < 20000; i++) {
        int j;
        int pubkeylen = 33;
        secp256k1_ecdsa_signature_t sig;
        CHECK(secp256k1_ecdsa_signature_parse_compact(data->ctx, &sig, data->sig, i % 2));
        CHECK(secp256k1_ecdsa_recover(data->ctx, data->msg, &sig, &pubkey));
        CHECK(secp256k1_ec_pubkey_serialize(data->ctx, pubkeyc, &pubkeylen, &pubkey, 1));
        for (j = 0; j < 32; j++) {
            data->sig[j + 32] = data->msg[j];    /* Move former message to S. */
            data->msg[j] = data->sig[j];         /* Move former R to message. */
            data->sig[j] = pubkeyc[j + 1];       /* Move recovered pubkey X coordinate to R (which must be a valid X coordinate). */
        }
    }
}

void bench_recover_setup(void* arg) {
    int i;
    bench_recover_t *data = (bench_recover_t*)arg;

    for (i = 0; i < 32; i++) data->msg[i] = 1 + i;
    for (i = 0; i < 64; i++) data->sig[i] = 65 + i;
}

int main(void) {
    bench_recover_t data;

    data.ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);

    run_benchmark("ecdsa_recover", bench_recover, bench_recover_setup, NULL, &data, 10, 20000);

    secp256k1_context_destroy(data.ctx);
    return 0;
}