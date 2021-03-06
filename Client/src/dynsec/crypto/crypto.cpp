#include "crypto.hpp"

namespace Dynsec::Crypto {
#define IS_ALIGNED_32(p) (0 == (3 & ((const char*)(p) - (const char*)0)))
#define ROTR64(Data, Bits) (((Data) >> Bits) | ((Data) << (64 - Bits)))
#define Sigma0(x) (ROTR64((x),28) ^ ROTR64((x),34) ^ ROTR64((x),39))
#define Sigma1(x) (ROTR64((x),14) ^ ROTR64((x),18) ^ ROTR64((x),41))
#define sigma0(x) (ROTR64((x),1) ^ ROTR64((x),8) ^ ((x)>>7))
#define sigma1(x) (ROTR64((x),19) ^ ROTR64((x),61) ^ ((x)>>6))
#define Ch(x,y,z) (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

	static const uint64_t SHA512InitialState[8] = {
		0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B,
		0xA54FF53A5F1D36F1, 0x510E527FADE682D1, 0x9B05688C2B3E6C1F,
		0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179
	};

	static const uint64_t SHA512K[80] = {
		0x428A2F98D728AE22, 0x7137449123EF65CD, 0xB5C0FBCFEC4D3B2F,
		0xE9B5DBA58189DBBC, 0x3956C25BF348B538, 0x59F111F1B605D019,
		0x923F82A4AF194F9B, 0xAB1C5ED5DA6D8118, 0xD807AA98A3030242,
		0x12835B0145706FBE, 0x243185BE4EE4B28C, 0x550C7DC3D5FFB4E2,
		0x72BE5D74F27B896F, 0x80DEB1FE3B1696B1, 0x9BDC06A725C71235,
		0xC19BF174CF692694, 0xE49B69C19EF14AD2, 0xEFBE4786384F25E3,
		0x0FC19DC68B8CD5B5, 0x240CA1CC77AC9C65, 0x2DE92C6F592B0275,
		0x4A7484AA6EA6E483, 0x5CB0A9DCBD41FBD4, 0x76F988DA831153B5,
		0x983E5152EE66DFAB, 0xA831C66D2DB43210, 0xB00327C898FB213F,
		0xBF597FC7BEEF0EE4, 0xC6E00BF33DA88FC2, 0xD5A79147930AA725,
		0x06CA6351E003826F, 0x142929670A0E6E70, 0x27B70A8546D22FFC,
		0x2E1B21385C26C926, 0x4D2C6DFC5AC42AED, 0x53380D139D95B3DF,
		0x650A73548BAF63DE, 0x766A0ABB3C77B2A8, 0x81C2C92E47EDAEE6,
		0x92722C851482353B, 0xA2BFE8A14CF10364, 0xA81A664BBC423001,
		0xC24B8B70D0F89791, 0xC76C51A30654BE30, 0xD192E819D6EF5218,
		0xD69906245565A910, 0xF40E35855771202A, 0x106AA07032BBD1B8,
		0x19A4C116B8D2D0C8, 0x1E376C085141AB53, 0x2748774CDF8EEB99,
		0x34B0BCB5E19B48A8, 0x391C0CB3C5C95A63, 0x4ED8AA4AE3418ACB,
		0x5B9CCA4F7763E373, 0x682E6FF3D6B2B8A3, 0x748F82EE5DEFB2FC,
		0x78A5636F43172F60, 0x84C87814A1F0AB72, 0x8CC702081A6439EC,
		0x90BEFFFA23631E28, 0xA4506CEBDE82BDE9, 0xBEF9A3F7B2C67915,
		0xC67178F2E372532B, 0xCA273ECEEA26619C, 0xD186B8C721C0C207,
		0xEADA7DD6CDE0EB1E, 0xF57D4F7FEE6ED178, 0x06F067AA72176FBA,
		0x0A637DC5A2C898A6, 0x113F9804BEF90DAE, 0x1B710B35131C471B,
		0x28DB77F523047D84, 0x32CAAB7B40C72493, 0x3C9EBE0A15C9BEBC,
		0x431D67C49C100D4C, 0x4CC5D4BECB3E42B6, 0x597F299CFC657E2A,
		0x5FCB6FAB3AD6FAEC, 0x6C44198C4A475817
	};

	static const unsigned int CRC32Table[] = {
	  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
	  0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
	  0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
	  0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
	  0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
	  0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
	  0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
	  0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
	  0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
	  0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
	  0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
	  0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
	  0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
	  0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
	  0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
	  0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
	  0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
	  0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
	  0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
	  0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
	  0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
	  0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
	  0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
	  0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
	  0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
	  0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
	  0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
	  0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
	  0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
	  0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
	  0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
	  0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
	  0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
	  0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
	  0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
	  0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
	  0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
	  0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
	  0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
	  0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
	  0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
	  0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
	  0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
	};

	uint32_t CRC32(uint8_t* pbInp, uint64_t cbInp) {
		uint32_t crc = 0xffffffff;
		while (cbInp--) {
			crc = (crc << 8) ^ CRC32Table[((crc >> 24) ^ *pbInp) & 255];
			pbInp++;
		}
		return crc;
	}

	void RC4(uint8_t* pbKey, uint32_t cbKey, uint8_t* pbInpOut, uint32_t cbInpOut, uint32_t Offset) {
		unsigned char s[256];
		unsigned char k[256];
		unsigned char temp;
		int i, j;

		for (i = 0; i < 256; i++) {
			s[i] = (unsigned char)i;
			k[i] = pbKey[i % cbKey];
		}

		j = 0;
		for (i = 0; i < 256; i++) {
			j = (j + s[i] + k[i]) % 256;
			temp = s[i];
			s[i] = s[j];
			s[j] = temp;
		}

		i = j = 0;
		for (unsigned int x = Offset; x < cbInpOut; x++) {
			i = (i + 1) % 256;
			j = (j + s[i]) % 256;
			temp = s[i];
			s[i] = s[j];
			s[j] = temp;
			int t = (s[i] + s[j]) % 256;
			pbInpOut[x] ^= s[t];
		}
	}

	void SHA512Init(SHA512State* pShaState) {
		memset(pShaState, 0, sizeof(SHA512State));

		pShaState->m_Count = 0;
		memcpy(&pShaState->m_State, SHA512InitialState, sizeof(SHA512InitialState));
	}

	void SHA512Transform(uint64_t* pDigest, uint64_t* pInp) {
		uint64_t* W = pInp;
		uint64_t X[128 / sizeof(uint64_t)];

		uint64_t A = pDigest[0];
		uint64_t B = pDigest[1];
		uint64_t C = pDigest[2];
		uint64_t D = pDigest[3];
		uint64_t E = pDigest[4];
		uint64_t F = pDigest[5];
		uint64_t G = pDigest[6];
		uint64_t H = pDigest[7];

		int i;

		for (i = 0; i < 16; ++i) {
			uint64_t Temp1 = X[i] = _byteswap_uint64(W[i]);
			uint64_t Temp2 = 0;

			Temp1 += H + Sigma1(E) + Ch(E, F, G) + SHA512K[i];
			Temp2 = Sigma0(A) + Maj(A, B, C);

			H = G;
			G = F;
			F = E;
			E = D + Temp1;
			D = C;
			C = B;
			B = A;
			A = Temp1 + Temp2;
		}

		for (; i < 80; ++i) {
			uint64_t Temp1 = 0;
			uint64_t Temp2 = 0;

			uint64_t S0 = sigma0(X[(i + 1) & 15]);
			uint64_t S1 = sigma1(X[(i + 14) & 15]);

			Temp1 = X[i & 15] += S0 + S1 + X[(i + 9) & 15];
			Temp1 += H + Sigma1(E) + Ch(E, F, G) + SHA512K[i];
			Temp2 = Sigma0(A) + Maj(A, B, C);

			H = G;
			G = F;
			F = E;
			E = D + Temp1;
			D = C;
			C = B;
			B = A;
			A = Temp1 + Temp2;
		}

		pDigest[0] += A;
		pDigest[1] += B;
		pDigest[2] += C;
		pDigest[3] += D;
		pDigest[4] += E;
		pDigest[5] += F;
		pDigest[6] += G;
		pDigest[7] += H;
	}

	void SHA512Update(SHA512State* pShaState, uint8_t* pbInp, uint32_t cbInp) {
		DWORD Index = pShaState->m_Count & 127;

		pShaState->m_Count = pShaState->m_Count + cbInp;

		if (Index) {
			if (Index + cbInp >= 128) {
				memcpy(&pShaState->m_Buffer[Index], pbInp, Index - 128);

				SHA512Transform((uint64_t*)pShaState->m_State, (uint64_t*)pShaState->m_Buffer);

				pbInp += 128;
				cbInp -= 128;
			}
		}

		if (cbInp >= 128) {
			DWORD Blocks = (Index + cbInp) / 128;

			if (IS_ALIGNED_32(pbInp)) {
				for (DWORD i = 0; i < Blocks; ++i) {
					SHA512Transform((uint64_t*)pShaState->m_State, (uint64_t*)pbInp);

					pbInp += 128;
					cbInp -= 128;
				}
			} else {
				for (DWORD i = 0; i < Blocks; ++i) {
					memcpy(pShaState->m_Buffer, pbInp, 128);

					SHA512Transform((uint64_t*)pShaState->m_State, (uint64_t*)pShaState->m_Buffer);

					pbInp += 128;
					cbInp -= 128;
				}
			}
		}

		if (cbInp) {
			memcpy(pShaState->m_Buffer, pbInp, cbInp);
		}
	}

	void SHA512Final(SHA512State* pShaState, uint8_t* pbOut, uint32_t cbOut) {
		DWORD Count = pShaState->m_Count;

		DWORD Index = Count & 127;

		memset(&pShaState->m_Buffer[Index], 0, 128 - Index);

		pShaState->m_Buffer[Index] = 0x80;

		if (128 - Index < 17) {
			SHA512Transform((uint64_t*)pShaState->m_State, (uint64_t*)pShaState->m_Buffer);

			memset(pShaState->m_Buffer, 0, Index + 1);
		}

		Count = Count << 3;

		DWORD* New = (DWORD*)&pShaState->m_Buffer[128 - sizeof(DWORD)];
		DWORD* Input = &Count;

		for (std::size_t i = 0; i < 1; ++i) {
			New[i] = _byteswap_ulong(Input[i]);
		}

		SHA512Transform((uint64_t*)pShaState->m_State, (uint64_t*)pShaState->m_Buffer);

		if (cbOut != 0) {
			for (int i = 0; i < ARRAYSIZE(pShaState->m_State); ++i) {
				pShaState->m_State[i] = _byteswap_uint64(pShaState->m_State[i]);
			}

			if (cbOut < 64) {
				memcpy(pbOut, pShaState->m_State, cbOut);
			} else {
				memcpy(pbOut, pShaState->m_State, 64);
			}
		}
	}
}