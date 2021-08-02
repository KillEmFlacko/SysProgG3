/*
 * Course: System Programming 2020/2021
 *
 * Lecturers:
 * Alessia		Saggese asaggese@unisa.it
 * Francesco	Moscato	fmoscato@unisa.it
 *
 * Group:
 * D'Alessio	Simone		0622701120	s.dalessio8@studenti.unisa.it
 * Falanga		Armando		0622701140  a.falanga13@studenti.unisa.it
 * Fattore		Alessandro  0622701115  a.fattore@studenti.unisa.it
 *
 * Copyright (C) 2021 - All Rights Reserved
 *
 * This file is part of SysProgG3.
 *
 * SysProgG3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SysProgG3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SysProgG3.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _H_CRYPT
#define _H_CRYPT

#include <openssl/evp.h>
#include <stdio.h>

#define _CRYPT_CIPHER (EVP_aes_256_ctr())
#define KEY_LEN 32 // Key len in byte
#define IV_LEN 16  // Initialization Vector len in byte

int Crypt_loadKey(const char *path, unsigned char *key);

int Crypt_loadIV(const char *path, unsigned char *iv);

int Crypt_encrypt(unsigned char *key, unsigned char *iv,
				  unsigned char *plaintext, size_t plaintext_len, off_t offset,
				  unsigned char *ciphertext);

int Crypt_decrypt(unsigned char *key, unsigned char *iv,
				  unsigned char *ciphertext, size_t ciphertext_len, off_t offset,
				  unsigned char *plaintext);

#endif /* ifndef _H_CRYPT */
