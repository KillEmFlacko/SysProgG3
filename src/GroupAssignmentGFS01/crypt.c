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

#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/ossl_typ.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "GroupAssignmentGFS01/crypt.h"

#define ERRMSG_MAX_LEN 128

/**
  @file crypt.c
  @brief Library for symmetric message encryption with AES in counter monde
  */


/**
 * @brief Load the key from disc
 * 
 * @param path path of the file on disc where the key is stored
 * @param key variable in which store the key
 * 
 */
int Crypt_loadKey(const char *path, unsigned char *key)
{
	char error_string[ERRMSG_MAX_LEN];

	int fd;
	if((fd = open(path,O_RDONLY)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"Crypt_loadKey(path: %p,key: %p) - Cannot open key file",path,key);
		perror(error_string);
		return -errno;
	}

	if(read(fd,key,KEY_LEN) != KEY_LEN)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"Crypt_loadKey(path: %p,key: %p) - Error while reading key",path,key);
		perror(error_string);
		return -errno;
	}

	return 0;
}

/**
 * @brief Load the initialization vector from disc
 * 
 * @param path path of the file on disc where the initialization vector is stored
 * @param iv variable in which store the initialization vector
 * 
 */
int Crypt_loadIV(const char *path, unsigned char *iv)
{
	char error_string[ERRMSG_MAX_LEN];

	int fd;
	if((fd = open(path,O_RDONLY)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"Crypt_loadIV(path: %p,iv: %p) - Cannot open IV file",path,iv);
		perror(error_string);
		return -errno;
	}

	if(read(fd,iv,IV_LEN) != IV_LEN)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"Crypt_loadKey(path: %p,iv: %p) - Error while reading iv",path,iv);
		perror(error_string);
		return -errno;
	}

	return 0;
}

/**
 * @brief Encrypt the plaintext in the corresponding ciphertext
 * 
 * @param key key for the AES encryption in CTR mode
 * @param iv initialization vector for the AES encryption in CTR mode
 * @param plaintext parameter containing the entire plaintext
 * @param plaintext_len lenght of the plaintext in byte
 * @param offset distance in byte from the start of the block
 * @param ciphertext parameter containing the ciphertext
 * 
 */
int Crypt_encrypt(unsigned char *key, unsigned char *iv,
				  unsigned char *plaintext, size_t plaintext_len, off_t offset,
				  unsigned char *ciphertext)
{

	/*
	 * Creo il contesto per la cifratura
	 */
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

	if(ctx == NULL)
	{
		fprintf(stderr,"Crypt_encrypt(key: %p,iv: %p, plaintext: %p, plaintext_len: %lu, offset: %lu, ciphertext: %p) - Cannot get context\n",key,iv,plaintext,plaintext_len,offset,ciphertext);
		return -1;
	}

	/*
	 * Inizializzo per cifrare
	 */
	if(EVP_EncryptInit_ex(ctx,_CRYPT_CIPHER,NULL,key,iv) != 1)
	{
		fprintf(stderr,"Crypt_encrypt(key: %p,iv: %p, plaintext: %p, plaintext_len: %lu, offset: %lu, ciphertext: %p) - Cannot init the cipher\n",key,iv,plaintext,plaintext_len,offset,ciphertext);
		return -1;
	}

	/*
	 * Gestisco l'eventuale offset
	 */
	int block_size = EVP_CIPHER_block_size(_CRYPT_CIPHER);
	int left_padding = offset % block_size;
	int ctr_num = offset / block_size;

	/*
	 * Incremento il contatore di AES 256 CTR in base ai blocchi da
	 * saltare a causa dell'offset
	 */
	int output_bytes = 0;
	if(ctr_num > 0)
	{
		unsigned char *dummy_plain = (unsigned char*)malloc(block_size);
		unsigned char *dummy_cipher = (unsigned char*)malloc(block_size);
		for(int i = 0; i < ctr_num; i++)
		{
			if(EVP_EncryptUpdate(ctx,dummy_cipher, &output_bytes, dummy_plain, block_size) != 1)
			{
				fprintf(stderr,"Crypt_encrypt(key: %p,iv: %p, plaintext: %p, plaintext_len: %lu, offset: %lu, ciphertext: %p) - Error encountered while handling offset\n",key,iv,plaintext,plaintext_len,offset,ciphertext);
				return -1;
			}
		}
		free(dummy_plain);
		free(dummy_cipher);
	}

	/*
	 * Poi aggiungo del padding all'inizio se
	 * l'offset si trova nel bel mezzo di un blocco
	 */
	unsigned char *act_plaintext = plaintext;
	size_t act_plaintext_len = plaintext_len + left_padding;

	unsigned char *act_ciphertext = ciphertext;
	size_t act_ciphertext_len = act_plaintext_len + block_size;

	if(left_padding > 0)
	{
		act_plaintext = (unsigned char*)malloc(act_plaintext_len);
		memcpy(act_plaintext + left_padding, plaintext, plaintext_len);

		act_ciphertext = (unsigned char*)malloc(act_ciphertext_len);
	}

	/*
	 * Cifro il plaintext
	 */
	int ciphertext_len = 0;
	output_bytes = 0;
	if(EVP_EncryptUpdate(ctx, act_ciphertext, &output_bytes, act_plaintext, act_plaintext_len) != 1)
	{
		fprintf(stderr,"Crypt_encrypt(key: %p,iv: %p, plaintext: %p, plaintext_len: %lu, offset: %lu, ciphertext: %p) - Error while encrypting ciphertext\n",key,iv,plaintext,plaintext_len,offset,ciphertext);
		return -1;
	}
	ciphertext_len = output_bytes;

	/*
	 * Finalizzo il processo di cifratura
	 */
	if(EVP_EncryptFinal_ex(ctx, act_ciphertext + output_bytes, &output_bytes) != 1)
	{
		fprintf(stderr,"Crypt_encrypt(key: %p,iv: %p, plaintext: %p, plaintext_len: %lu, offset: %lu, ciphertext: %p) - Error while finalizing encryption\n",key,iv,plaintext,plaintext_len,offset,ciphertext);
		return -1;
	}
	ciphertext_len += output_bytes;

	if(left_padding > 0)
	{
		free(act_plaintext);
		free(act_ciphertext);
	}

	/*
	 * Rimuovo il padding iniziale
	 */
	ciphertext_len -= left_padding;
	memcpy(ciphertext, act_ciphertext + left_padding, ciphertext_len);

	return ciphertext_len;
}

/**
 * @brief Decrypt the ciphertext in the corresponding plaintext
 * 
 * @param key key for the AES encryption in CTR mode
 * @param iv initialization vector for the AES encryption in CTR mode
 * @param plaintext parameter containing the entire plaintext
 * @param plaintext_len lenght of the plaintext in byte
 * @param offset distance in byte from the start of the block
 * @param ciphertext parameter containing the ciphertext
 * 
 */
int Crypt_decrypt(unsigned char *key, unsigned char *iv,
				  unsigned char *ciphertext, size_t ciphertext_len, off_t offset,
				  unsigned char *plaintext)
{

	/*
	 * Creo il contesto per la cifratura
	 */
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

	if(ctx == NULL)
	{
		fprintf(stderr,"Crypt_decrypt(key: %p,iv: %p, ciphertext: %p, ciphertext_len: %lu, offset: %lu, plaintext: %p) - Cannot get context\n",key,iv,ciphertext,ciphertext_len,offset,plaintext);
		return -1;
	}

	/*
	 * Inizializzo per decifrare
	 */
	if(EVP_DecryptInit_ex(ctx,_CRYPT_CIPHER,NULL,key,iv) != 1)
	{
		fprintf(stderr,"Crypt_decrypt(key: %p,iv: %p, ciphertext: %p, ciphertext_len: %lu, offset: %lu, plaintext: %p) - Cannot init cipher\n",key,iv,ciphertext,ciphertext_len,offset,plaintext);
		return -1;
	}

	/*
	 * Gestisco l'eventuale offset
	 */
	int block_size = EVP_CIPHER_block_size(_CRYPT_CIPHER);
	int left_padding = offset % block_size;
	int ctr_num = offset / block_size;

	/*
	 * Incremento il contatore di AES 256 CTR in base ai blocchi da
	 * saltare a causa dell'offset
	 */
	int output_bytes = 0;
	if(ctr_num > 0)
	{
		unsigned char *dummy_plain = (unsigned char*)malloc(block_size);
		unsigned char *dummy_cipher = (unsigned char*)malloc(block_size);
		for(int i = 0; i < ctr_num; i++)
		{
			if(EVP_DecryptUpdate(ctx,dummy_plain, &output_bytes, dummy_plain, block_size) != 1)
			{
				fprintf(stderr,"Crypt_decrypt(key: %p,iv: %p, ciphertext: %p, ciphertext_len: %lu, offset: %lu, plaintext: %p) - Cannot error while handling offset\n",key,iv,ciphertext,ciphertext_len,offset,plaintext);
				return -1;
			}
		}
		free(dummy_plain);
		free(dummy_cipher);
	}

	/*
	 * Poi aggiungo del padding all'inizio se
	 * l'offset si trova nel bel mezzo di un blocco
	 */
	unsigned char *act_ciphertext = ciphertext;
	size_t act_ciphertext_len = ciphertext_len + left_padding;

	unsigned char *act_plaintext = plaintext;
	size_t act_plaintext_len = act_ciphertext_len + block_size;


	if(left_padding > 0)
	{
		act_plaintext = (unsigned char*)malloc(act_plaintext_len);

		act_ciphertext = (unsigned char*)malloc(act_ciphertext_len);
		memcpy(act_ciphertext + left_padding, ciphertext, ciphertext_len);
	}

	/*
	 * Decifro il ciphertext
	 */
	int plaintext_len = 0;
	output_bytes = 0;
	if(EVP_DecryptUpdate(ctx, act_plaintext, &output_bytes, act_ciphertext, act_ciphertext_len) != 1)
	{
		fprintf(stderr,"Crypt_decrypt(key: %p,iv: %p, ciphertext: %p, ciphertext_len: %lu, offset: %lu, plaintext: %p) - Error while decrypting ciphertext\n",key,iv,ciphertext,ciphertext_len,offset,plaintext);
		return -1;
	}
	plaintext_len = output_bytes;

	/*
	 * Finalizzo il processo di decifratura
	 */
	if(EVP_DecryptFinal_ex(ctx, act_plaintext + output_bytes, &output_bytes) != 1)
	{
		fprintf(stderr,"Crypt_decrypt(key: %p,iv: %p, ciphertext: %p, ciphertext_len: %lu, offset: %lu, plaintext: %p) - Error while finalizing decryption\n",key,iv,ciphertext,ciphertext_len,offset,plaintext);
		return -1;
	}
	plaintext_len += output_bytes;

	if(left_padding > 0)
	{
		free(act_plaintext);
		free(act_ciphertext);
	}

	/*
	 * Rimuovo il padding iniziale
	 */
	plaintext_len -= left_padding;
	memcpy(plaintext, act_plaintext + left_padding, plaintext_len);

	return plaintext_len;
}
