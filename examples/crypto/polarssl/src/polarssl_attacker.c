#include "../annotated_api/polarssl.h"

#include "random.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

//@ #include "quantifiers.gh"

#define POLARSSL_ATTACKER_ITERATIONS 100

/*@

predicate attacker_invariant<T>(real f,
                       predicate(polarssl_cryptogram) pub,
                       predicate(T) pred, T arg,
                       struct havege_state* state, void* socket, int attacker) =
  [f]polarssl_world(pub, pred, arg) &*&
    pred(arg) &*&
    is_polarssl_bad_random_is_public(_, pub, pred, arg) &*&
    is_polarssl_bad_key_is_public(_, pub, pred, arg) &*&
    is_polarssl_public_key_is_public(_, pub, pred, arg) &*&
    is_polarssl_bad_private_key_is_public(_, pub, pred, arg) &*&
    is_polarssl_hash_is_public(_, pub, pred, arg) &*&
    is_polarssl_public_hmac_is_public(_, pub, pred, arg) &*&
    is_polarssl_public_encryption_is_public(_, pub, pred, arg) &*&
    is_polarssl_public_decryption_is_public(_, pub, pred, arg) &*&
    is_polarssl_public_auth_encryption_is_public(_, pub, pred, arg) &*&
    is_polarssl_public_auth_decryption_is_public(_, pub, pred, arg) &*&
    is_polarssl_public_asym_encryption_is_public(_, pub, pred, arg) &*&
    is_polarssl_public_asym_decryption_is_public(_, pub, pred, arg) &*&
    is_polarssl_public_asym_signature_is_public(_, pub, pred, arg) &*&
  true == bad(attacker) &*&
  polarssl_principal(attacker) &*&
  polarssl_generated_values(attacker, ?count) &*&
  havege_state_initialized(state) &*&
  integer(socket, ?fd) &*&
  polarssl_net_status(fd, ?ip, ?port, connected)
;

lemma void assume_chars_contain_public_cryptograms<T>(char* buffer, int size)
  nonghost_callers_only
  requires [?f]polarssl_world<T>(?pub, ?pred, ?arg) &*&
           chars(buffer, size, ?cs);
  ensures  [f]polarssl_world<T>(pub, pred, arg) &*&
           polarssl_public_message(pub)(buffer, size, cs);
{
  polarssl_generated_public_cryptograms_assume(pub, cs);
  close polarssl_public_message(pub)(buffer, size, cs);
}

@*/

void polarssl_attacker_send_data/*@ <T> @*/(havege_state *havege_state, 
                                            void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size;
  char buffer[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  
  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
  //@ assert pred(arg);
  r_int_with_bounds(havege_state, &temp, 1, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size = temp;
  //@ chars_split(buffer, size);
  //@ assert chars(buffer, size, ?cs);
  //@ assume_chars_contain_public_cryptograms(buffer, size);
  net_send(socket, buffer, (unsigned int) size);
  //@ open polarssl_public_message(pub)(buffer, _, _);
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_concatenation/*@ <T> @*/(havege_state *havege_state, 
                                                     void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int size1;
  int size2;
  char buffer1[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer2[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer3[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
  size1 = net_recv(socket, buffer1, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size2 = net_recv(socket, buffer2, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size1 > 0 && size2 > 0)
  {
    //@ open polarssl_public_message(pub)(buffer1, size1, ?cs1);
    //@ open polarssl_public_message(pub)(buffer2, size2, ?cs2);
    if (POLARSSL_MAX_MESSAGE_BYTE_SIZE - size1 > size2)
    {
      memcpy(buffer3, buffer1, (unsigned int) size1);
      memcpy((char*) buffer3 + size1, buffer2, (unsigned int) size2);
      /*@ polarssl_cryptograms_in_chars_upper_bound_join(
                           cs1, polarssl_generated_public_cryptograms(pub),
                           cs2, polarssl_generated_public_cryptograms(pub)); @*/
      //@ union_refl(polarssl_generated_public_cryptograms(pub));
      /*@ close polarssl_public_message(pub)(buffer3, size1 + size2, 
                                             append(cs1, cs2)); @*/
      net_send(socket, buffer3, (unsigned int) (size1 + size2));
      //@ open polarssl_public_message(pub)(buffer3, _, _);
    }
  }
  /*@ if (size1 > 0 && size2 <= 0) 
        open polarssl_public_message(pub)(buffer1, _, _); @*/
  /*@ if (size2 > 0 && size1 <= 0) 
        open polarssl_public_message(pub)(buffer2, _, _); @*/
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_split/*@ <T> @*/(havege_state *havege_state, 
                                             void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size1;
  int size2;
  char buffer[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
  size1 = net_recv(socket, buffer, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size1 > 0)
  {
    //@ open polarssl_public_message(pub)(buffer, size1, ?cs);
    r_int_with_bounds(havege_state, &temp, 0, size1);
    size2 = temp;
    //@ chars_split(buffer, size2);
    /*@ polarssl_cryptograms_in_chars_upper_bound_split(
                        cs, polarssl_generated_public_cryptograms(pub), size2); @*/
    //@ close polarssl_public_message(pub)(buffer, size2, take(size2, cs));
    net_send(socket, buffer, (unsigned int) (size2));
    /*@ close polarssl_public_message(pub)((void*)buffer + size2, size1 - size2, 
                                           drop(size2, cs)); @*/
    net_send(socket, (void*) buffer + size2, 
              (unsigned int) (size1 - size2));
    //@ open polarssl_public_message(pub)(buffer, _, _);
    //@ open polarssl_public_message(pub)((void*) buffer + size2, _, _);
    //@ chars_join(buffer);
  }
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_random/*@ <T> @*/(havege_state *havege_state, 
                                              void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size;
  char buffer[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
  r_int_with_bounds(havege_state, &temp, POLARSSL_MIN_RANDOM_BYTE_SIZE, 
                                         POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size = temp;
  r_int_with_bounds(havege_state, &temp, 0, INT_MAX);
  //@ close random_request(attacker, temp, false);
  if (havege_random(havege_state, buffer, (unsigned int) size) == 0)
  {
    //@ assert polarssl_cryptogram(buffer, size, ?cs, ?cg);
    //@ assert is_polarssl_bad_random_is_public(?proof, pub, pred, arg);
    //@ proof(cg);
    //@ polarssl_public_message_from_cryptogram(pub, buffer, size, cs, cg);
    net_send(socket, buffer, (unsigned int) size);
    //@ open polarssl_public_message(pub)(buffer, _, _);
    //@ chars_join(buffer);
  }
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

int attacker_key_item_havege_random_stub(void *havege_state, 
                                         char *output, size_t len)
  /*@ requires [?f]havege_state_initialized(havege_state) &*&
               random_request(?principal, ?info, ?key_request) &*&
               polarssl_generated_values(principal, ?count) &*&
               chars(output, len, _) &*& len >= POLARSSL_MIN_RANDOM_BYTE_SIZE;
  @*/
  /*@ ensures  [f]havege_state_initialized(havege_state) &*&
               polarssl_generated_values(principal, count + 1) &*&
               result == 0 ?
                 polarssl_cryptogram(output, len, ?cs, ?cg) &*&
                 info == polarssl_cryptogram_info(cg) &*&
                 key_request ?
                   cg == polarssl_symmetric_key(principal, count + 1)
                 :
                   cg == polarssl_random(principal, count + 1)
               :
                 chars(output, len, _);
  @*/
{
  return havege_random(havege_state, output, len);
}

void polarssl_attacker_send_keys/*@ <T> @*/(havege_state *havege_state, 
                                            void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  pk_context context;
  pk_context context_pub;
  pk_context context_priv;
  unsigned int key_size;
  
  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
  
  unsigned int temp;
  r_u_int_with_bounds(havege_state, &temp, 1024, 8192);
  key_size = temp;
  char* key = malloc((int) key_size);
  if ((key) == 0) abort();
  char* pub_key = malloc((int) key_size);
  if ((pub_key) == 0) abort();
  char* priv_key = malloc((int) key_size);
  if ((priv_key) == 0) abort();
  
  
  //@ close random_request(attacker, temp, true);
  if (havege_random(havege_state, key, key_size) != 0) abort();
  
  //@ close pk_context(&context);
  pk_init(&context);
  //@ close pk_context(&context_pub);
  pk_init(&context_pub);
  //@ close pk_context(&context_priv);
  pk_init(&context_priv);
  
  if (pk_init_ctx(&context, pk_info_from_type(POLARSSL_PK_RSA_TYPE)) != 0) 
    abort();
  //@ close rsa_key_request(attacker, 0);
  //@ close random_function_predicate(havege_state_initialized);
  /*@ produce_function_pointer_chunk random_function(
                      attacker_key_item_havege_random_stub)
                     (havege_state_initialized)(state, out, len) { call(); } @*/
  if (rsa_gen_key(context.pk_ctx, attacker_key_item_havege_random_stub, 
                  havege_state, key_size, 65537) != 0) abort();
  
  if (pk_write_pubkey_pem(&context, pub_key, key_size) != 0) abort();
  //@ open polarssl_cryptogram(pub_key, key_size, ?pub_key_cs, ?pub_key_cg);
  //@ close exists(pub_key_cg);
  if (pk_write_key_pem(&context, priv_key, key_size) != 0) abort();
  //@ open polarssl_cryptogram(priv_key, key_size, ?priv_key_cs, ?priv_key_cg);
  //@ close exists(priv_key_cg);
  if (pk_parse_public_key(&context_pub, pub_key, key_size) != 0) abort();
  if (pk_parse_key(&context_priv, priv_key, key_size, NULL, 0) != 0) abort();
  
  //@ assert is_polarssl_bad_key_is_public(?proof1, pub, pred, arg);
  //@ assert polarssl_cryptogram(key, key_size, ?key_cs, ?key_cg);
  //@ proof1(key_cg);
  //@ polarssl_public_message_from_cryptogram(pub, key, key_size, key_cs, key_cg);
  net_send(socket, key, key_size);
  //@ open polarssl_public_message(pub)(key, _, _);
    
  //@ assert is_polarssl_public_key_is_public(?proof2, pub, pred, arg);
  //@ proof2(pub_key_cg);
  //@ close polarssl_cryptogram(pub_key, key_size, pub_key_cs, pub_key_cg);
  /*@ polarssl_public_message_from_cryptogram(pub, pub_key, key_size, 
                                              pub_key_cs, pub_key_cg); @*/
  net_send(socket, pub_key, key_size);
  //@ open polarssl_public_message(pub)(pub_key, _, _);
  
  //@ assert is_polarssl_bad_private_key_is_public(?proof3, pub, pred, arg);
  //@ proof3(priv_key_cg);
  //@ close polarssl_cryptogram(priv_key, key_size, priv_key_cs, priv_key_cg);
  /*@ polarssl_public_message_from_cryptogram(pub, priv_key, key_size, 
                                              priv_key_cs, priv_key_cg); @*/
  net_send(socket, priv_key, key_size);
  //@ open polarssl_public_message(pub)(priv_key, _, _);
  
  //@ pk_release_context_with_keys(&context);
  pk_free(&context);
  //@ open pk_context(&context);
  //@ pk_release_context_with_key(&context_pub);
  pk_free(&context_pub);
  //@ open pk_context(&context_pub);
  //@ pk_release_context_with_key(&context_priv);
  pk_free(&context_priv);
  //@ open pk_context(&context_priv);
  free(key);
  free(pub_key);
  free(priv_key);
  
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_hash/*@ <T> @*/(havege_state *havege_state, 
                                            void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size;
  char buffer[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);

  size = net_recv(socket, buffer, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size >= POLARSSL_MIN_HMAC_INPUT_BYTE_SIZE)
  {
    char hash[64];
    //@ open polarssl_public_message(pub)(buffer, size, ?pay);
    sha512(buffer, (unsigned int) size, hash, 0);
    //@ assert polarssl_cryptogram(hash, 64, ?h_cs, ?h_cg);
    //@ assert h_cg == polarssl_hash(pay);
    //@ assert is_polarssl_hash_is_public(?proof, pub, pred, arg);
    //@ proof(h_cg, polarssl_generated_public_cryptograms(pub));
    //@ close polarssl_public_message(pub)(buffer, size, pay);
    //@ polarssl_public_message_from_cryptogram(pub, hash, 64, h_cs, h_cg);
    net_send(socket, hash, 64);
    //@ open polarssl_public_message(pub)(hash, 64, _);
  }
  //@ if (size > 0) open polarssl_public_message(pub)(buffer, _, _);
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_hmac/*@ <T> @*/(havege_state *havege_state, 
                                            void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size1;
  int size2;
  char buffer1[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer2[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer3[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);

  size1 = net_recv(socket, buffer1, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size2 = net_recv(socket, buffer2, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size1 > 0 && size2 >= POLARSSL_MIN_HMAC_INPUT_BYTE_SIZE)
  {
    //@ open polarssl_public_message(pub)(buffer1, size1, ?cs1);
    //@ chars_split(buffer3, 64);
    //@ int p;
    //@ int c;
    //@ polarssl_cryptogram cg = polarssl_symmetric_key(p, c);
    //@ close exists<polarssl_cryptogram>(cg);
    //@ open polarssl_public_message(pub)(buffer2, size2, ?cs2);
    sha512_hmac(buffer1, (unsigned int) size1, buffer2, 
                (unsigned int) size2, buffer3, 0);
    /*@ switch(cg)
        {
          case polarssl_random(p0, c0):
            assume_chars_contain_public_cryptograms(buffer3, 64);
          case polarssl_symmetric_key(p0, c0):
            if (cs1 == polarssl_chars_for_cryptogram(cg))
            {
              assert polarssl_cryptogram(buffer3, 64, ?cs_hmac, ?hmac);
              polarssl_cryptogram_in_upper_bound(
                              cs1, cg, polarssl_generated_public_cryptograms(pub));
              polarssl_generated_public_cryptograms_from(pub, cg);
              assert [_]pub(cg);
              assert is_polarssl_public_hmac_is_public(?proof2, pub, pred, arg);
              proof2(hmac, polarssl_generated_public_cryptograms(pub));
              polarssl_public_message_from_cryptogram(pub, buffer3, 64, cs_hmac, hmac);
            }
            else
            {
              assume_chars_contain_public_cryptograms(buffer3, 64);
            }
          case polarssl_public_key(p0, c0):
            assume_chars_contain_public_cryptograms(buffer3, 64);
          case polarssl_private_key(p0, c0):
            assume_chars_contain_public_cryptograms(buffer3, 64);
          case polarssl_hash(cs0):
            assume_chars_contain_public_cryptograms(buffer3, 64);
          case polarssl_hmac(p0, c0, cs0):
            assume_chars_contain_public_cryptograms(buffer3, 64);
          case polarssl_encrypted(p0, c0, cs0, ent0):
            assume_chars_contain_public_cryptograms(buffer3, 64);
          case polarssl_auth_encrypted(p0, c0, mac0, cs0, ent0):
            assume_chars_contain_public_cryptograms(buffer3, 64);
          case polarssl_asym_encrypted(p0, c0, cs0, ent0):
            assume_chars_contain_public_cryptograms(buffer3, 64);
          case polarssl_asym_signature(p0, c0, cs0, ent0):
            assume_chars_contain_public_cryptograms(buffer3, 64);
        }
    @*/
    net_send(socket, buffer3, 64);
    //@ open polarssl_public_message(pub)(buffer3, _, _);
  }
  /*@ if (size1 > 0 && size2 < POLARSSL_MIN_HMAC_INPUT_BYTE_SIZE) 
    open polarssl_public_message(pub)(buffer1, _, _); @*/
  /*@ if (size2 > 0 && (size1 <= 0 || size2 < POLARSSL_MIN_HMAC_INPUT_BYTE_SIZE)) 
    open polarssl_public_message(pub)(buffer2, _, _); @*/
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_encrypted/*@ <T> @*/(havege_state *havege_state, 
                                                 void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size1;
  int size2;
  char buffer1[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer2[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer3[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);

  size1 = net_recv(socket, buffer1, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size2 = net_recv(socket, buffer2, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size1 > 0 && size2 >= POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)
  {
    //@ open polarssl_public_message(pub)(buffer1, size1, ?cs1);
    //@ open polarssl_public_message(pub)(buffer2, size2, ?cs2);
    if (size1 == 16 || size1 == 24 || size1 == 32)
    {
      aes_context aes_context;
      //@ close aes_context(&aes_context);
      //@ int p0;
      //@ int id0;
      //@ polarssl_cryptogram cg_key = polarssl_symmetric_key(p0, id0);
      //@ close exists<polarssl_cryptogram>(cg_key);
      if (aes_setkey_enc(&aes_context, buffer1, (unsigned int) size1 * 8) == 0)
      {
        //@ assert aes_context_initialized(&aes_context, ?key_id);
        char iv[16];
        size_t iv_off;
        r_u_int_with_bounds(havege_state, &iv_off, 0, 15);
        //@ assert u_integer(&iv_off, ?iv_off_val);
        //@ close random_request(attacker, 0, false);
        if (havege_random(havege_state, iv, 16) == 0)
        {
          //@ open polarssl_cryptogram(iv, 16, _, _);
          //@ assert chars(iv, 16, ?cs_iv);
          if (aes_crypt_cfb128(&aes_context, POLARSSL_AES_ENCRYPT, 
                               (unsigned int) size2, &iv_off, iv, buffer2, 
                               buffer3) == 0)
          {
            /*@ switch (key_id)
                {
                  case some(pair):
                    switch (pair)
                    {
                      case pair(p1, id1):
                        assert cs1 == polarssl_chars_for_cryptogram(cg_key);
                        assert polarssl_cryptogram(buffer3, size2, 
                                                   ?cs_enc, ?cg_enc);
                        assert cg_enc == polarssl_encrypted(p1, id1, cs2,
                                       append(chars_of_int(iv_off_val), cs_iv));
                        polarssl_cryptogram_in_upper_bound(
                          cs1, cg_key, polarssl_generated_public_cryptograms(pub));
                        polarssl_generated_public_cryptograms_from(pub, cg_key);
                        assert [_]pub(cg_key);
                        assert is_polarssl_public_encryption_is_public(
                                                       ?proof2, pub, pred, arg);
                        proof2(cg_enc, polarssl_generated_public_cryptograms(pub));
                        polarssl_public_message_from_cryptogram(pub, buffer3, size2, cs_enc, cg_enc);
                    }
                  case none:
                    assume_chars_contain_public_cryptograms(buffer3, size2);
                }
            @*/
            net_send(socket, buffer3, (unsigned int) size2);
            //@ open polarssl_public_message(pub)(buffer3, _, _);
          }
        }
        aes_free(&aes_context);
      }
      //@ open aes_context(&aes_context);
    }
  }
  /*@ if (size1 > 0 && size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE) 
    open polarssl_public_message(pub)(buffer1, _, _); @*/
  /*@ if (size2 > 0 && (size1 <= 0 || size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)) 
    open polarssl_public_message(pub)(buffer2, _, _); @*/
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_decrypted/*@ <T> @*/(havege_state *havege_state, 
                                                 void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size1;
  int size2;
  char buffer1[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer2[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer3[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);

  size1 = net_recv(socket, buffer1, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size2 = net_recv(socket, buffer2, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size1 > 0 && size2 >= POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)
  {
    //@ open polarssl_public_message(pub)(buffer1, size1, ?cs1);
    //@ open polarssl_public_message(pub)(buffer2, size2, ?cs2);
    if (size1 == 16 || size1 == 24 || size1 == 32)
    {
      aes_context aes_context;
      //@ close aes_context(&aes_context);
      //@ int p0;
      //@ int id0;
      //@ polarssl_cryptogram cg_key = polarssl_symmetric_key(p0, id0);
      //@ close exists<polarssl_cryptogram>(cg_key);
      if (aes_setkey_enc(&aes_context, buffer1, (unsigned int) size1 * 8) == 0)
      {
        //@ assert aes_context_initialized(&aes_context, ?key_id);
        char iv[16];
        size_t iv_off;
        r_u_int_with_bounds(havege_state, &iv_off, 0, 15);
        //@ assert u_integer(&iv_off, ?iv_off_val);
        //@ close random_request(attacker, 0, false);
        if (havege_random(havege_state, iv, 16) == 0)
        {
          //@ open polarssl_cryptogram(iv, 16, _, _);
          //@ assert chars(iv, 16, ?cs_iv);
          if (aes_crypt_cfb128(&aes_context, POLARSSL_AES_DECRYPT, 
                               (unsigned int) size2, &iv_off, iv, buffer2, 
                               buffer3) == 0)
          {
            /*@ switch (key_id)
                {
                  case some(pair):
                    switch (pair)
                    {
                      case pair(p1, id1):
                        assert chars(buffer3, size2, ?cs_output);
                        assert cs1 == polarssl_chars_for_cryptogram(cg_key);
                        polarssl_cryptogram_in_upper_bound(cs1, cg_key,
                                    polarssl_generated_public_cryptograms(pub));
                        polarssl_generated_public_cryptograms_from(pub, cg_key);
                        assert [_]pub(cg_key);

                        polarssl_cryptogram cg_enc =
                                     polarssl_encrypted(p1, id1, cs_output,
                                       append(chars_of_int(iv_off_val), cs_iv));
                        assert cs2 == polarssl_chars_for_cryptogram(cg_enc);
                        polarssl_cryptogram_in_upper_bound(cs2, cg_enc,
                                    polarssl_generated_public_cryptograms(pub));
                        polarssl_generated_public_cryptograms_from(pub, cg_enc);
                        assert [_]pub(cg_enc);
                        
                        assert is_polarssl_public_decryption_is_public(
                                                       ?proof3, pub, pred, arg);
                        proof3(cg_key, cg_enc);
                        polarssl_public_message_from_chars(pub, buffer3, 
                                                           size2, cs_output);
                    }
                  case none:
                    assume_chars_contain_public_cryptograms(buffer3, size2);
                }
            @*/
            net_send(socket, buffer3, (unsigned int) size2);
            //@ open polarssl_public_message(pub)(buffer3, _, _);
          }
        }
        aes_free(&aes_context);
      }
      //@ open aes_context(&aes_context);
    }
  }
  /*@ if (size1 > 0 && size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE) 
    open polarssl_public_message(pub)(buffer1, _, _); @*/
  /*@ if (size2 > 0 && (size1 <= 0 || size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)) 
    open polarssl_public_message(pub)(buffer2, _, _); @*/
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_auth_encrypted/*@ <T> @*/(havege_state *havege_state, 
                                                      void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size1;
  int size2;
  char buffer1[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer2[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer3[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);

  size1 = net_recv(socket, buffer1, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size2 = net_recv(socket, buffer2, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size1 > 0 && size2 >= POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)
  {
    //@ open polarssl_public_message(pub)(buffer1, size1, ?cs1);
    //@ open polarssl_public_message(pub)(buffer2, size2, ?cs2);
    if (size1 == 16 || size1 == 24 || size1 == 32)
    {
      gcm_context gcm_context;
      //@ close gcm_context(&gcm_context);
      //@ int p0;
      //@ int id0;
      //@ polarssl_cryptogram cg_key = polarssl_symmetric_key(p0, id0);
      //@ close exists<polarssl_cryptogram>(cg_key);
      if (gcm_init(&gcm_context, POLARSSL_AES_CIPHER_ID, 
          buffer1, (unsigned int) size1 * 8) == 0)
      {
        //@ assert gcm_context_initialized(&gcm_context, ?key_id);
        char iv[16];
        //@ close random_request(attacker, 0, false);
        if (havege_random(havege_state, iv, 16) == 0)
        {
          char mac[16];
          //@ open polarssl_cryptogram(iv, 16, _, _);
          //@ assert chars(iv, 16, ?cs_iv);
          if (gcm_crypt_and_tag(&gcm_context, POLARSSL_GCM_ENCRYPT, 
                                (unsigned int) size2, iv, 16, NULL, 0, buffer2, 
                                buffer3, 16, mac) == 0)
          {
            /*@ switch (key_id)
                {
                  case some(pair):
                    switch (pair)
                    {
                      case pair(p1, id1):
                        assert cs1 == polarssl_chars_for_cryptogram(cg_key);
                        polarssl_cryptogram_in_upper_bound(
                          cs1, cg_key, polarssl_generated_public_cryptograms(pub));
                        polarssl_generated_public_cryptograms_from(pub, cg_key);
                        assert [_]pub(cg_key);
                        
                        assert polarssl_cryptogram(
                                              buffer3, size2, ?cs_enc, ?cg_enc);
                        assert chars(mac, 16, ?mac_cs);
                        assert cg_enc == polarssl_auth_encrypted(
                                                   p1, id1, mac_cs, cs2, cs_iv);
                        assert is_polarssl_public_auth_encryption_is_public(
                                                       ?proof1, pub, pred, arg);
                        proof1(cg_enc, polarssl_generated_public_cryptograms(pub));
                        polarssl_public_message_from_cryptogram(pub, buffer3, size2, 
                                                                cs_enc, cg_enc);
                    }
                  case none:
                    assume_chars_contain_public_cryptograms(buffer3, size2);
                }
            @*/
            net_send(socket, buffer3, (unsigned int) size2);
            //@ open polarssl_public_message(pub)(buffer3, _, _);
          }
        }
        gcm_free(&gcm_context);
      }
      //@ open gcm_context(&gcm_context);
    }
  }
  /*@ if (size1 > 0 && size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE) 
    open polarssl_public_message(pub)(buffer1, _, _); @*/
  /*@ if (size2 > 0 && (size1 <= 0 || size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)) 
    open polarssl_public_message(pub)(buffer2, _, _); @*/
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_auth_decrypted/*@ <T> @*/(havege_state *havege_state, 
                                                      void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size1;
  int size2;
  char buffer1[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer2[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer3[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);

  size1 = net_recv(socket, buffer1, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size2 = net_recv(socket, buffer2, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size1 > 0 && size2 >= POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)
  {
    //@ open polarssl_public_message(pub)(buffer1, size1, ?cs1);
    //@ open polarssl_public_message(pub)(buffer2, size2, ?cs2);
    if (size1 == 16 || size1 == 24 || size1 == 32)
    {
      gcm_context gcm_context;
      //@ close gcm_context(&gcm_context);
      //@ int p0;
      //@ int id0;
      //@ polarssl_cryptogram cg_key = polarssl_symmetric_key(p0, id0);
      //@ close exists<polarssl_cryptogram>(cg_key);
      if (gcm_init(&gcm_context, POLARSSL_AES_CIPHER_ID, 
          buffer1, (unsigned int) size1 * 8) == 0)
      {
        //@ assert gcm_context_initialized(&gcm_context, ?key_id);
        char iv[16];
        char tag[16];
        //@ close random_request(attacker, 0, false);
        if (havege_random(havege_state, iv, 16) == 0)
        {
          //@ open polarssl_cryptogram(iv, 16, _, _);
          //@ assert chars(iv, 16, ?cs_iv);
          //@ close random_request(attacker, 0, false);
          if (havege_random(havege_state, tag, 16) == 0)
          {
            //@ open polarssl_cryptogram(tag, 16, _, _);
            //@ assert chars(tag, 16, ?cs_mac);
            if (gcm_auth_decrypt(&gcm_context, (unsigned int) size2,
                                 iv, 16, NULL, 0, tag, 16,
                                 buffer2, buffer3) == 0)
            {
              /*@ switch (key_id)
                  {
                    case some(pair):
                      switch (pair)
                      {
                        case pair(p1, id1):
                          assert chars(buffer3, size2, ?cs_output);
                          assert cs1 == polarssl_chars_for_cryptogram(cg_key);
                          polarssl_cryptogram_in_upper_bound(
                                       cs1, cg_key, 
                                       polarssl_generated_public_cryptograms(pub));
                          polarssl_generated_public_cryptograms_from(pub, cg_key);
                          assert [_]pub(cg_key);

                          polarssl_cryptogram cg_enc =
                                  polarssl_auth_encrypted(
                                    p1, id1, cs_mac, cs_output, cs_iv);
                          assert cs2 == polarssl_chars_for_cryptogram(cg_enc);
                          polarssl_cryptogram_constraints(cs2, cg_enc);
                          polarssl_cryptogram_in_upper_bound(
                                       cs2, cg_enc, 
                                       polarssl_generated_public_cryptograms(pub));
                          polarssl_generated_public_cryptograms_from(pub, cg_enc);
                          assert [_]pub(cg_enc);

                          assert is_polarssl_public_auth_decryption_is_public(
                                                        ?proof, pub, pred, arg);
                          proof(cg_key, cg_enc);
                          polarssl_public_message_from_chars(pub, buffer3, size2, 
                                                             cs_output);
                      }
                    case none:
                      assert false;
                  }
              @*/
              net_send(socket, buffer3, (unsigned int) size2);
              //@ open polarssl_public_message(pub)(buffer3, _, _);
            }
          }
        }
        gcm_free(&gcm_context);
      }
      //@ open gcm_context(&gcm_context);
    }
  }
  /*@ if (size1 > 0 && size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE) 
    open polarssl_public_message(pub)(buffer1, _, _); @*/
  /*@ if (size2 > 0 && (size1 <= 0 || size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)) 
    open polarssl_public_message(pub)(buffer2, _, _); @*/
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_asym_encrypted/*@ <T> @*/(
                                       havege_state *havege_state, void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size1;
  int size2;
  unsigned int osize;
  char buffer1[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer2[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer3[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);

  size1 = net_recv(socket, buffer1, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size2 = net_recv(socket, buffer2, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size1 > 0 && size2 >= POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)
  {
    //@ open polarssl_public_message(pub)(buffer1, size1, ?cs1);
    //@ open polarssl_public_message(pub)(buffer2, size2, ?cs2);
    
    pk_context context;
    //@ close pk_context(&context);
    pk_init(&context);
    //@ int p0;
    //@ int id0;
    //@ polarssl_cryptogram cg_key = polarssl_public_key(p0, id0);
    //@ close exists<polarssl_cryptogram>(cg_key);
    if (pk_parse_public_key(&context, buffer1, (unsigned int) size1) == 0)
    {
      if (size2 * 8 <= size1)
      {
        //@ assert pk_context_with_key(&context, pk_public, ?key_len, ?key_id);
        //@ close random_function_predicate(havege_state_initialized);
        /*@ produce_function_pointer_chunk random_function(
                     attacker_key_item_havege_random_stub)
                    (havege_state_initialized)(state, out, len) { call(); } @*/
        if (pk_encrypt(&context, buffer2, (unsigned int) size2, 
                      buffer3, &osize, POLARSSL_MAX_MESSAGE_BYTE_SIZE,
                      attacker_key_item_havege_random_stub, havege_state) == 0)
        {        
          //@ assert u_integer(&osize, ?osize_val);
          /*@ switch (key_id)
              {
                case some(pair):
                  switch (pair)
                  {
                    case pair(p1, id1):
                      assert size1 == key_len;
                      assert cs1 == polarssl_chars_for_cryptogram(cg_key);
                      polarssl_cryptogram_in_upper_bound(
                          cs1, cg_key, polarssl_generated_public_cryptograms(pub));
                      polarssl_generated_public_cryptograms_from(pub, cg_key);
                      assert [_]pub(cg_key);
                      
                      assert polarssl_cryptogram(buffer3, osize_val, 
                                                 ?cs_enc, ?cg_enc);
                      assert cg_enc == polarssl_asym_encrypted(
                                                 p1, id1, cs2, _);
                      assert is_polarssl_public_asym_encryption_is_public(
                                                 ?proof, pub, pred, arg);
                      proof(cg_enc, polarssl_generated_public_cryptograms(pub));
                      assert [_]pub(cg_enc);
                      polarssl_public_message_from_cryptogram(
                                          pub, buffer3, osize_val, cs_enc, cg_enc);
                  }
                case none:
                  assume_chars_contain_public_cryptograms(buffer3, osize_val);
              }
          @*/
          net_send(socket, buffer3, osize);
          //@ open polarssl_public_message(pub)(buffer3, _, _);
        }
      }
      //@ pk_release_context_with_key(&context);
    }
    pk_free(&context);
    //@ open pk_context(&context);
  }
  /*@ if (size1 > 0 && size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE) 
    open polarssl_public_message(pub)(buffer1, _, _); @*/
  /*@ if (size2 > 0 && (size1 <= 0 || size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)) 
    open polarssl_public_message(pub)(buffer2, _, _); @*/
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_asym_decrypted/*@ <T> @*/(
                                       havege_state *havege_state, void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size1;
  int size2;
  unsigned int osize;
  char buffer1[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer2[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer3[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);

  size1 = net_recv(socket, buffer1, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size2 = net_recv(socket, buffer2, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size1 > 0 && size2 >= POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)
  {
    //@ open polarssl_public_message(pub)(buffer1, size1, ?cs1);
    //@ open polarssl_public_message(pub)(buffer2, size2, ?cs2);
    
    pk_context context;
    //@ close pk_context(&context);
    pk_init(&context);
    //@ int p0;
    //@ int id0;
    //@ polarssl_cryptogram cg_key = polarssl_private_key(p0, id0);
    //@ close exists<polarssl_cryptogram>(cg_key);
    if (pk_parse_key(&context, buffer1, (unsigned int) size1, NULL, 0) == 0)
    {
      if (size2 * 8 <= size1)
      {
        //@ assert pk_context_with_key(&context, pk_private, size1, ?key_id);
        //@ close random_function_predicate(havege_state_initialized);
        /*@ produce_function_pointer_chunk random_function(
                     attacker_key_item_havege_random_stub)
                    (havege_state_initialized)(state, out, len) { call(); } @*/
        if (pk_decrypt(&context, buffer2, (unsigned int) size2, buffer3, &osize,
                      POLARSSL_MAX_MESSAGE_BYTE_SIZE, 
                      attacker_key_item_havege_random_stub, havege_state) == 0)
        {
          //@ assert u_integer(&osize, ?osize_val);
          /*@ switch (key_id)
              {
                case some(pair):
                  assert exists<polarssl_cryptogram>(?cg);
                  assert cg == polarssl_asym_encrypted(
                                            ?principal2, ?id2, ?cs_out2, ?ent2);
                  assert chars(buffer3, osize_val, ?cs_output);
                  switch (pair)
                  {
                    case pair(p1, id1):
                      if (p1 == principal2 && id1 == id2)
                      {
                        assert cs1 == polarssl_chars_for_cryptogram(cg_key);
                        polarssl_cryptogram_in_upper_bound(cs1, cg_key, 
                                    polarssl_generated_public_cryptograms(pub));
                        polarssl_generated_public_cryptograms_from(pub, cg_key);
                        assert [_]pub(cg_key);
                        polarssl_cryptogram cg_enc =
                             polarssl_asym_encrypted(p1, id1, cs_output, ent2);
                        assert cs2 == polarssl_chars_for_cryptogram(cg_enc);
                        polarssl_cryptogram_constraints(cs2, cg_enc);
                        polarssl_cryptogram_in_upper_bound(cs2, cg_enc, 
                                    polarssl_generated_public_cryptograms(pub));
                        polarssl_generated_public_cryptograms_from(pub, cg_enc);
                        assert [_]pub(cg_enc);
                        
                        assert is_polarssl_public_asym_decryption_is_public(
                                                       ?proof3, pub, pred, arg);
                        proof3(cg_key, cg_enc);
                        polarssl_public_message_from_chars(pub, buffer3, 
                                                          osize_val, cs_output);
                      }
                      else
                      {
                        nil == polarssl_cryptograms_in_chars(cs_output);
                        polarssl_public_message_from_chars(pub, buffer3, 
                                                          osize_val, cs_output);
                      }
                  }
                case none:
                  assume_chars_contain_public_cryptograms(buffer3, osize_val);
              }
          @*/
          net_send(socket, buffer3, osize);
          //@ open polarssl_public_message(pub)(buffer3, _, _);
        }
      }
      //@ pk_release_context_with_key(&context);
    }
    pk_free(&context);
    //@ open pk_context(&context);
  }
  /*@ if (size1 > 0 && size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE) 
    open polarssl_public_message(pub)(buffer1, _, _); @*/
  /*@ if (size2 > 0 && (size1 <= 0 || size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)) 
    open polarssl_public_message(pub)(buffer2, _, _); @*/
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_send_asym_signature/*@ <T> @*/(
                                       havege_state *havege_state, void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int temp;
  int size1;
  int size2;
  unsigned int osize;
  char buffer1[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer2[POLARSSL_MAX_MESSAGE_BYTE_SIZE];
  char buffer3[POLARSSL_MAX_MESSAGE_BYTE_SIZE];

  //@ open attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);

  size1 = net_recv(socket, buffer1, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  size2 = net_recv(socket, buffer2, POLARSSL_MAX_MESSAGE_BYTE_SIZE);
  if (size1 > 0 && size2 >= POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)
  {
    //@ open polarssl_public_message(pub)(buffer1, size1, ?cs1);
    //@ open polarssl_public_message(pub)(buffer2, size2, ?cs2);
    
    pk_context context;
    //@ close pk_context(&context);
    pk_init(&context);
    //@ int p0;
    //@ int id0;
    //@ polarssl_cryptogram cg_key = polarssl_private_key(p0, id0);
    //@ close exists<polarssl_cryptogram>(cg_key);
    if (pk_parse_key(&context, buffer1, (unsigned int) size1, NULL, 0) == 0)
    {
      if (size2 * 8 < size1)
      {
        //@ assert pk_context_with_key(&context, pk_private, size1, ?key_id);
        //@ close random_function_predicate(havege_state_initialized);
        /*@ produce_function_pointer_chunk random_function(
                     attacker_key_item_havege_random_stub)
                    (havege_state_initialized)(state, out, len) { call(); } @*/
        if (pk_sign(&context, POLARSSL_MD_NONE, buffer2, (unsigned int) size2, 
                    buffer3, &osize, attacker_key_item_havege_random_stub, 
                    havege_state) == 0)
        {        
          //@ assert u_integer(&osize, ?osize_val);
          /*@ switch (key_id)
              {
                case some(pair):
                  switch (pair)
                  {
                    case pair(p1, id1):
                      assert cs1 == polarssl_chars_for_cryptogram(cg_key);
                      polarssl_cryptogram_in_upper_bound(cs1, cg_key,
                                    polarssl_generated_public_cryptograms(pub));
                      polarssl_generated_public_cryptograms_from(pub, cg_key);
                      assert [_]pub(cg_key);
                      
                      assert polarssl_cryptogram(buffer3, osize_val, 
                                                 ?cs_enc, ?cg_enc);
                      assert cg_enc == polarssl_asym_signature(p1, id1, cs2, _);
                      assert is_polarssl_public_asym_signature_is_public(
                                                        ?proof, pub, pred, arg);
                      proof(cg_enc, polarssl_generated_public_cryptograms(pub));
                      polarssl_public_message_from_cryptogram(pub, buffer3, 
                                                     osize_val, cs_enc, cg_enc);
                  }
                case none:
                  assume_chars_contain_public_cryptograms(buffer3, osize_val);
              }
          @*/
          net_send(socket, buffer3, osize);
          //@ open polarssl_public_message(pub)(buffer3, _, _);
          //@ chars_join(buffer3);
        }
      }
      //@ pk_release_context_with_key(&context);
    }
    pk_free(&context);
    //@ open pk_context(&context);
  }
  /*@ if (size1 > 0 && size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE) 
    open polarssl_public_message(pub)(buffer1, _, _); @*/
  /*@ if (size2 > 0 && (size1 <= 0 || size2 < POLARSSL_MIN_ENCRYPTED_BYTE_SIZE)) 
    open polarssl_public_message(pub)(buffer2, _, _); @*/
  //@ close attacker_invariant(f, pub, pred, arg, havege_state, socket, attacker);
}

void polarssl_attacker_core/*@ <T> @*/(havege_state *havege_state, void* socket)
  /*@ requires attacker_invariant<T>(?f, ?pub, ?pred, ?arg,
                                     havege_state, socket, ?attacker); @*/
  /*@ ensures  attacker_invariant<T>(f, pub, pred, arg, 
                                     havege_state, socket, attacker); @*/
{
  int action;
  //@ open attacker_invariant<T>(f, pub, pred, arg, havege_state, socket, attacker);
  r_int(havege_state, &action);
  //@ close attacker_invariant<T>(f, pub, pred, arg, havege_state, socket, attacker);

  switch (action % 14)
  {
    case 0:
      polarssl_attacker_send_data(havege_state, socket);
      break;
    case 1:
      polarssl_attacker_send_concatenation(havege_state, socket);
      break;
    case 2:
      polarssl_attacker_send_split(havege_state, socket);
      break;
    case 3:
      polarssl_attacker_send_random(havege_state, socket);
      break;
    case 4:
      polarssl_attacker_send_keys(havege_state, socket);
      break;
    case 5:
      polarssl_attacker_send_hash(havege_state, socket);
      break;
    case 6:
      polarssl_attacker_send_hmac(havege_state, socket);
      break;
    case 7:
      polarssl_attacker_send_encrypted(havege_state, socket);
      break;
    case 8:
      polarssl_attacker_send_decrypted(havege_state, socket);
      break;
    case 9:
      polarssl_attacker_send_auth_encrypted(havege_state, socket);
      break;
    case 10:
      polarssl_attacker_send_auth_decrypted(havege_state, socket);
      break;
    case 11:
      polarssl_attacker_send_asym_encrypted(havege_state, socket);
      break;
    case 12:
      polarssl_attacker_send_asym_decrypted(havege_state, socket);
      break;
    case 13:
      polarssl_attacker_send_asym_signature(havege_state, socket);
      break;
  }
}

void polarssl_attacker/*@ <T> @*/(int* i)
  /*@ requires [?f]polarssl_world<T>(?polarssl_pub,
                                     ?polarssl_proof_pred, 
                                     ?polarssl_proof_arg) &*&
               polarssl_proof_pred(polarssl_proof_arg) &*&
               integer(i, ?count1) &*& count1 >= 0 &*&
               polarssl_principals(count1); @*/
  /*@ ensures  [f]polarssl_world<T>(polarssl_pub, 
                                    polarssl_proof_pred, 
                                    polarssl_proof_arg) &*&
               polarssl_proof_pred(polarssl_proof_arg) &*&
               integer(i, ?count2) &*& polarssl_principals(count2) &*&
               count2 > count1; @*/
{
  havege_state havege_state;
  //@ close havege_state(&havege_state);
  havege_init(&havege_state);
  
  {
    bool havege_failure = false;
    int server_or_client;
    int port;
    int* socket;
    int socket1;
    int socket2;
    
    //@ int bad_one = polarssl_create_principal();
    //@ assume (bad(bad_one));
    if (*i < 0 || *i >= INT_MAX) abort(); (*i)++;
    
    r_int(&havege_state, &server_or_client);
    r_int(&havege_state, &port);

    bool network_failure = false;
    if (server_or_client % 2 == 0)
    {
      if(net_connect(&socket1, NULL, port) != 0) network_failure = true;
      else if(net_set_block(socket1) != 0) network_failure = true;
      socket = &socket1;
    }
    else
    {
      if(net_bind(&socket1, NULL, port) != 0) 
        {network_failure = true;}
      else if(net_accept(socket1, &socket2, NULL) != 0)
        {net_close(socket1); network_failure = true;}
      else if(net_set_block(socket2) != 0) 
        {net_close(socket1); network_failure = true;}
      socket = &socket2;
    }

    if (!network_failure)
    {
      //@ polarssl_get_proof_obligations();
      /*@ open polarssl_proof_obligations<T>(polarssl_pub, 
                                             polarssl_proof_pred, 
                                             polarssl_proof_arg); @*/
      /*@ close attacker_invariant<T>(f, polarssl_pub, polarssl_proof_pred, 
                       polarssl_proof_arg, &havege_state, socket, bad_one); @*/
      int j = 0;     
      while(j < POLARSSL_ATTACKER_ITERATIONS)
        /*@ invariant attacker_invariant<T>(f, polarssl_pub, polarssl_proof_pred, 
                       polarssl_proof_arg, &havege_state, socket, bad_one); @*/
      {
        polarssl_attacker_core(&havege_state, socket);
        j++;
      }
      /*@ open attacker_invariant<T>(f, polarssl_pub, polarssl_proof_pred, 
                       polarssl_proof_arg, &havege_state, socket, bad_one); @*/
      /*@ close polarssl_proof_obligations<T>(polarssl_pub, 
                                              polarssl_proof_pred, 
                                              polarssl_proof_arg); @*/
      /*@ leak polarssl_proof_obligations<T>(polarssl_pub, 
                                             polarssl_proof_pred, 
                                             polarssl_proof_arg); @*/
    }

    if (!network_failure)
    {
      if (server_or_client % 2 == 0)
        net_close(socket1);
      else
      {
        net_close(socket1);
        net_close(socket2);
      }
    }
    //@ polarssl_destroy_principal(bad_one);
  }
  havege_free(&havege_state);
  //@ open havege_state(&havege_state);
}
